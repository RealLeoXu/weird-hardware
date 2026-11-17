#pragma once

#include <cstdint>
#include <cstddef>

#include "../rnbo/src/RNBO_CoreObject.h"
#include "asset_manifest.h"

class RnboSamplePool {
public:

    bool begin(RNBO::CoreObject& rnbo);

    bool setRainType(int index);
    bool setBirdType(int index);

    int currentRainType() const { return _currentRainType; }
    int currentBirdType() const { return _currentBirdType; }

    size_t residentPsramBytes() const { return _residentBytes; }

private:
    bool loadAndSet(RNBO::ExternalDataId memoryId,
                     const AmbienceAssets::AssetInfo& asset,
                     size_t& residentCounterToUpdate);

    RNBO::CoreObject* _rnbo = nullptr;
    RNBO::ParameterIndex _raintypeParamIndex = -1;
    RNBO::ParameterIndex _birdtypeParamIndex = -1;

    int _currentRainType = -1;
    int _currentBirdType = -1;

    size_t _rainResidentBytes = 0;
    size_t _birdResidentBytes = 0;
    size_t _residentBytes = 0;

    bool _fsReady = false;
};
