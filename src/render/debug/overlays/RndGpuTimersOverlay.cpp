#include "render/debug/overlays/RndTimersOverlay.h"

#include "render/system/RndDevice.h"
#include "utl/threading/Thread.h"

// Reconstructed from eboot.elf at 0x6E2F40.
RndGpuTimersOverlay::RndGpuTimersOverlay() : RndTimersOverlay("gpu_timers", 0) {}

// Reconstructed from eboot.elf at 0x6E2F70 (deleting variant at 0x6E2F80).
RndGpuTimersOverlay::~RndGpuTimersOverlay() {}

// Reconstructed from eboot.elf at 0x6E2FC0.
void RndGpuTimersOverlay::PrintHelp(TextStream& stream) {
    stream << "Displays GPU timers\n";
    RndTimersOverlay::PrintHelp(stream);
}

// Reconstructed from eboot.elf at 0x6E2FA0. SetShowing has already updated
// mShowing.
void RndGpuTimersOverlay::_HandleShowingChanged(bool showing) {
    static_cast<void>(showing);
    TheRndDevice()->mGpuStats.mEnableCount += mShowing ? 1UL : -1UL;
}

// Reconstructed from eboot.elf at 0x6E2FF0.
bool RndGpuTimersOverlay::_ShowsThreads() {
    return false;
}

// Reconstructed from eboot.elf at 0x6E3000. The statistics are listed under
// the main thread.
void RndGpuTimersOverlay::_GatherThreads(eastl::vector<ScePthread>& threads) {
    threads.push_back(Thread::s_MainThreadID);
}

// Reconstructed from eboot.elf at 0x6E30C0.
void RndGpuTimersOverlay::_GatherThreadTimers(
    ScePthread thread,
    eastl::vector<PerfTimerBase*>& timers,
    unsigned int displayMode,
    int sortMode) {
    static_cast<void>(thread);
    TheRndDevice()->mGpuStats.GatherTimers(displayMode, sortMode, timers);
}

// Reconstructed from eboot.elf at 0x6E30E0.
void RndGpuTimersOverlay::_PrintExtraHeaders(TextStream& stream) {
    TimerItemView::_PrintHeader(this, stream, "num_verts", 12, true);
    TimerItemView::_PrintHeader(this, stream, "num_prims", 12, true);
    TimerItemView::_PrintHeader(this, stream, "vs_invocs", 14, true);
    TimerItemView::_PrintHeader(this, stream, "ps_invocs", 14, true);
    TimerItemView::_PrintHeader(this, stream, "cs_invocs", 14, true);
}

// Reconstructed from eboot.elf at 0x6E3180. Each column lists the frames
// in turn; the fourth counter is not shown.
void RndGpuTimersOverlay::_PrintExtraStats(
    TextStream& stream,
    const PerfTimerBase& timer,
    unsigned long numFrames) {
    const auto& stat = static_cast<const RndGpuStatsMgr::Stat&>(timer);
    for (unsigned long i = 0; i < numFrames; ++i) {
        TimerItemView::_PrintStat(
            this, stream, static_cast<int>(stat.mFrames[i].mCounters[0]), 12);
    }
    for (unsigned long i = 0; i < numFrames; ++i) {
        TimerItemView::_PrintStat(
            this, stream, static_cast<int>(stat.mFrames[i].mCounters[1]), 12);
    }
    for (unsigned long i = 0; i < numFrames; ++i) {
        TimerItemView::_PrintStat(
            this, stream, static_cast<int>(stat.mFrames[i].mCounters[2]), 14);
    }
    for (unsigned long i = 0; i < numFrames; ++i) {
        TimerItemView::_PrintStat(
            this, stream, static_cast<int>(stat.mFrames[i].mCounters[4]), 14);
    }
    for (unsigned long i = 0; i < numFrames; ++i) {
        TimerItemView::_PrintStat(
            this, stream, static_cast<int>(stat.mFrames[i].mCounters[5]), 14);
    }
}
