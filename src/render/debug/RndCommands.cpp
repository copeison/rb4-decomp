#include "render/debug/RndCommands.h"

#include <cstdint>
#include <cstring>

#include "os/debug/Debug.h"
#include "os/platform/PlatformMgr.h"
#include "render/debug/RndBufferInspection.h"
#include "render/debug/RndOverlay.h"
#include "render/debug/RndOverlayMgr.h"
#include "render/debug/screenshot_capture.h"
#include "render/shaders/RndShaderEnums.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndWindow.h"
#include "utl/data/DataArray.h"
#include "utl/text/Str.h"

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

// Reconstructed from eboot.elf at 0x6BA3D0. The toggle goes through
// SetShowing; RndOverlay::ToggleShowing is not called.
DataNode RndCommands::_OnToggleOverlay(DataArray* args) {
    if (args->Size() >= 2) {
        if (auto* overlay = RndOverlayMgr::TryGetOverlay(args->Sym(1))) {
            overlay->SetShowing(!overlay->IsShowing());
        }
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BA430.
DataNode RndCommands::_OnPrintOverlayHelp(DataArray* args) {
    static Symbol sAll;
    if (sAll == Symbol()) {
        sAll = Symbol("all");
    }

    Symbol name = sAll;
    if (args->Size() >= 2) {
        name = args->Sym(1);
    }
    if (name == sAll) {
        const auto end = RndOverlayMgr::End();
        for (auto it = RndOverlayMgr::Begin(); it != end; ++it) {
            if ((it->GetFlags() & RndOverlay::kFlagHasHelp) != 0) {
                it->PrintHelp(TheDebug);
            }
        }
    } else {
        auto* overlay = RndOverlayMgr::TryGetOverlay(args->Sym(1));
        if (overlay != nullptr &&
            (overlay->GetFlags() & RndOverlay::kFlagHasHelp) != 0) {
            overlay->PrintHelp(TheDebug);
        }
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BA730. The level names, and the list
// of valid names for an unknown one, are formatted for a message that is
// compiled out of this build.
DataNode RndCommands::_OnSetQualityLevel(DataArray* args) {
    auto& settings = render_settings();
    RndQualityLevel level;
    if (args->Size() <= 1) {
        level = settings.mQualityLevel;
    } else {
        level = RndQualityLevelFromName(args->Str(1));
        if (level == RndQualityLevel::kInvalid) {
            StackString<256> names;
            names << RndQualityLevelName(RndQualityLevel::kLow);
            names << ", ";
            names << RndQualityLevelName(RndQualityLevel::kMedium);
            names << ", ";
            names << RndQualityLevelName(RndQualityLevel::kHigh);
            return DataNode(0);
        }
        settings.mQualityLevel = level;
    }
    static_cast<void>(RndQualityLevelName(level));
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BA880.
DataNode RndCommands::_OnToggleVSync(DataArray*) {
    toggle(render_settings().mVSyncEnabled);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BA8B0.
DataNode RndCommands::_OnToggleSceneMask(DataArray*) {
    toggle(render_settings().mSceneMaskEnabled);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BA8E0.
DataNode RndCommands::_OnToggleShadows(DataArray*) {
    toggle(render_settings().mShadowsEnabled);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BA910.
DataNode RndCommands::_OnTogglePostProc(DataArray*) {
    toggle(render_settings().mPostProcEnabled);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BA940.
DataNode RndCommands::_OnToggleToneMapping(DataArray*) {
    toggle(render_settings().mTonemappingEnabled);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BA970.
DataNode RndCommands::_OnToggleVScat(DataArray*) {
    toggle(render_settings().mVolumetricScatteringEnabled);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BA590.
DataNode RndCommands::_OnReloadShaders(DataArray*) {
    render_system().mShaderMgr.ReloadAll();
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BAA40.
DataNode RndCommands::_OnToggleMultithreadedRendering(DataArray*) {
    toggle(render_settings().mMultithreadedRenderingEnabled);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BAA70.
DataNode RndCommands::_OnToggleAsyncCompute(DataArray*) {
    toggle(render_settings().mAsyncComputeEnabled);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BAAA0.
DataNode RndCommands::_OnToggleAsyncCopy(DataArray*) {
    toggle(render_settings().mAsyncCopyEnabled);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BAAD0.
DataNode RndCommands::_OnToggleTiledLightInterpolation(DataArray*) {
    toggle(render_settings().mTiledLightInterpolationEnabled);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BAB00.
DataNode RndCommands::_OnTogglePartialFramerate(DataArray*) {
    auto& settings = render_settings();
    settings.mPartialFramerateEnabled =
        settings.mMaxPartialFramerateScenes != 0 &&
        !settings.mPartialFramerateEnabled;
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BAB40.
DataNode RndCommands::_OnToggleStereoOptimizations(DataArray*) {
    toggle(render_settings().mStereoOptimizationsEnabled);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BAB70.
DataNode RndCommands::_OnToggle64BitLightAccum(DataArray*) {
    toggle(render_settings().mUse64BitLightAccum);
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BABA0.
DataNode RndCommands::_OnToggleHdr(DataArray*) {
    auto& mode = render_system().mHdrOutputMode;
    mode = mode == 1 ? 0 : 1;
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BABD0.
DataNode RndCommands::_OnTakeScreenshot(DataArray*) {
    screenshot_request();
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BAC00.
DataNode RndCommands::_OnCycleScreenshotResolution(DataArray*) {
    auto& resolution = render_settings().mScreenshotResolution;
    const auto next =
        (static_cast<std::uint32_t>(resolution) + 1) % 6;
    resolution = static_cast<ScreenshotResolution>(next);
    static_cast<void>(screenshot_resolution_name(resolution));
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BA5D0.
DataNode RndCommands::_OnSetResolution(DataArray* args) {
    auto& device = render_system();
    auto& settings = *device.mSettings;
    Vector2i resolution{0, 0};
    if (args->Size() >= 3) {
        resolution.x = args->Node(1).Int(args);
        resolution.y = args->Node(2).Int(args);
    } else if (args->Size() == 2) {
        if (args->Node(1).Type() != kDataInt) {
            if (!ParseResolution(args->Node(1).Str(args), resolution)) {
                return DataNode(0);
            }
        } else {
            resolution.y = args->Node(1).Int(args);
            resolution.x = 16 * resolution.y / 9;
        }
    } else {
        settings.mResolutionOverridden = false;
        return DataNode(0);
    }

    for (const auto& supported : device.mCapabilities[kPlatformPS4].mResolutions) {
        if (supported.x == resolution.x && supported.y == resolution.y) {
            settings.mOutputResolution = resolution;
            settings.mResolutionOverridden = true;
            break;
        }
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BA9A0.
DataNode RndCommands::_OnSetDrawnSceneRange(DataArray* args) {
    auto& settings = render_settings();
    settings.mFirstDrawnScene = 0;
    settings.mLastDrawnScene = -1;
    if (args->Size() >= 2) {
        settings.mFirstDrawnScene = args->Node(1).Int(args);
        if (args->Size() >= 3) {
            settings.mLastDrawnScene = args->Node(2).Int(args);
        }
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BAC70.
DataNode RndCommands::_OnSetShadingMode(DataArray* args) {
    if (auto* window = render_system().mMainWindow) {
        _SetShadingMode(*window, args);
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BACB0. The listed names are not
// printed in this build.
bool RndCommands::_SetShadingMode(RndWindow& window, DataArray* args) {
    unsigned int mode = 0;
    if (args->Size() == 2) {
        const DataNode node = args->Evaluate(1);
        const char* name = nullptr;
        if (node.Type() == kDataString) {
            name = reinterpret_cast<const char*>(node.mValue.array->mNodes);
        } else if (node.Type() == kDataSymbol) {
            name = node.mValue.symbol;
        } else {
            mode = node.Type() == kDataInt ? static_cast<unsigned int>(node.mValue.integer) : 0;
        }
        if (name != nullptr) {
            if (strcasecmp(name, "help") == 0) {
                for (unsigned int index = 0; index < kNumUserShadingModes; ++index) {
                    static_cast<void>(ToString(static_cast<RndUserShadingMode>(index)));
                }
                return true;
            }
            mode = UserShadingModeFromString(name);
            if (mode == static_cast<unsigned int>(-1)) {
                return false;
            }
        }
    }
    static_cast<void>(ToString(static_cast<RndUserShadingMode>(mode)));
    window.SetShadingMode(mode);
    return true;
}

// Reconstructed from eboot.elf at 0x6BAF40.
DataNode RndCommands::_OnSetBufferInspectionMode(DataArray* args) {
    if (auto* window = render_system().mMainWindow) {
        _SetBufferInspectionMode(*window, args);
    }
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0x6BAF80.
bool RndCommands::_SetBufferInspectionMode(RndWindow& window, DataArray* args) {
    unsigned int mode = 0;
    if (args->Size() == 2) {
        const DataNode node = args->Evaluate(1);
        const char* name = nullptr;
        if (node.Type() == kDataString) {
            name = reinterpret_cast<const char*>(node.mValue.array->mNodes);
        } else if (node.Type() == kDataSymbol) {
            name = node.mValue.symbol;
        } else {
            mode = node.Type() == kDataInt ? static_cast<unsigned int>(node.mValue.integer) : 0;
        }
        if (name != nullptr) {
            if (strcasecmp(name, "help") == 0) {
                for (unsigned int index = 0; index < RndBufferInspection::kNumModes; ++index) {
                    static_cast<void>(
                        RndBufferInspection::ToString(static_cast<RndBufferInspectionMode>(index)));
                }
                return true;
            }
            mode = RndBufferInspection::FromString(name);
            if (mode == static_cast<unsigned int>(-1)) {
                return false;
            }
        }
    }
    static_cast<void>(RndBufferInspection::ToString(static_cast<RndBufferInspectionMode>(mode)));
    window.SetBufferInspectionMode(mode);
    return true;
}

// Reconstructed from eboot.elf at 0x6BB0E0.
void RndCommands::Init() {
    DataRegisterFunc(Symbol("toggle_overlay"), _OnToggleOverlay);
    DataRegisterFunc(Symbol("overlay_help"), _OnPrintOverlayHelp);
    DataRegisterFunc(Symbol("reload_shaders"), _OnReloadShaders);
    DataRegisterFunc(Symbol("set_resolution"), _OnSetResolution);
    DataRegisterFunc(Symbol("set_quality_level"), _OnSetQualityLevel);
    DataRegisterFunc(Symbol("toggle_vsync"), _OnToggleVSync);
    DataRegisterFunc(Symbol("toggle_scene_mask"), _OnToggleSceneMask);
    DataRegisterFunc(Symbol("toggle_shadows"), _OnToggleShadows);
    DataRegisterFunc(Symbol("toggle_postproc"), _OnTogglePostProc);
    DataRegisterFunc(Symbol("toggle_tonemapping"), _OnToggleToneMapping);
    DataRegisterFunc(Symbol("toggle_vscat"), _OnToggleVScat);
    DataRegisterFunc(Symbol("set_drawn_scene_range"), _OnSetDrawnSceneRange);
    DataRegisterFunc(Symbol("toggle_multithreaded_rendering"), _OnToggleMultithreadedRendering);
    DataRegisterFunc(Symbol("toggle_async_compute"), _OnToggleAsyncCompute);
    DataRegisterFunc(Symbol("toggle_async_copy"), _OnToggleAsyncCopy);
    DataRegisterFunc(Symbol("toggle_tiled_light_interpolation"), _OnToggleTiledLightInterpolation);
    DataRegisterFunc(Symbol("toggle_partial_framerate"), _OnTogglePartialFramerate);
    DataRegisterFunc(Symbol("toggle_stereo_optimizations"), _OnToggleStereoOptimizations);
    DataRegisterFunc(Symbol("toggle_64_bit_light_accum"), _OnToggle64BitLightAccum);
    DataRegisterFunc(Symbol("toggle_hdr"), _OnToggleHdr);
    DataRegisterFunc(Symbol("take_screenshot"), _OnTakeScreenshot);
    DataRegisterFunc(Symbol("cycle_screenshot_resolution"), _OnCycleScreenshotResolution);
    DataRegisterFunc(Symbol("set_shading_mode"), _OnSetShadingMode);
    DataRegisterFunc(Symbol("set_buffer_inspection_mode"), _OnSetBufferInspectionMode);
}
