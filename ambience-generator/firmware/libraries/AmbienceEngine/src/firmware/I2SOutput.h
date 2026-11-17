#pragma once

#include <cstdint>
#include <cstddef>

// UDA1334A: BCLK 4, WSEL 5, DIN 6
#define I2S_BCLK_PIN   4
#define I2S_LRCLK_PIN  5
#if CONFIG_IDF_TARGET_ESP32  // GPIO6 is flash on the classic ESP32
#define I2S_DOUT_PIN   18
#else
#define I2S_DOUT_PIN   6
#endif

class I2SOutput {
public:

    bool begin(int sampleRate);

    void writeInterleaved(const int16_t* interleaved, size_t frameCount);
};
