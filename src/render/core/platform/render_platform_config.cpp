#include "render/core/platform/render_platform_config.h"

#include <algorithm>
#include <new>

#include "render/core/platform/render_platform_config_adapters.h"
#include "render/core/settings/render_settings.h"

namespace rb4 {

namespace {

constexpr RenderExtent kFallbackResolution{1920, 1080};
constexpr std::size_t kPlatformConfigOffset = 312;

bool extent_less(RenderExtent left, RenderExtent right) {
    if (left.width != right.width) {
        return left.width < right.width;
    }
    return left.height < right.height;
}

void set_uniform_capabilities(
    RenderPlatformConfig& config,
    std::uint64_t resource_tier,
    std::uint32_t feature_flags,
    std::uint64_t capability_mask) {
    config.resource_tier = resource_tier;
    config.feature_flags = feature_flags;
    for (auto& value : config.capability_values) {
        value = 16;
    }
    config.enabled = true;
    config.capability_mask[0] = capability_mask;
    config.capability_mask[1] = 0;
}

void set_restricted_capabilities(
    RenderPlatformConfig& config,
    std::uint64_t resource_tier,
    std::uint64_t capability_mask) {
    config.resource_tier = resource_tier;
    config.feature_flags = 9;
    config.capability_values[0] = 15;
    config.capability_values[1] = 0;
    config.capability_values[2] = 0;
    config.capability_values[3] = 15;
    config.capability_values[4] = 15;
    config.enabled = true;
    config.capability_mask[0] = capability_mask;
    config.capability_mask[1] = 0;
}

void apply_platform_capabilities(
    RenderPlatformConfig& config,
    RenderPlatform platform) {
    switch (static_cast<std::uint32_t>(platform)) {
    case 3:
        set_uniform_capabilities(
            config, 8, 31, 0x1A01FFFFFF7FCC3ULL);
        break;
    case 5:
    case 7:
        set_uniform_capabilities(
            config, 8, 31, 0x1601FFFFFF7FCC3ULL);
        break;
    case 8:
        set_uniform_capabilities(
            config, 8, 9, 0xFFBFE00007E57FC3ULL);
        config.capability_mask[1] = 0x1FFFFFULL;
        break;
    case 9:
    case 11:
        set_restricted_capabilities(
            config, 4, 0x11FE00007F79CC3ULL);
        break;
    case 10:
        set_restricted_capabilities(
            config, 8, 0x1201FFFFFF7FCC3ULL);
        break;
    case 12:
        set_uniform_capabilities(
            config, 8, 9, 0x1A01FFFFFF7FFC3ULL);
        break;
    default:
        break;
    }
}

}  // namespace

RenderPlatformConfig& render_system_platform_config_at(
    RenderSystem& system,
    std::size_t index) {
    auto* bytes = reinterpret_cast<std::uint8_t*>(&system);
    return *reinterpret_cast<RenderPlatformConfig*>(
        bytes + kPlatformConfigOffset + index * sizeof(RenderPlatformConfig));
}

// Reconstructed from eboot.elf at 0x6B9940.
void render_platform_config_construct(RenderPlatformConfig& config) {
    new (&config.resolutions) std::vector<RenderExtent>();
    config.resource_tier = ~std::uint64_t{0};
    config.feature_flags = 0;
    config.reserved_44 = 0;
    for (auto& value : config.capability_values) {
        value = 0;
    }
    config.enabled = false;
    for (auto& byte : config.reserved_89) {
        byte = 0;
    }
    config.resource_budget = 0x2000;
    config.option_flags = 0;
    config.reserved_108 = 0;
    config.capability_mask[0] = 0;
    config.capability_mask[1] = 0;
}

// Reconstructed from eboot.elf at 0x6B99B0.
void render_platform_config_initialize(
    RenderPlatformConfig& config,
    std::uint32_t platform_id) {
    const auto platform = static_cast<RenderPlatform>(platform_id);
    apply_platform_capabilities(config, platform);

    const auto configured_resolutions = render_configured_resolutions(
        render_platform_name(platform));
    std::vector<RenderExtent> resolutions;
    resolutions.reserve(configured_resolutions.size());
    for (const auto* text : configured_resolutions) {
        RenderExtent extent{};
        if (render_parse_resolution(text, extent)) {
            resolutions.push_back(extent);
        }
    }

    if (resolutions.empty()) {
        resolutions.push_back(kFallbackResolution);
    }
    std::sort(resolutions.begin(), resolutions.end(), extent_less);
    config.resolutions = resolutions;
}

void render_platform_config_destruct(RenderPlatformConfig& config) {
    using ResolutionList = std::vector<RenderExtent>;
    config.resolutions.~ResolutionList();
}

// Reconstructed from eboot.elf at 0x6B9FE0.
bool render_platform_config_boot_probe(const RenderPlatformConfig& config) {
    return config.resource_tier >= 4 &&
        (config.feature_flags & 1U) != 0 &&
        (config.feature_flags & 8U) != 0;
}

// Reconstructed from eboot.elf at 0x3641B0.
std::vector<std::uint32_t> render_supported_platform_ids() {
    return render_configured_supported_platform_ids(
        "platform_mgr", "supported_platforms");
}

}  // namespace rb4
