#include "render/system/RndConfig.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

#include "utl/options/Option.h"
#include "render/system/RndCapabilities.h"
#include "render/system/RndDevice.h"
#include "render/system/rnd_config_adapters.h"
#include "os/platform/platform_adapters.h"

using namespace rb4;

namespace {

constexpr std::size_t kCurrentPlatformConfigIndex = 7;
constexpr std::uint32_t kAsyncComputeFeature = 0x10;

const RndCapabilities& CurrentCapabilities() {
    return TheRndDevice()->mCapabilities[kCurrentPlatformConfigIndex];
}

bool PlatformSupportsAsyncCompute() {
    return (CurrentCapabilities().mFeatureFlags & kAsyncComputeFeature) != 0;
}

Vector2i PlatformDefaultResolution() {
    return CurrentCapabilities().mResolutions.back();
}

bool PlatformSupportsResolution(Vector2i resolution) {
    for (const auto& supported : CurrentCapabilities().mResolutions) {
        if (supported.x == resolution.x && supported.y == resolution.y) {
            return true;
        }
    }
    return false;
}

bool EqualsIgnoreAsciiCase(const char* left, const char* right) {
    while (*left != '\0' && *right != '\0') {
        const auto left_char = static_cast<unsigned char>(*left++);
        const auto right_char = static_cast<unsigned char>(*right++);
        if (std::tolower(left_char) != std::tolower(right_char)) {
            return false;
        }
    }
    return *left == *right;
}

void ReadSignExtendedInt32(
    const DataConfig& config,
    const char* key,
    std::int64_t& destination) {
    auto value = static_cast<std::int32_t>(destination);
    config_read_int32(config, key, value);
    destination = value;
}

void ReadValidationSettings(RndConfig& settings, const DataConfig& root) {
    if (const auto* validation =
            config_find_block(root, "graphics_api_validation")) {
        config_read_bool(
            *validation, "enabled", settings.mGraphicsApiValidationEnabled);
        config_read_bool(
            *validation, "break_on_warning", settings.mBreakOnGraphicsWarning);
        config_read_bool(
            *validation, "break_on_error", settings.mBreakOnGraphicsError);
    }

    if (const auto* debugger = config_find_block(root, "graphics_debugger")) {
        config_read_bool(
            *debugger, "enabled", settings.mGraphicsDebuggerEnabled);
    }
    if (const auto* barrier =
            config_find_block(root, "graphics_barrier_validation")) {
        config_read_bool(
            *barrier, "enabled", settings.mGraphicsBarrierValidationEnabled);
    }
}

void ReadShaderCompilationSettings(
    RndConfig& settings,
    const DataConfig& root) {
    const auto* shader = config_find_block(root, "shader_compilation");
    if (shader == nullptr) {
        return;
    }

    config_read_bool(*shader, "print", settings.mPrintShaderCompilation);
    config_read_bool(
        *shader, "print_verbose", settings.mPrintVerboseShaderCompilation);
    config_read_bool(
        *shader, "output_intermediates", settings.mOutputShaderIntermediates);
    config_read_bool(
        *shader, "generate_debug_info", settings.mGenerateShaderDebugInfo);
}

void ReadRenderConfig(RndConfig& settings, const DataConfig& config) {
    config_read_extent(
        config, "content_resolution", settings.mContentResolution);
    config_read_bool(config, "pc_init_fullscreen", settings.mPcFullscreen);
    config_read_extent(
        config, "pc_init_window_resolution", settings.mPcWindowResolution);
    config_read_int32(config, "vsync_mode", settings.mVSyncMode);
    config_read_bool(config, "use_lod", settings.mUseLod);
    config_read_bool(
        config,
        "use_gbuffer_vertex_normals",
        settings.mUseGBufferVertexNormals);
    config_read_bool(
        config, "use_64_bit_light_accum", settings.mUse64BitLightAccum);
    config_read_bool(
        config, "use_40_bit_depth_stencil", settings.mUse40BitDepthStencil);
    config_read_bool(config, "use_tiled_lighting", settings.mUseTiledLighting);
    ReadSignExtendedInt32(
        config,
        "max_partial_framerate_scenes",
        settings.mMaxPartialFramerateScenes);
    ReadSignExtendedInt32(
        config,
        "max_shadow_contrib_buffers",
        settings.mMaxShadowContribBuffers);

    if (const auto* quality_name = config_read_string(config, "quality_level")) {
        settings.mQualityLevel = RndQualityLevelFromName(quality_name);
    }
    config_read_bool(
        config, "scene_mask_enabled", settings.mSceneMaskEnabled);
    config_read_bool(
        config,
        "multi_threaded_rendering_enabled",
        settings.mMultithreadedRenderingEnabled);
    config_read_bool(
        config, "async_compute_enabled", settings.mAsyncComputeEnabled);
    ReadSignExtendedInt32(
        config, "max_geo_overdraw", settings.mMaxGeoOverdraw);
    ReadSignExtendedInt32(
        config, "max_lighting_overdraw", settings.mMaxLightingOverdraw);
    ReadSignExtendedInt32(
        config,
        "max_light_probe_overdraw",
        settings.mMaxLightProbeOverdraw);

    ReadValidationSettings(settings, config);
    ReadShaderCompilationSettings(settings, config);
}

void ApplyPlatformLimits(RndConfig& settings) {
    settings.mPartialFramerateEnabled =
        settings.mMaxPartialFramerateScenes != 0;

    if (PlatformSupportsAsyncCompute()) {
        if (settings.mAsyncComputeEnabled) {
            settings.mMultithreadedRenderingEnabled = false;
        }
    } else {
        settings.mAsyncComputeEnabled = false;
        settings.mUseTiledLighting = false;
    }
}

}  // namespace

