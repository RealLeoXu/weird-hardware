#include "OledDisplay.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <cstdio>

namespace {
constexpr int kScreenWidth = 128;
constexpr int kScreenHeight = 64;
Adafruit_SSD1306 display(kScreenWidth, kScreenHeight, &Wire, -1 );
}

bool OledDisplay::begin() {
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
        return false;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
    return true;
}

void OledDisplay::showKnob(const KnobGroup& knob) {
    showRaw(knob.groupLabel(), knob.currentSlotLabel(), knob.currentValue(), knob.currentValueNormalized());
}

void OledDisplay::showRaw(const char* groupLabel, const char* slotLabel, float value, float normalized) {
    display.clearDisplay();

    display.setTextWrap(false);

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print(groupLabel);

    display.setTextSize(1);
    display.setCursor(0, 20);
    display.print(slotLabel);

    char valueStr[24];
    snprintf(valueStr, sizeof(valueStr), "%.2f", value);
    display.setCursor(0, 32);
    display.print(valueStr);

    const int barX = 0, barY = 48, barW = kScreenWidth - 2 * barX, barH = 8;
    display.drawRect(barX, barY, barW, barH, SSD1306_WHITE);
    const int fillW = static_cast<int>(barW * normalized);
    if (fillW > 0) {
        display.fillRect(barX, barY, fillW, barH, SSD1306_WHITE);
    }

    display.display();
}
