#include "render/debug/overlays/RndFramerateOverlay.h"

#include "os/joypads/Keyboard.h"
#include "os/profiling/PerfMgr.h"
#include "os/profiling/PerfTimer.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "utl/text/HmxSnprintf.h"

namespace {

// Counts the overlay in or out of the GPU statistics. Name not in the
// reference map.
void CountGpuStatsUser(bool add) {
    TheRndDevice()->mGpuStats.mEnableCount += add ? 1UL : -1UL;
}

}  // namespace

bool RndFramerateOverlay::gSplitFrameTiming = false;

// Reconstructed from eboot.elf at 0x6E27A0.
RndFramerateOverlay::RndFramerateOverlay()
    : RndOverlayTextBase("framerate", kFlagKeyboard | kFlagHasHelp | kFlagUnknown4),
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
