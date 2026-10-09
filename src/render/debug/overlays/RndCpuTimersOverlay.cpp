#include "render/debug/overlays/RndTimersOverlay.h"

#include "os/profiling/PerfMgr.h"
#include "os/profiling/PerfTimer.h"

// Reconstructed from eboot.elf at 0x6E1F90.
RndCpuTimersOverlay::RndCpuTimersOverlay() : RndTimersOverlay("cpu_timers", 0) {}

// Reconstructed from eboot.elf at 0x6E1FC0 (deleting variant at 0x6E1FD0).
RndCpuTimersOverlay::~RndCpuTimersOverlay() {}

// Reconstructed from eboot.elf at 0x6E1FF0.
void RndCpuTimersOverlay::PrintHelp(TextStream& stream) {
    stream << "Displays CPU timers\n";
    RndTimersOverlay::PrintHelp(stream);
    stream << "Related commands:\n"
              "  expand_all_timers:       expands all timers\n"
              "  collapse_all_timers:     collapses all timers\n"
              "  toggle_entity_timers:    toggles per-entity timers on/off (on by default)\n"
              "  toggle_component_timers: toggles per-component timers on/off (off by default)\n";
}

// Reconstructed from eboot.elf at 0x6E2030. Every thread with a timer
// table.
void RndCpuTimersOverlay::_GatherThreads(eastl::vector<ScePthread>& threads) {
    threads.reserve(thePerfMgr.mThreadTimers.size());
    for (PerfTimerMgr::ThreadTimers* entry : thePerfMgr.mThreadTimers) {
        threads.push_back(entry->mThread);
    }
}

// Reconstructed from eboot.elf at 0x6E21E0.
void RndCpuTimersOverlay::_GatherThreadTimers(
    ScePthread thread,
    eastl::vector<PerfTimerBase*>& timers,
    unsigned int displayMode,
    int sortMode) {
    for (PerfTimerMgr::ThreadTimers* entry : thePerfMgr.mThreadTimers) {
        if (entry->mThread == thread) {
            GatherSortedTimers(*entry->mTimers, timers, displayMode, sortMode);
        }
    }
}
