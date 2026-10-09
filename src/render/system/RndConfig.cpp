#include "render/system/RndConfig.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

#include "utl/options/Option.h"
#include "render/system/RndCapabilities.h"
#include "render/system/RndDevice.h"
#include "os/system/System.h"
#include "utl/data/DataArray.h"

namespace {

constexpr std::size_t kCurrentPlatformConfigIndex = 7;
constexpr std::uint32_t kAsyncComputeFeature = 0x10;

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

// Reads a 32-bit value into a 64-bit setting, sign-extending it.
void FindInt64(const DataArray* config, const char* key, std::int64_t& destination) {
    auto value = static_cast<int>(destination);
    config->FindData(Symbol(key), value, false);
    destination = value;
}

}  // namespace

std::int32_t RndConfig::ActiveVSyncMode() const {
    return mVSyncEnabled ? mVSyncMode : 0;
}

// Reconstructed from eboot.elf at 0x4414A0.
HxGfxApi RndGfxApiForPlatform(HxPlatform platform) {
    const Symbol api =
        SystemConfig(Symbol("rnd"), PlatformSymbol(platform), Symbol("api"))->Sym(1);
    for (std::uint32_t index = 0; index < kNumGfxApis; ++index) {
        if (GfxApiSymbol(static_cast<HxGfxApi>(index)) == api) {
            return static_cast<HxGfxApi>(index);
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
// defaults; the rnd config block then overrides them, the platform's
// capabilities clamp them, and the -resolution option picks a supported
// output resolution.
RndConfig::RndConfig() {
    auto& capabilities = TheRndDevice()->mCapabilities[kCurrentPlatformConfigIndex];
    const auto* config = SystemConfig(Symbol("rnd"));
    mContentResolution.x = config->FindArray(Symbol("content_resolution"), false)->Int(1);
    mContentResolution.y = config->FindArray(Symbol("content_resolution"), false)->Int(2);
    config->FindData(Symbol("pc_init_fullscreen"), mPcFullscreen, false);
    mPcWindowResolution.x = config->FindArray(Symbol("pc_init_window_resolution"), false)->Int(1);
    mPcWindowResolution.y = config->FindArray(Symbol("pc_init_window_resolution"), false)->Int(2);
    config->FindData(Symbol("vsync_mode"), mVSyncMode, false);
    config->FindData(Symbol("use_lod"), mUseLod, false);
    config->FindData(Symbol("use_gbuffer_vertex_normals"), mUseGBufferVertexNormals, false);
    config->FindData(Symbol("use_64_bit_light_accum"), mUse64BitLightAccum, false);
    config->FindData(Symbol("use_40_bit_depth_stencil"), mUse40BitDepthStencil, false);
    config->FindData(Symbol("use_tiled_lighting"), mUseTiledLighting, false);
    FindInt64(config, "max_partial_framerate_scenes", mMaxPartialFramerateScenes);
    FindInt64(config, "max_shadow_contrib_buffers", mMaxShadowContribBuffers);
    mPartialFramerateEnabled = mMaxPartialFramerateScenes != 0;

    Symbol quality("");
    config->FindData(Symbol("quality_level"), quality, false);
    if (quality != Symbol()) {
        mQualityLevel = RndQualityLevelFromName(quality.Str());
    }
    config->FindData(Symbol("scene_mask_enabled"), mSceneMaskEnabled, false);
    config->FindData(
        Symbol("multi_threaded_rendering_enabled"), mMultithreadedRenderingEnabled, false);
    config->FindData(Symbol("async_compute_enabled"), mAsyncComputeEnabled, false);
    FindInt64(config, "max_geo_overdraw", mMaxGeoOverdraw);
    FindInt64(config, "max_lighting_overdraw", mMaxLightingOverdraw);
    FindInt64(config, "max_light_probe_overdraw", mMaxLightProbeOverdraw);

    const auto* validation = config->FindArray(Symbol("graphics_api_validation"), false);
    validation->FindData(Symbol("enabled"), mGraphicsApiValidationEnabled, false);
    validation->FindData(Symbol("break_on_warning"), mBreakOnGraphicsWarning, false);
    validation->FindData(Symbol("break_on_error"), mBreakOnGraphicsError, false);
    config->FindArray(Symbol("graphics_debugger"), false)
        ->FindData(Symbol("enabled"), mGraphicsDebuggerEnabled, false);
    config->FindArray(Symbol("graphics_barrier_validation"), false)
        ->FindData(Symbol("enabled"), mGraphicsBarrierValidationEnabled, false);
    const auto* shaders = config->FindArray(Symbol("shader_compilation"), false);
    shaders->FindData(Symbol("print"), mPrintShaderCompilation, false);
    shaders->FindData(Symbol("print_verbose"), mPrintVerboseShaderCompilation, false);
    shaders->FindData(Symbol("output_intermediates"), mOutputShaderIntermediates, false);
    shaders->FindData(Symbol("generate_debug_info"), mGenerateShaderDebugInfo, false);

    if ((capabilities.mFeatureFlags & kAsyncComputeFeature) != 0) {
        if (mAsyncComputeEnabled) {
            mMultithreadedRenderingEnabled = false;
        }
    } else {
        mAsyncComputeEnabled = false;
        mUseTiledLighting = false;
    }

    // The override is accepted when the current resolution, not the
    // requested one, is supported.
    mOutputResolution = capabilities.mResolutions.back();
    if (const auto* text = OptionStr(gOptionArgs, "resolution", nullptr)) {
        Vector2i resolution{0, 0};
        if (ParseResolution(text, resolution)) {
            for (const auto& supported : capabilities.mResolutions) {
                if (supported.x == mOutputResolution.x && supported.y == mOutputResolution.y) {
                    mOutputResolution = resolution;
                    mPcWindowResolution = resolution;
                    mResolutionOverridden = true;
                    break;
                }
            }
        }
    }
}
