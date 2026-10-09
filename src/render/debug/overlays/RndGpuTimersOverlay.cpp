#include "render/debug/overlays/RndTimersOverlay.h"

#include "render/system/RndDevice.h"

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
bool RndGpuTimersOverlay::_Unknown11() {
    return false;
}
