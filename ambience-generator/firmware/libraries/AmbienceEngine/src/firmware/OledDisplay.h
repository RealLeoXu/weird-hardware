#pragma once

#include "KnobGroup.h"

// SSD1306 128x64
#define OLED_SDA_PIN 8
#define OLED_SCL_PIN 9
#define OLED_I2C_ADDRESS 0x3C

class OledDisplay {
public:
    bool begin();

    void showKnob(const KnobGroup& knob);

    void showRaw(const char* groupLabel, const char* slotLabel, float value, float normalized);
};
