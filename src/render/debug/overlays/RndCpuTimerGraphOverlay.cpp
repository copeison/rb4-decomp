#include "render/debug/overlays/RndOverlayGraphBase.h"

#include "os/profiling/PerfMgr.h"
#include "os/profiling/PerfTimer.h"

// Reconstructed from eboot.elf at 0x6E1EC0.
RndCpuTimerGraphOverlay::RndCpuTimerGraphOverlay()
    : RndTimerGraphOverlay(
          "cpu_timer_graph",
          0,
          "cpu_timers",
          thePerfMgr.GetTimer(Symbol("cpu"))->mBudget) {}

// Reconstructed from eboot.elf at 0x6E1F40 (deleting variant at 0x6E1F50).
RndCpuTimerGraphOverlay::~RndCpuTimerGraphOverlay() {}
