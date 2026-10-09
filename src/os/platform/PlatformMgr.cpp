#include "os/platform/PlatformMgr.h"

#include <array>
#include <cstddef>

#include "os/platform/platform_adapters.h"

namespace {

constexpr std::array<const char*, kNumPlatforms> kPlatformNames = {
    "", "", "", "pc", "", "xb1", "", "ps4", "android", "ios", "osx",
    "tvos", "nx",
};

constexpr std::array<const char*, kNumGfxApis> kGfxApiNames = {
    "null", "dx11", "ps4", "mtl", "vlk", "nx", "gles3",
};

}  // namespace

// Reconstructed from eboot.elf at 0x363030.
const char* PlatformSymbol(HxPlatform platform) {
    const auto index = static_cast<std::size_t>(platform);
    return index < kPlatformNames.size() ? kPlatformNames[index] : "";
}

// Reconstructed from eboot.elf at 0x1AE4D0.
const char* GfxApiSymbol(HxGfxApi api) {
    const auto index = static_cast<std::size_t>(api);
    return index < kGfxApiNames.size() ? kGfxApiNames[index] : "";
}

// Reconstructed from eboot.elf at 0x3641B0.
std::vector<std::uint32_t> GetSupportedPlatforms() {
    return rb4::render_configured_supported_platform_ids(
        "platform_mgr", "supported_platforms");
}
