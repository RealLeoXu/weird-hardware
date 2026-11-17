#include "KnobGroup.h"

#include <cstring>

void KnobGroup::begin(RNBO::CoreObject& rnbo, RnboSamplePool& samplePool,
                       const Slot* slots, int slotCount, const char* groupLabel) {
    _rnbo = &rnbo;
    _samplePool = &samplePool;
    _slots = slots;
    _slotCount = slotCount;
    _groupLabel = groupLabel;
    _currentSlot = 0;

    for (int i = 0; i < _slotCount; i++) {
        Slot& s = const_cast<Slot&>(_slots[i]);
        if (s.kind == SlotKind::Continuous) {
            s.paramIndex = _rnbo->getParameterIndexForID(s.label);
        }
    }

    refreshCacheFromEngine();
}

void KnobGroup::refreshCacheFromEngine() {
    const Slot& s = _slots[_currentSlot];
    switch (s.kind) {
        case SlotKind::Continuous:
            _currentValueCache = (s.paramIndex != RNBO::INVALID_INDEX)
                ? static_cast<float>(_rnbo->getParameterValue(s.paramIndex))
                : 0.0f;
            break;
        case SlotKind::RainSelect:
            _currentValueCache = static_cast<float>(_samplePool->currentRainType());
            break;
        case SlotKind::BirdSelect:
            _currentValueCache = static_cast<float>(_samplePool->currentBirdType());
            break;
    }
}

void KnobGroup::applyCurrentSlotValue(float newValue) {
    const Slot& s = _slots[_currentSlot];

    if (newValue < s.minV) newValue = s.minV;
    if (newValue > s.maxV) newValue = s.maxV;

    switch (s.kind) {
        case SlotKind::Continuous:
            if (s.paramIndex != RNBO::INVALID_INDEX) {
                _rnbo->setParameterValue(s.paramIndex, static_cast<RNBO::ParameterValue>(newValue));
            }
            break;
        case SlotKind::RainSelect:

            _samplePool->setRainType(static_cast<int>(newValue + 0.5f));
            break;
        case SlotKind::BirdSelect:
            _samplePool->setBirdType(static_cast<int>(newValue + 0.5f));
            break;
    }

    _currentValueCache = newValue;
}

void KnobGroup::onRotate(int delta) {
    if (_slotCount == 0) return;
    const Slot& s = _slots[_currentSlot];
    float newValue = _currentValueCache + delta * s.stepPerDetent;
    applyCurrentSlotValue(newValue);
}

void KnobGroup::onPress() {
    if (_slotCount == 0) return;
    _currentSlot = (_currentSlot + 1) % _slotCount;
    refreshCacheFromEngine();
}

float KnobGroup::currentValueNormalized() const {
    if (_slotCount == 0) return 0.0f;
    const Slot& s = _slots[_currentSlot];
    if (s.maxV <= s.minV) return 0.0f;
    return (_currentValueCache - s.minV) / (s.maxV - s.minV);
}
