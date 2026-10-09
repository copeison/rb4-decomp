#include "render/debug/overlays/RndOverlayGraphBase.h"

#include "render/system/RndDevice.h"

// Reconstructed from eboot.elf at 0x6E2E60.
RndGpuTimerGraphOverlay::RndGpuTimerGraphOverlay()
    : RndTimerGraphOverlay(
          "gpu_timer_graph",
          0,
          "gpu_timers",
          TheRndDevice()->mGpuStats.GetBudget(Symbol("GPU Total"))) {}

// Reconstructed from eboot.elf at 0x6E2EF0 (deleting variant at 0x6E2F00).
RndGpuTimerGraphOverlay::~RndGpuTimerGraphOverlay() {}
