#include "render/debug/overlays/RndOverlayGraphBase.h"

// Reconstructed from eboot.elf at 0x6E33A0.
RndOverlayGraphBase::GraphOptions::GraphOptions()
    : mUnknown0(0.5F), mUnknown4(true), mUnknown8(2) {}

// Reconstructed from eboot.elf at 0x6E33E0.
RndOverlayGraphBase::GraphAxes::GraphAxes()
    : mColor(Hmx::Color::GetWhite()), mUnknown64(0.0F), mUnknown68(0) {}

// Reconstructed from eboot.elf at 0x6E3470.
RndOverlayGraphBase::GraphSeries::GraphSeries()
    : mColor(Hmx::Color::GetWhite()), mPoints(nullptr), mNumPoints(0) {}

// Reconstructed from eboot.elf at 0x6E32E0. Reserves room for 200 lines.
RndOverlayGraphBase::RndOverlayGraphBase(const char* name, unsigned int flags)
    : RndOverlay(name, flags) {
    mLines.reserve(200);
}

// Reconstructed from eboot.elf at 0x6E34F0 (deleting variant at 0x6E3530).
RndOverlayGraphBase::~RndOverlayGraphBase() {}
