#include "render/debug/overlays/RndOverlayGraphBase.h"

#include "render/system/RndDevice.h"
#include "utl/time/TimeMgr.h"

namespace {

// The samples kept. Name not in the reference map.
constexpr unsigned long kMaxSamples = 300;

// The current real time in seconds. Inlined. Name not in the reference
// map.
float RealTimeSeconds() {
    return TheTimeMgr->mRealTime.SplitMs() * 0.001F;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6E22A0. Reserves room for 300 samples.
RndFramerateGraphOverlay::RndFramerateGraphOverlay() : RndOverlayGraphBase("fps_graph", 0) {
    mSamples.reserve(kMaxSamples);
}

// Reconstructed from eboot.elf at 0x6E2370 (deleting variant at 0x6E23B0).
RndFramerateGraphOverlay::~RndFramerateGraphOverlay() {}

// Reconstructed from eboot.elf at 0x6E2400.
void RndFramerateGraphOverlay::_Update() {
    const float now = RealTimeSeconds();
    const Vector2 sample = {now, TheRndDevice()->mFrameRate};
    mSamples.push_back(sample);
    while (mSamples.size() >= kMaxSamples) {
        mSamples.erase(mSamples.begin());
    }
}

// Reconstructed from eboot.elf at 0x6E25A0. The last five seconds, against
// up to 90 frames per second.
RndOverlayGraphBase::GraphAxes RndFramerateGraphOverlay::_GetAxes() {
    GraphAxes axes;
    const float now = RealTimeSeconds();
    axes.mX.mLabel = Symbol("Time (sec)");
    axes.mX.mMax = now;
    axes.mX.mMin = now + -5.0F;
    axes.mX.mStep = 1.0F;
    axes.mY.mLabel = Symbol("FPS");
    axes.mY.mMin = 0.0F;
    axes.mY.mMax = 90.0F;
    axes.mY.mStep = 10.0F;
    axes.mY.mLabelSide = 1.0F;
    axes.mOriginX = now;
    axes.mOriginY = 0.0F;
    return axes;
}

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
