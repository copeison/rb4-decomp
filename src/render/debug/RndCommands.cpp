#include "render/debug/RndCommands.h"

#include <array>
#include <cstdint>

#include "render/core/capture/screenshot_capture.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/shaders/RndShaderMgr.h"

namespace {

RndDevice& render_system() {
    return *TheRndDevice();
}

RndConfig& render_settings() {
    return *render_system().mSettings;
}

void toggle(bool& value) {
    value = !value;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6BA880.
void RndCommands::_OnToggleVSync() {
    toggle(render_settings().mVSyncEnabled);
}

// Reconstructed from eboot.elf at 0x6BA8B0.
void RndCommands::_OnToggleSceneMask() {
    toggle(render_settings().mSceneMaskEnabled);
}

// Reconstructed from eboot.elf at 0x6BA8E0.
void RndCommands::_OnToggleShadows() {
    toggle(render_settings().mShadowsEnabled);
}

// Reconstructed from eboot.elf at 0x6BA910.
void RndCommands::_OnTogglePostProc() {
    toggle(render_settings().mPostProcEnabled);
}

// Reconstructed from eboot.elf at 0x6BA940.
void RndCommands::_OnToggleToneMapping() {
    toggle(render_settings().mTonemappingEnabled);
}

// Reconstructed from eboot.elf at 0x6BA970.
void RndCommands::_OnToggleVScat() {
    toggle(render_settings().mVolumetricScatteringEnabled);
}

// Reconstructed from eboot.elf at 0x6BA590.
void RndCommands::_OnReloadShaders() {
    render_system().mShaderMgr.ReloadAll();
}

// Reconstructed from eboot.elf at 0x6BAA40.
void RndCommands::_OnToggleMultithreadedRendering() {
    toggle(render_settings().mMultithreadedRenderingEnabled);
}

// Reconstructed from eboot.elf at 0x6BAA70.
void RndCommands::_OnToggleAsyncCompute() {
    toggle(render_settings().mAsyncComputeEnabled);
}

// Reconstructed from eboot.elf at 0x6BAAA0.
void RndCommands::_OnToggleAsyncCopy() {
    toggle(render_settings().mAsyncCopyEnabled);
}

// Reconstructed from eboot.elf at 0x6BAAD0.
void RndCommands::_OnToggleTiledLightInterpolation() {
    toggle(render_settings().mTiledLightInterpolationEnabled);
}

// Reconstructed from eboot.elf at 0x6BAB00.
void RndCommands::_OnTogglePartialFramerate() {
    auto& settings = render_settings();
    settings.mPartialFramerateEnabled =
        settings.mMaxPartialFramerateScenes != 0 &&
        !settings.mPartialFramerateEnabled;
}

// Reconstructed from eboot.elf at 0x6BAB40.
void RndCommands::_OnToggleStereoOptimizations() {
    toggle(render_settings().mStereoOptimizationsEnabled);
}

// Reconstructed from eboot.elf at 0x6BAB70.
void RndCommands::_OnToggle64BitLightAccum() {
    toggle(render_settings().mUse64BitLightAccum);
}

// Reconstructed from eboot.elf at 0x6BABA0.
void RndCommands::_OnToggleHdr() {
    auto& mode = render_system().mHdrOutputMode;
    mode = mode == 1 ? 0 : 1;
}

// Reconstructed from eboot.elf at 0x6BABD0.
void RndCommands::_OnTakeScreenshot() {
    rb4::screenshot_request();
}

// Reconstructed from eboot.elf at 0x6BAC00.
void RndCommands::_OnCycleScreenshotResolution() {
    auto& resolution = render_settings().mScreenshotResolution;
    const auto next =
        (static_cast<std::uint32_t>(resolution) + 1) % 6;
    resolution = static_cast<rb4::ScreenshotResolution>(next);
    static_cast<void>(rb4::screenshot_resolution_name(resolution));
}

namespace {

struct RndCommandDefinition {
    const char* name;
    RndCommands::Handler handler;
};

constexpr std::array<RndCommandDefinition, 24> kRndCommands = {{
    {"toggle_overlay", &RndCommands::_OnToggleOverlay},
    {"overlay_help", &RndCommands::_OnPrintOverlayHelp},
    {"reload_shaders", &RndCommands::_OnReloadShaders},
    {"set_resolution", &RndCommands::_OnSetResolution},
    {"set_quality_level", &RndCommands::_OnSetQualityLevel},
    {"toggle_vsync", &RndCommands::_OnToggleVSync},
    {"toggle_scene_mask", &RndCommands::_OnToggleSceneMask},
    {"toggle_shadows", &RndCommands::_OnToggleShadows},
    {"toggle_postproc", &RndCommands::_OnTogglePostProc},
    {"toggle_tonemapping", &RndCommands::_OnToggleToneMapping},
    {"toggle_vscat", &RndCommands::_OnToggleVScat},
    {"set_drawn_scene_range", &RndCommands::_OnSetDrawnSceneRange},
    {
        "toggle_multithreaded_rendering",
        &RndCommands::_OnToggleMultithreadedRendering,
    },
    {"toggle_async_compute", &RndCommands::_OnToggleAsyncCompute},
    {"toggle_async_copy", &RndCommands::_OnToggleAsyncCopy},
    {
        "toggle_tiled_light_interpolation",
        &RndCommands::_OnToggleTiledLightInterpolation,
    },
    {"toggle_partial_framerate", &RndCommands::_OnTogglePartialFramerate},
    {
        "toggle_stereo_optimizations",
        &RndCommands::_OnToggleStereoOptimizations,
    },
    {"toggle_64_bit_light_accum", &RndCommands::_OnToggle64BitLightAccum},
    {"toggle_hdr", &RndCommands::_OnToggleHdr},
    {"take_screenshot", &RndCommands::_OnTakeScreenshot},
    {
        "cycle_screenshot_resolution",
        &RndCommands::_OnCycleScreenshotResolution,
    },
    {"set_shading_mode", &RndCommands::_OnSetShadingMode},
    {
        "set_buffer_inspection_mode",
        &RndCommands::_OnSetBufferInspectionMode,
    },
}};

}  // namespace

// Reconstructed from eboot.elf at 0x6BB0E0.
void RndCommands::Init() {
    for (const auto& command : kRndCommands) {
        register_debug_command(command.name, command.handler);
    }
}
