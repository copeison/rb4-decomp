#pragma once

#include <cstddef>

#include "math/color/Color.h"
#include "math/geometry/Segment.h"
#include "math/vector/Vector2.h"
#include "render/debug/RndOverlay.h"
#include "utl/containers/FixedVector.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

// An overlay that plots series of points against two labelled axes; the
// subclasses supply the axes and the series through slots 8 to 11. Name not
// in the reference map, whose build has no graph overlays. In this build
// its object links between those of the GPU timers and the memory overlay,
// which an object named RndOverlayGraphBase would not; the name is kept
// from the earlier reconstruction. The vtable is at 0x1939778.
class RndOverlayGraphBase : public RndOverlay {
public:
    // Plot settings. Names not in the reference map; the field names are
    // not either.
    struct GraphOptions {
        GraphOptions();  // 0x6E33A0

        float mUnknown0;  // 0.5.
        bool mUnknown4;   // Set.
        long mUnknown8;   // 2.
    };

    // One axis: its label and range. The constructor (0x6E33C0) is inlined
    // into GraphAxes'.
    struct GraphAxis {
        GraphAxis() : mMin(-1.0F), mMax(1.0F), mStep(0.0F), mUnknown20(-1.0F) {}

        Symbol mLabel;
        float mMin;
        float mMax;
        float mStep;
        float mUnknown20;
    };

    struct GraphAxes {
        GraphAxes();  // 0x6E33E0

        GraphAxis mX;
        GraphAxis mY;
        Hmx::Color mColor;  // White.
        float mUnknown64;
        unsigned int mUnknown68;
    };

    // One plotted series.
    struct GraphSeries {
        GraphSeries();  // 0x6E3470

        Symbol mName;
        Hmx::Color mColor;  // White.
        const Vector2* mPoints;
        unsigned long mNumPoints;
    };

    RndOverlayGraphBase(const char* name, unsigned int flags);  // 0x6E32E0
    // Slots 0-1: 0x6E34F0, 0x6E3530.
    ~RndOverlayGraphBase() override;

    // Draws the axes, the grid, the series and their legend through the
    // helpers at 0x6E3880, 0x6E3A70, 0x6E4190 and 0x6E47B0. Not
    // reconstructed.
    int Draw(RndContext& context, int y) override;  // slot 2: 0x6E3580

    // Slots 8 to 11; names not in the reference map.
    // Slot 8 at 0x6E2740, emitted with the framerate graph.
    virtual GraphOptions _GetOptions() {
        return GraphOptions();
    }
    virtual GraphAxes _GetAxes() = 0;                             // slot 9
    virtual unsigned long _GetNumSeries() = 0;                    // slot 10
    virtual GraphSeries _GetSeries(unsigned long index) = 0;  // slot 11

    // Draw's scratch lines; that they are Segment2Ds for DrawLines2D is
    // inferred from their size. Name not in the reference map.
    eastl::vector<Segment2D> mLines;
};

static_assert(sizeof(RndOverlayGraphBase::GraphOptions) == 16);
static_assert(sizeof(RndOverlayGraphBase::GraphAxis) == 24);
static_assert(offsetof(RndOverlayGraphBase::GraphAxes, mColor) == 48);
static_assert(sizeof(RndOverlayGraphBase::GraphAxes) == 72);
static_assert(offsetof(RndOverlayGraphBase::GraphSeries, mPoints) == 24);
static_assert(sizeof(RndOverlayGraphBase::GraphSeries) == 40);
static_assert(offsetof(RndOverlayGraphBase, mLines) == 64);
static_assert(sizeof(RndOverlayGraphBase) == 96);

// The "fps_graph" overlay: the framerate over the last five seconds, kept
// for at most 300 frames. Name not in the reference map. The vtable is at
// 0x19395A0.
class RndFramerateGraphOverlay : public RndOverlayGraphBase {
public:
    RndFramerateGraphOverlay();  // 0x6E22A0
    // Slots 0-1: 0x6E2370, 0x6E23B0.
    ~RndFramerateGraphOverlay() override;

