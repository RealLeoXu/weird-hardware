#include "RnboSamplePool.h"

#include <LittleFS.h>
#include <esp_heap_caps.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace {

constexpr size_t kChunkFrames = 2048;

constexpr float kInt16ToFloat = 1.0f / 32768.0f;
}

bool RnboSamplePool::begin(RNBO::CoreObject& rnbo) {
    _rnbo = &rnbo;

    if (!LittleFS.begin(false)) {

        printf("[RnboSamplePool] LittleFS mount failed\n");
        _fsReady = false;
        return false;
    }
    _fsReady = true;

    _raintypeParamIndex = _rnbo->getParameterIndexForID("raintype");
    _birdtypeParamIndex = _rnbo->getParameterIndexForID("birdtype");

    return true;
}

bool RnboSamplePool::loadAndSet(RNBO::ExternalDataId memoryId,
                                  const AmbienceAssets::AssetInfo& asset,
                                  size_t& residentCounterToUpdate) {
    if (!_fsReady || _rnbo == nullptr) {
        return false;
    }

    File f = LittleFS.open(asset.path, "r");
    if (!f) {
        printf("[RnboSamplePool] failed to open %s\n", asset.path);
        return false;
    }

    const size_t fileBytes = f.size();
    const size_t expectedBytes = asset.frames * AmbienceAssets::kChannels * (AmbienceAssets::kBitsPerSample / 8);
    if (fileBytes != expectedBytes) {

        printf("[RnboSamplePool] %s size mismatch: file=%u expected=%u (re-run convert_samples.py and reflash data)\n",
               asset.path, (unsigned)fileBytes, (unsigned)expectedBytes);
        f.close();
        return false;
    }

    const size_t destBytes = asset.frames * AmbienceAssets::kChannels * sizeof(float);
    float* dest = static_cast<float*>(heap_caps_malloc(destBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (dest == nullptr) {
        printf("[RnboSamplePool] PSRAM alloc failed for %s (%u bytes)\n", asset.path, (unsigned)destBytes);
        f.close();
        return false;
    }

    static int16_t chunk[kChunkFrames * AmbienceAssets::kChannels];
    size_t framesDone = 0;

    while (framesDone < asset.frames) {
        const size_t framesWanted = std::min(kChunkFrames, asset.frames - framesDone);
        const size_t bytesWanted = framesWanted * AmbienceAssets::kChannels * sizeof(int16_t);
        const size_t bytesGot = f.read(reinterpret_cast<uint8_t*>(chunk), bytesWanted);
        const size_t framesGot = bytesGot / (AmbienceAssets::kChannels * sizeof(int16_t));

        for (size_t i = 0; i < framesGot; i++) {
            const size_t outIdx = (framesDone + i) * AmbienceAssets::kChannels;
            dest[outIdx + 0] = static_cast<float>(chunk[i * 2 + 0]) * kInt16ToFloat;
            dest[outIdx + 1] = static_cast<float>(chunk[i * 2 + 1]) * kInt16ToFloat;
        }

        framesDone += framesGot;

        if (framesGot < framesWanted) {

            const size_t remainingFrames = asset.frames - framesDone;
            std::memset(dest + framesDone * AmbienceAssets::kChannels, 0,
                        remainingFrames * AmbienceAssets::kChannels * sizeof(float));
            break;
        }
    }

    f.close();

    RNBO::Float32AudioBuffer bufferType(AmbienceAssets::kChannels, AmbienceAssets::kSampleRate);

    _rnbo->setExternalData(
        memoryId,
        reinterpret_cast<char*>(dest),
        destBytes,
        bufferType,
        [](RNBO::ExternalDataId , char* oldData) {
            if (oldData != nullptr) {
                heap_caps_free(oldData);
            }
        });

    residentCounterToUpdate = destBytes;
    _residentBytes = _rainResidentBytes + _birdResidentBytes;

    printf("[RnboSamplePool] loaded %s -> %s (%.2fs, %u KB flash, %.2f MB psram)\n",
           asset.path, memoryId, asset.frames / float(AmbienceAssets::kSampleRate),
           (unsigned)(asset.bytesOnFlash / 1024), destBytes / 1024.0f / 1024.0f);

    return true;
}

bool RnboSamplePool::setRainType(int index) {
    if (index < 0 || index > 2) return false;
    if (index == _currentRainType) return true;

    const auto& asset = AmbienceAssets::kRainAssets[index];
    if (!loadAndSet("rainbuffer", asset, _rainResidentBytes)) {
        return false;
    }
    _currentRainType = index;

    if (_raintypeParamIndex >= 0) {
        _rnbo->setParameterValue(_raintypeParamIndex, static_cast<RNBO::ParameterValue>(index));
    }
    return true;
}

bool RnboSamplePool::setBirdType(int index) {
    if (index < 0 || index > 2) return false;
    if (index == _currentBirdType) return true;

    const auto& asset = AmbienceAssets::kBirdAssets[index];
    if (!loadAndSet("birdbuffer", asset, _birdResidentBytes)) {
        return false;
    }
    _currentBirdType = index;

    if (_birdtypeParamIndex >= 0) {
        _rnbo->setParameterValue(_birdtypeParamIndex, static_cast<RNBO::ParameterValue>(index));
    }
    return true;
}
