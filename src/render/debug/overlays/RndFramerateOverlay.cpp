#include "render/debug/overlays/RndFramerateOverlay.h"

#include "entity/props/PropMetadata.h"
#include "entity/props/PropRegistry.h"
#include "os/joypads/Keyboard.h"
#include "os/profiling/PerfMgr.h"
#include "os/profiling/PerfTimer.h"
#include "render/debug/RndOverlayMgr.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "utl/text/HmxSnprintf.h"

namespace {

// Counts the overlay in or out of the GPU statistics. Name not in the
// reference map.
void CountGpuStatsUser(bool add) {
    TheRndDevice()->mGpuStats.mEnableCount += add ? 1UL : -1UL;
}

// The overlay, looked up by name. Name not in the reference map.
RndFramerateOverlay* FramerateOverlay() {
    return static_cast<RndFramerateOverlay*>(RndOverlayMgr::GetOverlay(Symbol("framerate")));
}

// The accessors of the overlay's options. Names not in the reference map.

// Reconstructed from eboot.elf at 0x6E2CC0.
bool GetShowCpuAverage(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return FramerateOverlay()->mShowCpuAverage;
}

// Reconstructed from eboot.elf at 0x6E2D10.
void SetShowCpuAverage(const PropAccessorArgs& args) {
    FramerateOverlay()->mShowCpuAverage = *static_cast<const bool*>(args.mValue);
}

// Reconstructed from eboot.elf at 0x6E2D70.
bool GetShowGpuAverage(const PropAccessorArgs& args) {
    static_cast<void>(args);
    return FramerateOverlay()->mShowGpuAverage;
}

// Reconstructed from eboot.elf at 0x6E2DC0. Toggles as the 'G' key does.
void SetShowGpuAverage(const PropAccessorArgs& args) {
    RndFramerateOverlay* overlay = FramerateOverlay();
    if (overlay->mShowGpuAverage != *static_cast<const bool*>(args.mValue)) {
        overlay->mShowGpuAverage = !overlay->mShowGpuAverage;
        if (overlay->mShowing) {
            CountGpuStatsUser(overlay->mShowGpuAverage);
        }
    }
}

}  // namespace

bool RndFramerateOverlay::gSplitFrameTiming = false;

// Reconstructed from eboot.elf at 0x6E27A0.
RndFramerateOverlay::RndFramerateOverlay()
    : RndOverlayTextBase("framerate", kFlagKeyboard | kFlagHasHelp | kFlagHasOptions),
      mShowCpuAverage(false),
      mShowGpuAverage(false),
      mCpuTimer(nullptr) {
    mCpuTimer = thePerfMgr.GetTimer(Symbol("cpu"));
}

// Reconstructed from eboot.elf at 0x6E2830 (deleting variant at 0x6E2840).
RndFramerateOverlay::~RndFramerateOverlay() {}

// Reconstructed from eboot.elf at 0x6E2860.
bool RndFramerateOverlay::HandleKeyboardMsg(const KeyboardKeyMsg& msg) {
    switch (msg.GetKey()) {
    case 'C':
    case 'c':
        mShowCpuAverage = !mShowCpuAverage;
        return true;
    case 'G':
    case 'g':
        mShowGpuAverage = !mShowGpuAverage;
        if (mShowing) {
            CountGpuStatsUser(mShowGpuAverage);
        }
        return true;
    default:
        return false;
    }
}

// Reconstructed from eboot.elf at 0x6E2940.
void RndFramerateOverlay::PrintHelp(TextStream& stream) {
    stream << "Displays the current framerate.\n"
              "Keyboard Controls:\n"
              "  C: toggle average CPU time\n"
              "  G: toggle average GPU time (which adds some overhead, be careful!)\n";
}

// Reconstructed from eboot.elf at 0x6E2910. SetShowing has already updated
// mShowing.
void RndFramerateOverlay::_HandleShowingChanged(bool showing) {
    static_cast<void>(showing);
    if (mShowGpuAverage) {
        CountGpuStatsUser(mShowing);
    }
}

// Reconstructed from eboot.elf at 0x6E2960. Both options are bools kept
// by the overlay and reached through accessors. Each registration builds
// a "prop" symbol that it does not use.
void RndFramerateOverlay::_RegisterOptions(PropRegistry& registry) {
    bool type = false;
    BoolMetadata& cpu = TypeSpecificMetadata(
        registry.RegisterProp("show_cpu_average", -1, kPropertyBool, 0), type);
    static_cast<void>(Symbol("prop"));
    cpu.mGetter = GetShowCpuAverage;
    cpu.mSetter = SetShowCpuAverage;

    bool gpuType = false;
    BoolMetadata& gpu = TypeSpecificMetadata(
        registry.RegisterProp("show_gpu_average", -1, kPropertyBool, 0), gpuType);
    static_cast<void>(Symbol("prop"));
    gpu.mGetter = GetShowGpuAverage;
    gpu.mSetter = SetShowGpuAverage;
}

// Reconstructed from eboot.elf at 0x6E2A60.
void RndFramerateOverlay::_Print(TextStream& stream) {
    char buffer[16];
    const RndConfig* settings = TheRndDevice()->mSettings;
    HmxSnprintf(
        buffer,
        sizeof(buffer),
        "%dx%d %2.1f",
        settings->mOutputResolution.x,
        settings->mOutputResolution.y,
        static_cast<double>(TheRndDevice()->mSmoothedFrameRate));
    stream << buffer << " fps   ";

    const unsigned long frames = static_cast<unsigned long>(gSplitFrameTiming) + 1;
    if (mShowCpuAverage) {
        stream << " (cpu avg ";
        for (unsigned long i = 0; i < frames; ++i) {
            if (i != 0) {
                stream << "/";
            }
            HmxSnprintf(
                buffer,
                sizeof(buffer),
                "%2.1f",
                static_cast<double>(mCpuTimer->mFrames[i].mAverageMs));
            stream << buffer;
        }
        stream << "ms)";
    }
    if (mShowGpuAverage) {
        static Symbol sGpuTotalName;
        if (sGpuTotalName == Symbol()) {
            sGpuTotalName = Symbol("GPU Total");
        }
        stream << " (gpu avg ";
        for (unsigned long i = 0; i < frames; ++i) {
            if (i != 0) {
                stream << "/";
            }
            HmxSnprintf(
                buffer,
                sizeof(buffer),
                "%2.1f",
                static_cast<double>(
                    TheRndDevice()->mGpuStats.GetAverageMs(sGpuTotalName, i)));
            stream << buffer;
        }
        stream << "ms)";
    }
}

// Reconstructed from eboot.elf at 0x6E2CA0: a translucent green.
Hmx::Color RndFramerateOverlay::_GetBackgroundColor() const {
    return Hmx::Color(0.0F, 0.3F, 0.0F, 0.4F);
}
