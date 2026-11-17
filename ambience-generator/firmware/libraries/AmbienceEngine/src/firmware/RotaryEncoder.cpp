#include "RotaryEncoder.h"

#include <Arduino.h>
#include <esp_attr.h>

namespace {

constexpr uint8_t R_START = 0x0;
constexpr uint8_t R_CW_FINAL = 0x1;
constexpr uint8_t R_CW_BEGIN = 0x2;
constexpr uint8_t R_CW_NEXT = 0x3;
constexpr uint8_t R_CCW_BEGIN = 0x4;
constexpr uint8_t R_CCW_FINAL = 0x5;
constexpr uint8_t R_CCW_NEXT = 0x6;
constexpr uint8_t DIR_NONE = 0x0;
constexpr uint8_t DIR_CW = 0x10;
constexpr uint8_t DIR_CCW = 0x20;

const DRAM_ATTR uint8_t kTransitionTable[7][4] = {
    {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
    {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
    {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
    {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
    {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
    {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
    {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};

constexpr unsigned long kDebounceMs = 15;
}

void RotaryEncoder::begin(int clkPin, int dtPin, int swPin) {
    _clkPin = clkPin;
    _dtPin = dtPin;
    _swPin = swPin;

    pinMode(_clkPin, INPUT_PULLUP);
    pinMode(_dtPin, INPUT_PULLUP);
    pinMode(_swPin, INPUT_PULLUP);

    _state = R_START;

    attachInterruptArg(digitalPinToInterrupt(_clkPin), isrTrampoline, this, CHANGE);
    attachInterruptArg(digitalPinToInterrupt(_dtPin), isrTrampoline, this, CHANGE);
}

void IRAM_ATTR RotaryEncoder::isrTrampoline(void* arg) {
    static_cast<RotaryEncoder*>(arg)->handleInterrupt();
}

void IRAM_ATTR RotaryEncoder::handleInterrupt() {
    const uint8_t pinState = (digitalRead(_clkPin) << 1) | digitalRead(_dtPin);
    _state = kTransitionTable[_state & 0x0F][pinState];
    const uint8_t dir = _state & 0x30;
    if (dir == DIR_CW) {
        _position++;
    } else if (dir == DIR_CCW) {
        _position--;
    }
}

int RotaryEncoder::readAndClearDelta() {
    noInterrupts();
    int32_t p = _position;
    _position = 0;
    interrupts();
    return static_cast<int>(p);
}

bool RotaryEncoder::pressedEdge() {
    const bool rawPressed = (digitalRead(_swPin) == LOW);
    const unsigned long now = millis();

    if (rawPressed != _lastPressedRaw) {
        _lastEdgeMs = now;
        _lastPressedRaw = rawPressed;
    }

    if ((now - _lastEdgeMs) >= kDebounceMs && _debouncedPressed != _lastPressedRaw) {
        _debouncedPressed = _lastPressedRaw;
        return _debouncedPressed;
    }
    return false;
}
