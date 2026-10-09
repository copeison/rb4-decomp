#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "render/core/frame/render_extent.h"
#include "render/core/platform/render_platform.h"

// The engine's platform enumeration. It is reconstructed as
// rb4::RenderPlatform until the platform tables are converted. Name in the
// reference map; the alias is not.
using HxPlatform = rb4::RenderPlatform;

// The renderer's capabilities on one platform: its supported output
// resolutions, its resource tier and feature flags. RndDevice embeds one per
// platform. Field names are not in the reference map.
class RndCapabilities {
public:
    RndCapabilities();  // 0x6B9940
    void InitForPlatform(HxPlatform platform);  // 0x6B99B0
    bool CheckMinimumRequirements() const;  // 0x6B9FE0

    std::vector<rb4::RenderExtent> mResolutions;
    std::uint64_t mResourceTier;
    std::uint32_t mFeatureFlags;
    std::uint32_t mReserved44;
    std::uint64_t mCapabilityValues[5];
    bool mEnabled;
    std::uint8_t mReserved89[7];
    std::uint64_t mResourceBudget;
    std::uint32_t mOptionFlags;
    std::uint32_t mReserved108;
    std::uint64_t mCapabilityMask[2];
};

static_assert(offsetof(RndCapabilities, mResourceTier) == 32);
static_assert(offsetof(RndCapabilities, mFeatureFlags) == 40);
static_assert(offsetof(RndCapabilities, mEnabled) == 88);
static_assert(offsetof(RndCapabilities, mResourceBudget) == 96);
static_assert(offsetof(RndCapabilities, mCapabilityMask) == 112);
static_assert(sizeof(RndCapabilities) == 128);
