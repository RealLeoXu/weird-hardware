#pragma once

#include <cstdint>
#include "../rnbo/src/RNBO_CoreObject.h"
#include "RnboSamplePool.h"

class KnobGroup {
public:
    enum class SlotKind { Continuous, RainSelect, BirdSelect };

    struct Slot {
        const char* label;
        SlotKind kind;
        RNBO::ParameterIndex paramIndex;
        float minV;
        float maxV;
        float stepPerDetent;
    };

    void begin(RNBO::CoreObject& rnbo, RnboSamplePool& samplePool,
               const Slot* slots, int slotCount, const char* groupLabel);

    void onRotate(int delta);

    void onPress();

    const char* groupLabel() const { return _groupLabel; }
    const char* currentSlotLabel() const { return _slots[_currentSlot].label; }
    float currentValue() const { return _currentValueCache; }

    float currentValueNormalized() const;

private:
    void applyCurrentSlotValue(float newValue);
    void refreshCacheFromEngine();

    RNBO::CoreObject* _rnbo = nullptr;
    RnboSamplePool* _samplePool = nullptr;
    const Slot* _slots = nullptr;
    int _slotCount = 0;
    int _currentSlot = 0;
    const char* _groupLabel = "";
    float _currentValueCache = 0.0f;
};

namespace AmbienceKnobs {

inline KnobGroup::Slot* NoiseSlots() {
    static KnobGroup::Slot slots[] = {
        { "noisevolume",      KnobGroup::SlotKind::Continuous, RNBO::INVALID_INDEX, -60.0f, 0.0f, 2.0f },
        { "noisewarmth",      KnobGroup::SlotKind::Continuous, RNBO::INVALID_INDEX,   0.0f, 1.0f, 0.04f },
        { "noisebrightness",  KnobGroup::SlotKind::Continuous, RNBO::INVALID_INDEX,  36.0f, 127.0f, 2.0f },
        { "cracklefrequency", KnobGroup::SlotKind::Continuous, RNBO::INVALID_INDEX,   0.0f, 1.0f, 0.04f },
        { "cracklevolume",    KnobGroup::SlotKind::Continuous, RNBO::INVALID_INDEX, -60.0f, 0.0f, 2.0f },
    };
    return slots;
}
constexpr int kNoiseSlotCount = 5;

inline KnobGroup::Slot* RainSlots() {
    static KnobGroup::Slot slots[] = {
        { "rainvolume", KnobGroup::SlotKind::Continuous, RNBO::INVALID_INDEX, -60.0f, 0.0f, 2.0f },
        { "rainfilter", KnobGroup::SlotKind::Continuous, RNBO::INVALID_INDEX,   0.0f, 1.0f, 0.04f },
        { "raintype",   KnobGroup::SlotKind::RainSelect, RNBO::INVALID_INDEX,   0.0f, 2.0f, 1.0f },
    };
    return slots;
}
constexpr int kRainSlotCount = 3;

inline KnobGroup::Slot* BirdSlots() {
    static KnobGroup::Slot slots[] = {
        { "birdvolume", KnobGroup::SlotKind::Continuous, RNBO::INVALID_INDEX, -60.0f, 0.0f, 2.0f },
        { "howoften",   KnobGroup::SlotKind::Continuous, RNBO::INVALID_INDEX,   0.0f, 1.0f, 0.04f },
        { "space",      KnobGroup::SlotKind::Continuous, RNBO::INVALID_INDEX,   0.0f, 1.0f, 0.04f },
        { "birdtype",   KnobGroup::SlotKind::BirdSelect, RNBO::INVALID_INDEX,   0.0f, 2.0f, 1.0f },
    };
    return slots;
}
constexpr int kBirdSlotCount = 4;

}
