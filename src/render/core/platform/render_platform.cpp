#include "render/core/platform/render_platform.h"

#include <array>
#include <cstring>

#include "render/core/platform/render_platform_adapters.h"

namespace rb4 {

namespace {

constexpr std::array<const char*, 13> kPlatformNames = {
    "", "", "", "pc", "", "xb1", "", "ps4", "android", "ios", "osx",
    "tvos", "nx",
};

constexpr std::array<const char*, 7> kRenderApiNames = {
    "null", "dx11", "ps4", "mtl", "vlk", "nx", "gles3",
};

}  // namespace

// Reconstructed from eboot.elf at 0x363030.
const char* render_platform_name(RenderPlatform platform) {
    const auto index = static_cast<std::size_t>(platform);
    return index < kPlatformNames.size() ? kPlatformNames[index] : "";
}

// Reconstructed from eboot.elf at 0x1AE4D0.
const char* render_api_name(RenderApi api) {
    const auto index = static_cast<std::size_t>(api);
    return index < kRenderApiNames.size() ? kRenderApiNames[index] : "";
}

// Reconstructed from eboot.elf at 0x4414A0.
RenderApi render_api_for_platform(RenderPlatform platform) {
    const auto* configured_name =
        render_configured_api_name(render_platform_name(platform));
    if (configured_name == nullptr) {
        return RenderApi::kNull;
    }

    for (std::size_t index = 0; index < kRenderApiNames.size(); ++index) {
        if (std::strcmp(configured_name, kRenderApiNames[index]) == 0) {
            return static_cast<RenderApi>(index);
        }
    }
    return RenderApi::kNull;
}

// Reconstructed from eboot.elf at 0x8D5DE0.
RenderApi orbis_render_api() {
    return RenderApi::kPlayStation4;
}

// Reconstructed from eboot.elf at 0x3641B0.
std::vector<std::uint32_t> render_supported_platform_ids() {
    return render_configured_supported_platform_ids(
        "platform_mgr", "supported_platforms");
}

}  // namespace rb4
