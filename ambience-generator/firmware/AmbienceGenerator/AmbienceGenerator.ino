// Ambience Generator - ESP32-S3
// Noise + Crackle runs in RNBO (rnbo_source.cpp).
// Rain and Bird are my own PSRAM players, because RNBO's groove~ made a whine on the ESP32.
// All three layers get summed in audioTask and go out over I2S.

#include <Arduino.h>
#include <pthread.h>
#include <esp_pthread.h>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <WiFi.h>
#include <esp32-hal-bt.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>
#include <LittleFS.h>
#include <esp_heap_caps.h>

#include "AmbienceEngine.h"

extern "C" RNBO::PatcherFactoryFunctionPtr GetPatcherFactoryFunction();

namespace {

constexpr double kSampleRate = 44100.0;
constexpr int kBlockSize = 128;
constexpr int kBlocksPerPrint = static_cast<int>(kSampleRate / kBlockSize + 0.5);

// swap CLK/DT if a knob turns the wrong way
constexpr int kKnob1ClkPin = 14, kKnob1DtPin = 13, kKnob1SwPin = 21;  // Noise
constexpr int kKnob2ClkPin = 10, kKnob2DtPin = 11, kKnob2SwPin = 12;  // Rain
constexpr int kKnob3ClkPin = 15, kKnob3DtPin = 16, kKnob3SwPin = 17;  // Bird

// 44.1k 16-bit stereo raw PCM in LittleFS
constexpr const char* kRainAssetPaths[3] = { "/rain_0.pcm", "/rain_1.pcm", "/rain_2.pcm" };
constexpr const char* kBirdAssetPaths[3] = { "/bird_0.pcm", "/bird_1.pcm", "/bird_2.pcm" };

struct PsramLoop {
    float* buffer = nullptr;
    size_t frames = 0;
    volatile size_t playhead = 0;
    float gain = 0.0f;
};

PsramLoop g_bird{nullptr, 0, 0, 0.35f};
PsramLoop g_rain{nullptr, 0, 0, 0.35f};

struct RainExtra {
    float filterOpen = 1.0f;  // 1 = open, 0 = 200 Hz
    float lpL = 0.0f, lpR = 0.0f;
};
RainExtra g_rainExtra;

struct BirdExtra {
    float howoften = 0.5f;
    float space = 0.0f;
    bool active = false;
    uint32_t framesUntilNext = 0;
    // float playhead so each chirp can play at a slightly different speed
    float playheadF = 0.0f;
    float playbackRate = 1.0f;
};
BirdExtra g_birdExtra;

// one feedback delay line for the bird "space"
constexpr float kMaxDelaySeconds = 0.35f;
constexpr size_t kMaxDelayFrames = static_cast<size_t>(kMaxDelaySeconds * kSampleRate);
float* g_birdDelayL = nullptr;
float* g_birdDelayR = nullptr;
size_t g_birdDelayWritePos = 0;

// blend the tail into the head so the rain loop doesn't click
void applyEqualPowerLoopCrossfade(float* buffer, size_t frames) {
    constexpr float kCrossfadeSeconds = 0.2f;
    size_t xfadeFrames = static_cast<size_t>(kCrossfadeSeconds * kSampleRate);
    if (xfadeFrames * 4 > frames) xfadeFrames = frames / 4;
    if (xfadeFrames == 0) return;
    for (size_t i = 0; i < xfadeFrames; i++) {
        const float t = static_cast<float>(i) / static_cast<float>(xfadeFrames);
        const float fadeOut = cosf(t * (float)M_PI / 2.0f);
        const float fadeIn = sinf(t * (float)M_PI / 2.0f);
        const size_t tailIdx = frames - xfadeFrames + i;
        buffer[tailIdx * 2 + 0] = buffer[tailIdx * 2 + 0] * fadeOut + buffer[i * 2 + 0] * fadeIn;
        buffer[tailIdx * 2 + 1] = buffer[tailIdx * 2 + 1] * fadeOut + buffer[i * 2 + 1] * fadeIn;
    }
}

bool loadPcmFileToPsram(const char* path, float** outBuffer, size_t* outFrames) {
    if (!LittleFS.begin(/*formatOnFail=*/false)) {
        Serial.println("[pcmloop] LittleFS mount failed - check partition scheme and that data/ was flashed");
        return false;
    }
    File f = LittleFS.open(path, "r");
    if (!f) {
        Serial.printf("[pcmloop] open %s failed\n", path);
        return false;
    }
    const size_t fileBytes = f.size();
    const size_t frames = fileBytes / (2 * sizeof(int16_t));
    const size_t destBytes = frames * 2 * sizeof(float);
    float* dest = static_cast<float*>(heap_caps_malloc(destBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (dest == nullptr) {
        Serial.printf("[pcmloop] PSRAM alloc failed for %s (%u bytes)\n", path, (unsigned)destBytes);
        f.close();
        return false;
    }

    constexpr size_t kChunkFrames = 2048;
    static int16_t chunk[kChunkFrames * 2];
    size_t framesDone = 0;
    while (framesDone < frames) {
        const size_t framesWanted = std::min(kChunkFrames, frames - framesDone);
        const size_t bytesWanted = framesWanted * 2 * sizeof(int16_t);
        const size_t bytesGot = f.read(reinterpret_cast<uint8_t*>(chunk), bytesWanted);
        const size_t framesGot = bytesGot / (2 * sizeof(int16_t));
        if (framesGot == 0) break;
        for (size_t i = 0; i < framesGot; i++) {
            const size_t outIdx = (framesDone + i) * 2;
            dest[outIdx + 0] = static_cast<float>(chunk[i * 2 + 0]) / 32768.0f;
            dest[outIdx + 1] = static_cast<float>(chunk[i * 2 + 1]) / 32768.0f;
        }
        framesDone += framesGot;
    }
    f.close();

    *outBuffer = dest;
    *outFrames = framesDone;
    Serial.printf("[pcmloop] %s: loaded %u frames (%.2fs) into PSRAM\n", path, (unsigned)framesDone, framesDone / (float)kSampleRate);
    return true;
}

// audioTask reads these from the other core with no lock,
// so frames goes to 0 first and gets set again last
bool swapLoopAsset(PsramLoop& loop, const char* path, bool applyLoopCrossfade) {
    float* newBuffer = nullptr;
    size_t newFrames = 0;
    if (!loadPcmFileToPsram(path, &newBuffer, &newFrames)) {
        return false;
    }
    if (applyLoopCrossfade) {
        applyEqualPowerLoopCrossfade(newBuffer, newFrames);
    }
    float* oldBuffer = loop.buffer;
    loop.frames = 0;
    if (oldBuffer != nullptr) heap_caps_free(oldBuffer);
    loop.playhead = 0;
    loop.buffer = newBuffer;
    loop.frames = newFrames;
    return true;
}

// rain knob: press cycles volume / filter / type
struct RainKnobState {
    enum { kVolume = 0, kFilter, kType, kSlotCount };
    int slot = kVolume;
    int typeIndex = 0;

    void onRotate(int delta) {
        switch (slot) {
            case kVolume:
                g_rain.gain = std::clamp(g_rain.gain + delta * 0.02f, 0.0f, 1.0f);
                break;
            case kFilter:
                g_rainExtra.filterOpen = std::clamp(g_rainExtra.filterOpen + delta * 0.02f, 0.0f, 1.0f);
                break;
            case kType: {
                int next = ((typeIndex + delta) % 3 + 3) % 3;
                if (next != typeIndex && swapLoopAsset(g_rain, kRainAssetPaths[next], /*applyLoopCrossfade=*/true)) {
                    typeIndex = next;
                }
                break;
            }
        }
    }
    void onPress() { slot = (slot + 1) % kSlotCount; }
    const char* slotLabel() const {
        static const char* kLabels[] = { "rainvolume", "rainfilter", "raintype" };
        return kLabels[slot];
    }
    float slotValue() const {
        switch (slot) {
            case kVolume: return g_rain.gain;
            case kFilter: return g_rainExtra.filterOpen;
            default: return static_cast<float>(typeIndex);
        }
    }
    float slotNormalized() const {
        switch (slot) {
            case kVolume: return g_rain.gain;
            case kFilter: return g_rainExtra.filterOpen;
            default: return typeIndex / 2.0f;
        }
    }
};
RainKnobState g_rainKnob;

// bird knob: press cycles volume / howoften / space / type
struct BirdKnobState {
    enum { kVolume = 0, kHowOften, kSpace, kType, kSlotCount };
    int slot = kVolume;
    int typeIndex = 0;

    void onRotate(int delta) {
        switch (slot) {
            case kVolume:
                g_bird.gain = std::clamp(g_bird.gain + delta * 0.02f, 0.0f, 1.0f);
                break;
            case kHowOften:
                g_birdExtra.howoften = std::clamp(g_birdExtra.howoften + delta * 0.02f, 0.0f, 1.0f);
                break;
            case kSpace:
                g_birdExtra.space = std::clamp(g_birdExtra.space + delta * 0.02f, 0.0f, 1.0f);
                break;
            case kType: {
                int next = ((typeIndex + delta) % 3 + 3) % 3;
                if (next != typeIndex && swapLoopAsset(g_bird, kBirdAssetPaths[next], /*applyLoopCrossfade=*/false)) {
                    typeIndex = next;
                }
                break;
            }
        }
    }
    void onPress() { slot = (slot + 1) % kSlotCount; }
    const char* slotLabel() const {
        static const char* kLabels[] = { "birdvolume", "howoften", "space", "birdtype" };
        return kLabels[slot];
    }
    float slotValue() const {
        switch (slot) {
            case kVolume: return g_bird.gain;
            case kHowOften: return g_birdExtra.howoften;
            case kSpace: return g_birdExtra.space;
            default: return static_cast<float>(typeIndex);
        }
    }
    float slotNormalized() const {
        switch (slot) {
            case kVolume: return g_bird.gain;
            case kHowOften: return g_birdExtra.howoften;
            case kSpace: return g_birdExtra.space;
            default: return typeIndex / 2.0f;
        }
    }
};
BirdKnobState g_birdKnob;

RNBO::CoreObject* g_rnbo = nullptr;
RnboSamplePool g_samplePool;  // only here because KnobGroup::begin() wants one
I2SOutput g_i2s;
OledDisplay g_oled;
RotaryEncoder g_knobEncoder1, g_knobEncoder2, g_knobEncoder3;
KnobGroup g_knob1;  // Noise, RNBO params

// whatever knob moved last is what the OLED shows
const char* g_displayGroupLabel = "Noise";
const char* g_displaySlotLabel = "";
float g_displayValue = 0.0f;
float g_displayNormalized = 0.0f;
bool g_displayDirty = true;

pthread_t g_audioThread;

inline int16_t floatToInt16(float v) {
    if (v > 1.0f) v = 1.0f;
    if (v < -1.0f) v = -1.0f;
    return static_cast<int16_t>(v * 32767.0f);
}

void* audioTask(void* /*arg*/) {
    static float outL[kBlockSize];
    static float outR[kBlockSize];
    static int16_t interleaved[kBlockSize * 2];
    RNBO::SampleValue* const outs[2] = { outL, outR };
    const RNBO::SampleValue* const* const noInputs = nullptr;

    Serial.println("[boot] step7 audioTask started running");
    Serial.flush();

    int debugBlockCounter = 0;
    int64_t processUs = 0, writeUs = 0;
    for (;;) {
        int64_t t0 = esp_timer_get_time();
        g_rnbo->process(noInputs, 0, outs, 2, kBlockSize);
        int64_t t1 = esp_timer_get_time();
        processUs += (t1 - t0);

        // rain: loop + one-pole lowpass (coef once per block is enough)
        if (g_rain.buffer != nullptr && g_rain.frames != 0) {
            size_t ph = g_rain.playhead;
            const float gain = g_rain.gain;
            const float cutoffHz = 200.0f + g_rainExtra.filterOpen * (12000.0f - 200.0f);
            const float alpha = 1.0f - expf(-2.0f * (float)M_PI * cutoffHz / (float)kSampleRate);
            float lpL = g_rainExtra.lpL, lpR = g_rainExtra.lpR;
            for (int i = 0; i < kBlockSize; i++) {
                const float sL = g_rain.buffer[ph * 2 + 0];
                const float sR = g_rain.buffer[ph * 2 + 1];
                lpL += alpha * (sL - lpL);
                lpR += alpha * (sR - lpR);
                outL[i] += lpL * gain;
                outR[i] += lpR * gain;
                ph++;
                if (ph >= g_rain.frames) ph = 0;
            }
            g_rain.playhead = ph;
            g_rainExtra.lpL = lpL;
            g_rainExtra.lpR = lpR;
        }

        // bird: wait, play one chirp, wait again
        if (g_bird.buffer != nullptr && g_bird.frames != 0) {
            const float gain = g_bird.gain;
            const bool hasDelay = (g_birdDelayL != nullptr && g_birdDelayR != nullptr);
            const float wet = g_birdExtra.space * 0.5f;
            const float feedback = g_birdExtra.space * 0.45f;
            for (int i = 0; i < kBlockSize; i++) {
                float dryL = 0.0f, dryR = 0.0f;
                if (g_birdExtra.active) {
                    const float ph = g_birdExtra.playheadF;
                    const size_t i0 = static_cast<size_t>(ph);
                    const size_t i1 = (i0 + 1 < g_bird.frames) ? i0 + 1 : i0;
                    const float frac = ph - static_cast<float>(i0);
                    const float sL = g_bird.buffer[i0 * 2 + 0] * (1.0f - frac) + g_bird.buffer[i1 * 2 + 0] * frac;
                    const float sR = g_bird.buffer[i0 * 2 + 1] * (1.0f - frac) + g_bird.buffer[i1 * 2 + 1] * frac;
                    dryL = sL * gain;
                    dryR = sR * gain;
                    g_birdExtra.playheadF += g_birdExtra.playbackRate;
                    if (g_birdExtra.playheadF >= static_cast<float>(g_bird.frames)) {
                        g_birdExtra.active = false;
                        // 2 s to 20 s between chirps, with some randomness
                        const float avgGapSec = 20.0f - g_birdExtra.howoften * 18.0f;
                        const float gapSec = avgGapSec * (0.6f + 0.8f * (random(1000) / 1000.0f));
                        g_birdExtra.framesUntilNext = static_cast<uint32_t>(gapSec * kSampleRate);
                    }
                } else if (g_birdExtra.framesUntilNext == 0) {
                    g_birdExtra.active = true;
                    g_birdExtra.playheadF = 0.0f;
                    // 0.88x - 1.12x speed so it doesn't sound like the same recording every time
                    g_birdExtra.playbackRate = 0.88f + 0.24f * (random(1000) / 1000.0f);
                } else {
                    g_birdExtra.framesUntilNext--;
                }

                if (hasDelay) {
                    const size_t readPos = (g_birdDelayWritePos + kMaxDelayFrames / 2) % kMaxDelayFrames;
                    const float delayedL = g_birdDelayL[readPos];
                    const float delayedR = g_birdDelayR[readPos];
                    g_birdDelayL[g_birdDelayWritePos] = dryL + delayedL * feedback;
                    g_birdDelayR[g_birdDelayWritePos] = dryR + delayedR * feedback;
                    g_birdDelayWritePos = (g_birdDelayWritePos + 1) % kMaxDelayFrames;
                    outL[i] += dryL + delayedL * wet;
                    outR[i] += dryR + delayedR * wet;
                } else {
                    outL[i] += dryL;
                    outR[i] += dryR;
                }
            }
        }

        // once a second: peak + how much of the 2.9 ms block budget process() uses
        if (++debugBlockCounter >= kBlocksPerPrint) {
            debugBlockCounter = 0;
            float peak = 0.0f;
            for (int i = 0; i < kBlockSize; i++) {
                if (fabsf(outL[i]) > peak) peak = fabsf(outL[i]);
                if (fabsf(outR[i]) > peak) peak = fabsf(outR[i]);
            }
            int budgetUs = (int)(kBlockSize * 1000000.0 / kSampleRate);
            int64_t avgProcess = processUs / kBlocksPerPrint;
            int64_t avgWrite = writeUs / kBlocksPerPrint;
            Serial.printf(
                "[audio] block peak = %.6f  avg process=%lldus avg write=%lldus (budget=%dus, process headroom=%.0f%%)\n",
                peak, avgProcess, avgWrite, budgetUs,
                100.0 * (1.0 - (double)avgProcess / (double)budgetUs));
            processUs = 0;
            writeUs = 0;
        }

        for (int i = 0; i < kBlockSize; i++) {
            interleaved[i * 2 + 0] = floatToInt16(outL[i]);
            interleaved[i * 2 + 1] = floatToInt16(outR[i]);
        }

        int64_t t2 = esp_timer_get_time();
        g_i2s.writeInterleaved(interleaved, kBlockSize);
        int64_t t3 = esp_timer_get_time();
        writeUs += (t3 - t2);
    }
}

}  // namespace

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("[boot] step0 Serial up");
    Serial.flush();

    WiFi.mode(WIFI_OFF);
    btStop();
    Serial.println("[boot] step0a WiFi/BT off");
    Serial.flush();

    RNBO::PatcherFactoryFunctionPtr factory = GetPatcherFactoryFunction();
    RNBO::UniquePtr<RNBO::PatcherInterface> patcher(factory());
    static RNBO::CoreObject rnbo(std::move(patcher));
    g_rnbo = &rnbo;
    Serial.println("[boot] step1 CoreObject constructed");
    Serial.flush();

    if (!g_rnbo->prepareToProcess(kSampleRate, kBlockSize)) {
        Serial.println("prepareToProcess failed");
        return;
    }
    Serial.println("[boot] step1b prepareToProcess ok");
    Serial.flush();

    for (const char* paramId : {"noisevolume", "cracklevolume"}) {
        RNBO::ParameterIndex idx = g_rnbo->getParameterIndexForID(paramId);
        if (idx != RNBO::INVALID_INDEX) {
            g_rnbo->setParameterValue(idx, -24.0f);
        }
    }
    // RNBO doesn't push default param values downstream until they're set once,
    // so set each one to itself
    for (const char* paramId : {"noisewarmth", "noisebrightness", "cracklefrequency"}) {
        RNBO::ParameterIndex idx = g_rnbo->getParameterIndexForID(paramId);
        if (idx != RNBO::INVALID_INDEX) {
            g_rnbo->setParameterValue(idx, g_rnbo->getParameterValue(idx));
        }
    }
    Serial.println("[boot] step2 initial parameter values pushed");
    Serial.flush();

    g_knob1.begin(*g_rnbo, g_samplePool, AmbienceKnobs::NoiseSlots(),
                  AmbienceKnobs::kNoiseSlotCount, "Noise");
    g_knobEncoder1.begin(kKnob1ClkPin, kKnob1DtPin, kKnob1SwPin);
    g_knobEncoder2.begin(kKnob2ClkPin, kKnob2DtPin, kKnob2SwPin);
    g_knobEncoder3.begin(kKnob3ClkPin, kKnob3DtPin, kKnob3SwPin);
    Serial.println("[boot] step3 knob group + encoders begin done");
    Serial.flush();

    if (!g_oled.begin()) {
        Serial.println("OLED init failed - check I2C wiring / address 0x3C");
    }
    g_displayGroupLabel = "Noise";
    g_displaySlotLabel = g_knob1.currentSlotLabel();
    g_displayValue = g_knob1.currentValue();
    g_displayNormalized = g_knob1.currentValueNormalized();
    Serial.println("[boot] step4 oled begin done");
    Serial.flush();

    if (!g_i2s.begin(static_cast<int>(kSampleRate))) {
        Serial.println("I2S init failed - check pins in I2SOutput.h");
    }
    Serial.println("[boot] step5 i2s begin done");
    Serial.flush();

    // ~124 KB, too big for DRAM
    g_birdDelayL = static_cast<float*>(heap_caps_malloc(kMaxDelayFrames * sizeof(float), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    g_birdDelayR = static_cast<float*>(heap_caps_malloc(kMaxDelayFrames * sizeof(float), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (g_birdDelayL == nullptr || g_birdDelayR == nullptr) {
        Serial.println("[boot] bird delay buffer PSRAM alloc failed - space will be silent");
    } else {
        memset(g_birdDelayL, 0, kMaxDelayFrames * sizeof(float));
        memset(g_birdDelayR, 0, kMaxDelayFrames * sizeof(float));
    }

    swapLoopAsset(g_rain, kRainAssetPaths[g_rainKnob.typeIndex], /*applyLoopCrossfade=*/true);
    swapLoopAsset(g_bird, kBirdAssetPaths[g_birdKnob.typeIndex], /*applyLoopCrossfade=*/false);
    Serial.println("[boot] step5b rain+bird sample load attempt done");
    Serial.flush();

    // audio on core 0, otherwise it starves loop() on core 1
    esp_pthread_cfg_t audioThreadCfg = esp_pthread_get_default_config();
    audioThreadCfg.stack_size = 8192;
    audioThreadCfg.prio = configMAX_PRIORITIES - 1;
    audioThreadCfg.pin_to_core = 0;
    audioThreadCfg.thread_name = "rnbo_audio";
    esp_pthread_set_cfg(&audioThreadCfg);
    pthread_create(&g_audioThread, nullptr, audioTask, nullptr);

    // stop the watchdog from resetting because the idle task never gets to run
    esp_task_wdt_config_t twdtConfig = {
        .timeout_ms = 5000,
        .idle_core_mask = 0,
        .trigger_panic = true,
    };
    esp_task_wdt_reconfigure(&twdtConfig);

    Serial.println("[boot] step6 audioTask created, setup() done");
    Serial.flush();
}

void loop() {
    static bool loggedFirstLoop = false;
    if (!loggedFirstLoop) {
        Serial.println("[boot] step8 first loop() iteration");
        Serial.flush();
        loggedFirstLoop = true;
    }

    // one shared button (on knob 1's SW pin, GPIO21) for all three knobs
    const int d1 = g_knobEncoder1.readAndClearDelta();
    const bool sharedButtonPressed = g_knobEncoder1.pressedEdge();
    if (d1 != 0) g_knob1.onRotate(d1);
    if (d1 != 0) {
        g_displayGroupLabel = "Noise";
        g_displaySlotLabel = g_knob1.currentSlotLabel();
        g_displayValue = g_knob1.currentValue();
        g_displayNormalized = g_knob1.currentValueNormalized();
        g_displayDirty = true;
    }

    // hold 3 s to restart, since the case covers the EN button
    {
        static unsigned long restartHoldStartMs = 0;
        constexpr unsigned long kRestartHoldMs = 3000;
        if (g_knobEncoder1.isPressed()) {
            if (restartHoldStartMs == 0) {
                restartHoldStartMs = millis();
            } else if (millis() - restartHoldStartMs >= kRestartHoldMs) {
                Serial.println("[restart] Knob1 held 3s - software restarting...");
                Serial.flush();
                delay(50);
                ESP.restart();
            }
        } else {
            restartHoldStartMs = 0;
        }
    }

    // knob 2/3 SW pins aren't wired, so only read rotation
    const int d2 = g_knobEncoder2.readAndClearDelta();
    if (d2 != 0) g_rainKnob.onRotate(d2);
    if (d2 != 0) {
        g_displayGroupLabel = "Rain";
        g_displaySlotLabel = g_rainKnob.slotLabel();
        g_displayValue = g_rainKnob.slotValue();
        g_displayNormalized = g_rainKnob.slotNormalized();
        g_displayDirty = true;
    }

    const int d3 = g_knobEncoder3.readAndClearDelta();
    if (d3 != 0) g_birdKnob.onRotate(d3);
    if (d3 != 0) {
        g_displayGroupLabel = "Bird";
        g_displaySlotLabel = g_birdKnob.slotLabel();
        g_displayValue = g_birdKnob.slotValue();
        g_displayNormalized = g_birdKnob.slotNormalized();
        g_displayDirty = true;
    }

    // button goes to whichever knob was turned last
    if (sharedButtonPressed) {
        if (strcmp(g_displayGroupLabel, "Rain") == 0) {
            g_rainKnob.onPress();
            g_displaySlotLabel = g_rainKnob.slotLabel();
            g_displayValue = g_rainKnob.slotValue();
            g_displayNormalized = g_rainKnob.slotNormalized();
        } else if (strcmp(g_displayGroupLabel, "Bird") == 0) {
            g_birdKnob.onPress();
            g_displaySlotLabel = g_birdKnob.slotLabel();
            g_displayValue = g_birdKnob.slotValue();
            g_displayNormalized = g_birdKnob.slotNormalized();
        } else {
            g_knob1.onPress();
            g_displaySlotLabel = g_knob1.currentSlotLabel();
            g_displayValue = g_knob1.currentValue();
            g_displayNormalized = g_knob1.currentValueNormalized();
        }
        g_displayDirty = true;
    }

    if (g_displayDirty) {
        g_oled.showRaw(g_displayGroupLabel, g_displaySlotLabel, g_displayValue, g_displayNormalized);
        g_displayDirty = false;
    }

    delay(2);
}
