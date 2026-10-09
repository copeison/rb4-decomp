#include "render/debug/overlays/RndMemOverlay.h"

// Reconstructed from eboot.elf at 0x6E54A0.
RndMemOverlay::RndMemOverlay() : RndOverlayTextBase("mem", kFlagStripedLines) {
    mDisplayName = "memory";
}

// Reconstructed from eboot.elf at 0x6E5520 (deleting variant at 0x6E5530).
RndMemOverlay::~RndMemOverlay() {}
