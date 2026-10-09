#include "render/system/RndCapabilities.h"

#include <algorithm>

#include "render/system/render_platform_adapters.h"
#include "render/system/RndConfig.h"

using namespace rb4;

namespace {

constexpr RenderExtent kFallbackResolution{1920, 1080};

bool ExtentLess(RenderExtent left, RenderExtent right) {
    if (left.width != right.width) {
        return left.width < right.width;
    }
    return left.height < right.height;
}

void SetUniformCapabilities(
    RndCapabilities& caps,
    std::uint64_t resourceTier,
    std::uint32_t featureFlags,
    std::uint64_t capabilityMask) {
    caps.mResourceTier = resourceTier;
    caps.mFeatureFlags = featureFlags;
    for (auto& value : caps.mCapabilityValues) {
        value = 16;
    }
    caps.mEnabled = true;
    caps.mCapabilityMask[0] = capabilityMask;
    caps.mCapabilityMask[1] = 0;
}

void SetRestrictedCapabilities(
    RndCapabilities& caps,
    std::uint64_t resourceTier,
    std::uint64_t capabilityMask) {
    caps.mResourceTier = resourceTier;
    caps.mFeatureFlags = 9;
    caps.mCapabilityValues[0] = 15;
    caps.mCapabilityValues[1] = 0;
    caps.mCapabilityValues[2] = 0;
    caps.mCapabilityValues[3] = 15;
    caps.mCapabilityValues[4] = 15;
    caps.mEnabled = true;
    caps.mCapabilityMask[0] = capabilityMask;
    caps.mCapabilityMask[1] = 0;
}

void ApplyPlatformCapabilities(RndCapabilities& caps, HxPlatform platform) {
    switch (static_cast<std::uint32_t>(platform)) {
    case 3:
        SetUniformCapabilities(caps, 8, 31, 0x1A01FFFFFF7FCC3ULL);
        break;
    case 5:
    case 7:
        SetUniformCapabilities(caps, 8, 31, 0x1601FFFFFF7FCC3ULL);
        break;
    case 8:
        SetUniformCapabilities(caps, 8, 9, 0xFFBFE00007E57FC3ULL);
        caps.mCapabilityMask[1] = 0x1FFFFFULL;
        break;
    case 9:
    case 11:
        SetRestrictedCapabilities(caps, 4, 0x11FE00007F79CC3ULL);
        break;
    case 10:
        SetRestrictedCapabilities(caps, 8, 0x1201FFFFFF7FCC3ULL);
        break;
    case 12:
        SetUniformCapabilities(caps, 8, 9, 0x1A01FFFFFF7FFC3ULL);
        break;
    default:
        break;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B9940.
RndCapabilities::RndCapabilities() {
    mResourceTier = ~std::uint64_t{0};
    mFeatureFlags = 0;
    mReserved44 = 0;
    for (auto& value : mCapabilityValues) {
        value = 0;
    }
    mEnabled = false;
    for (auto& byte : mReserved89) {
        byte = 0;
    }
    mResourceBudget = 0x2000;
    mOptionFlags = 0;
    mReserved108 = 0;
    mCapabilityMask[0] = 0;
    mCapabilityMask[1] = 0;
}

// Reconstructed from eboot.elf at 0x6B99B0.
void RndCapabilities::InitForPlatform(HxPlatform platform) {
    ApplyPlatformCapabilities(*this, platform);

    const auto configured_resolutions = render_configured_resolutions(
        render_platform_name(platform));
    std::vector<RenderExtent> resolutions;
    resolutions.reserve(configured_resolutions.size());
    for (const auto* text : configured_resolutions) {
        RenderExtent extent{};
        if (ParseResolution(text, extent)) {
            resolutions.push_back(extent);
        }
    }

    if (resolutions.empty()) {
        resolutions.push_back(kFallbackResolution);
    }
    std::sort(resolutions.begin(), resolutions.end(), ExtentLess);
    mResolutions = resolutions;
}

// Reconstructed from eboot.elf at 0x6B9FE0.
bool RndCapabilities::CheckMinimumRequirements() const {
    return mResourceTier >= 4 &&
        (mFeatureFlags & 1U) != 0 &&
        (mFeatureFlags & 8U) != 0;
}