std::int32_t RndConfig::ActiveVSyncMode() const {
    return mVSyncEnabled ? mVSyncMode : 0;
}

// Reconstructed from eboot.elf at 0x4414A0.
HxGfxApi RndGfxApiForPlatform(HxPlatform platform) {
    const auto* configured_name =
        render_configured_api_name(PlatformSymbol(platform));
    if (configured_name == nullptr) {
        return kGfxApiNull;
    }

    for (std::uint32_t index = 0; index < kNumGfxApis; ++index) {
        const auto api = static_cast<HxGfxApi>(index);
        if (std::strcmp(configured_name, GfxApiSymbol(api)) == 0) {
            return api;
        }
    }
    return kGfxApiNull;
}

// Reconstructed from eboot.elf at 0x441940.
bool ParseResolution(const char* text, Vector2i& extent) {
    if (text == nullptr) {
        return false;
    }

    char* separator = nullptr;
    const auto first_value = std::strtol(text, &separator, 0);
    if (first_value <= 0) {
        return false;
    }

    if (*separator != 'x') {
        // The binary computes the width unsigned.
        extent.y = static_cast<int>(first_value);
        extent.x = static_cast<int>(
            16 * static_cast<std::uint32_t>(extent.y) / 9);
        return true;
    }

    const auto second_value = std::strtol(separator + 1, nullptr, 0);
    if (second_value <= 0) {
        return false;
    }

    extent.x = static_cast<int>(first_value);
    extent.y = static_cast<int>(second_value);
    return true;
}

// Reconstructed from eboot.elf at 0x442520.
const char* RndQualityLevelName(RndQualityLevel level) {
    switch (level) {
    case RndQualityLevel::kLow:
        return "Low";
    case RndQualityLevel::kMedium:
        return "Medium";
    case RndQualityLevel::kHigh:
        return "High";
    case RndQualityLevel::kInvalid:
        return nullptr;
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x442540.
RndQualityLevel RndQualityLevelFromName(const char* name) {
    if (name != nullptr) {
        if (EqualsIgnoreAsciiCase(name, "Low")) {
            return RndQualityLevel::kLow;
        }
        if (EqualsIgnoreAsciiCase(name, "Medium")) {
            return RndQualityLevel::kMedium;
        }
        if (EqualsIgnoreAsciiCase(name, "High")) {
            return RndQualityLevel::kHigh;
        }
    }
    return RndQualityLevel::kInvalid;
}

// Reconstructed from eboot.elf at 0x6BB470. The members start at their
// defaults; the rnd config block then overrides them.
RndConfig::RndConfig() {
    if (const auto* config = load_data_config("rnd")) {
        ReadRenderConfig(*this, *config);
    }
    ApplyPlatformLimits(*this);

    mOutputResolution = PlatformDefaultResolution();
    if (const auto* override_text =
            OptionStr(gOptionArgs, "resolution", nullptr)) {
        Vector2i override_resolution{};
        if (ParseResolution(override_text, override_resolution) &&
            PlatformSupportsResolution(mOutputResolution)) {
            mOutputResolution = override_resolution;
            mPcWindowResolution = override_resolution;
            mResolutionOverridden = true;
        }
    }
}
