#pragma once
#include <cstddef>

namespace AmbienceAssets {

constexpr int kSampleRate = 11025;
constexpr int kChannels = 2;
constexpr int kBitsPerSample = 16;

struct AssetInfo {
    const char* path;
    size_t frames;
    size_t bytesOnFlash;
    size_t bytesExpandedF32;
};

constexpr AssetInfo kRainAssets[3] = {
    { "/rain_0.pcm", 81504u, 326016u, 652032u },
    { "/rain_1.pcm", 110880u, 443520u, 887040u },
    { "/rain_2.pcm", 88992u, 355968u, 711936u },
};

constexpr AssetInfo kBirdAssets[3] = {
    { "/bird_0.pcm", 44064u, 176256u, 352512u },
    { "/bird_1.pcm", 32832u, 131328u, 262656u },
    { "/bird_2.pcm", 34272u, 137088u, 274176u },
};

}