    // Appends the current time and framerate, dropping the oldest samples
    // past 300. Not reconstructed: the time comes from a global timer at
    // 0x19F2520 that has not been identified.
    void _Update() override;                    // slot 5: 0x6E2400
    // Not reconstructed, for the same timer.
    GraphAxes _GetAxes() override;               // slot 9: 0x6E25A0
    unsigned long _GetNumSeries() override;      // slot 10: 0x6E26B0
    GraphSeries _GetSeries(unsigned long index) override;  // slot 11: 0x6E26C0

    // Time in seconds and framerate. Name not in the reference map.
    eastl::vector<Vector2> mSamples;
};

static_assert(offsetof(RndFramerateGraphOverlay, mSamples) == 96);
static_assert(sizeof(RndFramerateGraphOverlay) == 128);

// A graph of the timers of one timers overlay against a budget line. Name
// not in the reference map. The vtable is at 0x1939930.
class RndTimerGraphOverlay : public RndOverlayGraphBase {
public:
    // A series color. Name not in the reference map.
    struct PaletteEntry {
        Hmx::Color mColor;
        void* mUnknown16;
    };

    // Looks up the overlay named `timersName`. A positive budget draws a
    // budget line and scales the graph to twice the budget.
    RndTimerGraphOverlay(
        const char* name,
        unsigned int flags,
        const char* timersName,
        float budget);  // 0x6E6280
    // Slots 0-1: 0x6E6880, 0x6E6930.
    ~RndTimerGraphOverlay() override;

    // Not reconstructed.
    void _Update() override;                     // slot 5: 0x6E6950
    GraphOptions _GetOptions() override;         // slot 8: 0x6E72F0
    // Not reconstructed: the time comes from the unidentified timer at
    // 0x19F2520.
    GraphAxes _GetAxes() override;               // slot 9: 0x6E7340
    // Not reconstructed.
    unsigned long _GetNumSeries() override;      // slot 10: 0x6E7470
    GraphSeries _GetSeries(unsigned long index) override;  // slot 11: 0x6E74C0

    // Field names are not in the reference map.
    RndOverlay* mTimersOverlay;
    FixedVector<Vector2, 2> mBudgetLine;
    float mTimeWindow;  // Seconds shown, 5.
    float mMaxMs;       // The top of the graph.
    eastl::vector<void*> mUnknown152;
    eastl::vector<PaletteEntry> mPalette;
    eastl::vector<void*> mUnknown216;
    eastl::vector<void*> mUnknown248;
};

static_assert(sizeof(RndTimerGraphOverlay::PaletteEntry) == 24);
static_assert(offsetof(RndTimerGraphOverlay, mTimersOverlay) == 96);
static_assert(offsetof(RndTimerGraphOverlay, mBudgetLine) == 104);
static_assert(offsetof(RndTimerGraphOverlay, mTimeWindow) == 144);
static_assert(offsetof(RndTimerGraphOverlay, mMaxMs) == 148);
static_assert(offsetof(RndTimerGraphOverlay, mUnknown152) == 152);
static_assert(offsetof(RndTimerGraphOverlay, mPalette) == 184);
static_assert(offsetof(RndTimerGraphOverlay, mUnknown216) == 216);
static_assert(offsetof(RndTimerGraphOverlay, mUnknown248) == 248);
static_assert(sizeof(RndTimerGraphOverlay) == 280);

// The "cpu_timer_graph" overlay, against the "cpu" timer's budget. Name not
// in the reference map. The vtable is at 0x19394A0.
class RndCpuTimerGraphOverlay : public RndTimerGraphOverlay {
public:
    RndCpuTimerGraphOverlay();  // 0x6E1EC0
    // Slots 0-1: 0x6E1F40, 0x6E1F50.
    ~RndCpuTimerGraphOverlay() override;
};

static_assert(sizeof(RndCpuTimerGraphOverlay) == 280);

// The "gpu_timer_graph" overlay, against the "GPU Total" GPU statistic's
// budget. Name not in the reference map. The vtable is at 0x1939678.
class RndGpuTimerGraphOverlay : public RndTimerGraphOverlay {
public:
    RndGpuTimerGraphOverlay();  // 0x6E2E60
    // Slots 0-1: 0x6E2EF0, 0x6E2F00.
    ~RndGpuTimerGraphOverlay() override;
};

static_assert(sizeof(RndGpuTimerGraphOverlay) == 280);
