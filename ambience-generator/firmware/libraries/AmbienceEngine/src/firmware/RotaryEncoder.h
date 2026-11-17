#pragma once

#include <cstdint>
#include <esp_attr.h>

class RotaryEncoder {
public:
    void begin(int clkPin, int dtPin, int swPin);

    int readAndClearDelta();

    bool pressedEdge();

    bool isPressed() const { return _debouncedPressed; }

private:
    static void IRAM_ATTR isrTrampoline(void* arg);
    void IRAM_ATTR handleInterrupt();

    int _clkPin = -1, _dtPin = -1, _swPin = -1;

    volatile uint8_t _state = 0;
    volatile int32_t _position = 0;

    bool _lastPressedRaw = false;
    bool _debouncedPressed = false;
    unsigned long _lastEdgeMs = 0;
};
