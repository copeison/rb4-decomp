#include "render_platform_config.h"

#include <algorithm>

#include "render_platform_config_adapters.h"
#include "render_settings.h"

namespace rb4 {

namespace {

constexpr RenderExtent kFallbackResolution{1920, 1080};

bool extent_less(RenderExtent left, RenderExtent right) {
    if (left.width != right.width) {
        return left.width < right.width;
    }
    return left.height < right.height;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B9940.
void render_platform_config_construct(RenderPlatformConfig& config) {
    render_platform_config_reset(config);
}

// Reconstructed from eboot.elf at 0x6B99B0.
void render_platform_config_initialize(
    RenderPlatformConfig& config,
    std::uint32_t platform_id) {
    const auto platform = static_cast<RenderPlatform>(platform_id);
    render_platform_config_apply_capabilities(config, platform);

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
    render_platform_config_set_resolutions(config, resolutions);
}

// Reconstructed from eboot.elf at 0x3641B0.
std::vector<std::uint32_t> render_supported_platform_ids() {
    return render_configured_supported_platform_ids(
        "platform_mgr", "supported_platforms");
}

}  // namespace rb4
