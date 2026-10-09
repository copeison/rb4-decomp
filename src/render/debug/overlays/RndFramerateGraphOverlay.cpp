#include "render/debug/overlays/RndOverlayGraphBase.h"

// Reconstructed from eboot.elf at 0x6E22A0. Reserves room for 300 samples.
RndFramerateGraphOverlay::RndFramerateGraphOverlay() : RndOverlayGraphBase("fps_graph", 0) {
    mSamples.reserve(300);
}

// Reconstructed from eboot.elf at 0x6E2370 (deleting variant at 0x6E23B0).
RndFramerateGraphOverlay::~RndFramerateGraphOverlay() {}

// Reconstructed from eboot.elf at 0x6E26B0.
unsigned long RndFramerateGraphOverlay::_GetNumSeries() {
    return 1;
}

// Reconstructed from eboot.elf at 0x6E26C0.
RndOverlayGraphBase::GraphSeries RndFramerateGraphOverlay::_GetSeries(unsigned long index) {
    static_cast<void>(index);
    GraphSeries series;
    series.mName = Symbol("fps");
    series.mPoints = mSamples.begin();
    series.mNumPoints = mSamples.size();
    return series;
}
