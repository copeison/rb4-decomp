#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class DataArray;

// A named, nestable timer that the timer displays sort and list. Each
// subclass keeps its own frame history and reports it through the five
// accessors; the base builds the timer's display name and reads its
// "expanded", "isolated" and "budget" settings. The CPU timers (constructor
// 0x24A730, vtable 0x18EF3D8) and RndGpuStatsMgr::Stat derive from it. The
// base vtable is at 0x18EF420.
//
// Name not in the reference map. The map's build has a non-virtual PerfTimer
// in os/PerfTimer.o, whose PerfTimerCfg(DataArray*), GetDisplayName and
// _BuildSortName correspond to LoadConfig, UpdateFullName and GetSortName. In
// this build the code sits among the utl objects, between MsgSink and
// PollMgr.
class PerfTimerBase {
public:
    // What GetSortName prefixes to the name so that the names sort in the
    // display's order. Names not in the reference map.
    enum SortMode {
        kSortName = 0,
        kSortAverageMs = 1,
        kSortWorstMs = 2,
        kSortCount = 3,
        kSortAverageCount = 4,
    };

    explicit PerfTimerBase(Symbol name);  // 0x24B720
    // Slots 0 and 1 at 0x24B1C0 and 0x24B1E0.
    virtual ~PerfTimerBase();

    // Slots 2-6, the timings of the given history frame. Names not in the
    // reference map.
    virtual int _GetCount(unsigned long frame) const = 0;
    virtual float _GetAverageCount(unsigned long frame) const = 0;
    virtual float _GetMs(unsigned long frame) const = 0;
    virtual float _GetAverageMs(unsigned long frame) const = 0;
    virtual float _GetWorstMs(unsigned long frame) const = 0;

    // Rebuilds mFullName from the sort names of the timer and its parents,
    // outermost first. Name not in the reference map.
    void UpdateFullName(int mode);  // 0x24B760
    // The timer's name, prefixed by the value that the mode sorts by. Name
    // not in the reference map.
    const char* GetSortName(int mode) const;  // 0x24B8C0
    // Reads the timer's entry in a timer configuration array. Name not in
    // the reference map.
    void LoadConfig(const DataArray* config);  // 0x24BA10

    // Field names are not in the reference map.
    Symbol mName;
    String mFullName;
    PerfTimerBase* mParent;
    // Set at the end of a frame (0x24A820) when the timer ran under more
    // than one parent; the timer then shows without its parents, after a
    // "~ ", and the timers overlay lists it under "<ambiguous parents>".
    bool mAmbiguousParent;
    bool mHasChildren;
    bool mExpanded;
    // Shows the timer without its parents, after a "!~ ".
    bool mIsolated;
    float mBudget;
    // The budget category, or ~0.
    unsigned int mBudgetCategory;
};

static_assert(offsetof(PerfTimerBase, mName) == 8);
static_assert(offsetof(PerfTimerBase, mFullName) == 16);
static_assert(offsetof(PerfTimerBase, mParent) == 32);
static_assert(offsetof(PerfTimerBase, mAmbiguousParent) == 40);
static_assert(offsetof(PerfTimerBase, mHasChildren) == 41);
static_assert(offsetof(PerfTimerBase, mExpanded) == 42);
static_assert(offsetof(PerfTimerBase, mIsolated) == 43);
static_assert(offsetof(PerfTimerBase, mBudget) == 44);
static_assert(offsetof(PerfTimerBase, mBudgetCategory) == 48);
static_assert(sizeof(PerfTimerBase) == 56);

// A CPU timer (constructor 0x24A730, vtable 0x18EF3D8). The map's
// os/PerfTimer.o has a PerfTimer that keeps the same per-frame history. Its
// accessors (slots 2-6 at 0x24B190, 0x24B1A0, 0x24B1B0, 0x24AD90 and
// 0x24ADA0) read the frames; the start at 0x368B20 and the end of frame at
// 0x24A820 maintain them. The frame in use is the global at 0x19E7E60.
class PerfTimer : public PerfTimerBase {
public:
    // One frame's timing. Name not in the reference map; the field names
    // are not either.
    struct Frame {
        // The time stamp of the outermost start.
        unsigned long mStartCycles;
        // The cycles accumulated this frame; cleared at the end of a frame.
        unsigned long mCycles;
        // How deeply the timer is started; a negative value disables it.
        int mDepth;
        unsigned char mPadding12[4];  // Never read or written.
        float mMs;
        float mWorstMs;
        float mAverageMs;
        // mFrameNumber when mWorstMs was recorded; the worst time resets
        // after a fixed number of frames.
        int mWorstFrame;
        // Counts the frames.
        int mFrameNumber;
        // The starts this frame; copied to mCount at the end of a frame.
        int mPendingCount;
        int mCount;
        float mAverageCount;
        // The timer that was running when this one started, this frame.
        PerfTimerBase* mFrameParent;
        // Whether mFrameParent was set this frame and last frame.
        bool mHasParent;
        bool mHadParent;
        // Whether the timer started under different parents this frame and
        // last frame.
        bool mParentAmbiguous;
        bool mWasParentAmbiguous;
        unsigned char mPadding68[4];  // Never read or written.
    };

    // Reads the timer's settings and its "enabled" flag from the
    // configuration entry when there is one. The map's signature is
    // PerfTimer(PerfTimerCfg const&).
    PerfTimer(Symbol name, const DataArray* config);  // 0x24A730

    int _GetCount(unsigned long frame) const override;            // 0x24B190
    float _GetAverageCount(unsigned long frame) const override;   // 0x24B1A0
    float _GetMs(unsigned long frame) const override;             // 0x24B1B0
    float _GetAverageMs(unsigned long frame) const override;      // 0x24AD90
    float _GetWorstMs(unsigned long frame) const override;        // 0x24ADA0

    // Starts timing the current frame, nesting under the running timer of
    // the thread when there is one. False when the timer is disabled or its
    // parent is collapsed. Inlined into its users; the binary keeps an
    // out-of-line copy among the os/System.o functions. Name not in the
    // reference map.
    bool Start();  // 0x368B20
    // Records `ms` as the current frame's time and updates its worst time
    // and average. Name not in the reference map.
    void UpdateMs(float ms);  // 0x24AA10

    // Closes the current frame: records its time, worst time, averages and
    // count, and settles the timer's parent. `reset` also clears the worst
    // times and averages of both frames. Name not in the reference map.
    void EndFrame(bool reset);  // 0x24A820

    // The frame the timers record into, at 0x19E7E60. The map has
    // PerfTimer::gCurrentFrameIndex.
    static unsigned long gCurrentFrameIndex;
    // The frames after which a worst time expires, 600, at 0x19B03B0.
    static int kWorstResetFrames;

    // Field names are not in the reference map.
    Frame mFrames[2];
    // Cleared by the timer's "enabled" setting; a disabled timer does not
    // start.
    bool mEnabled;
    // The running timers of the thread, which a start pushes the timer
    // onto.
    eastl::vector<PerfTimerBase*>* mRunningTimers;
};

// Replaces `timers` with the non-null timers of `source`, their full names
// rebuilt for the sort mode, in case-insensitive order of the full names.
// The display mode is unused. Name not in the reference map.
void GatherSortedTimers(
    const eastl::vector<PerfTimerBase*>& source,
    eastl::vector<PerfTimerBase*>& timers,
    unsigned int displayMode,
    int sortMode);  // 0x24ADB0

// The worst time in milliseconds below which the timers overlay hides a
// timer that is not isolated, 0.1 by default. The script commands from
// 0x24BB50 toggle the display flags that follow it. Name not in the
// reference map.
extern float gTimerThresholdMs;  // 0x19B03B8

static_assert(sizeof(PerfTimer::Frame) == 72);
static_assert(offsetof(PerfTimer::Frame, mMs) == 24);
static_assert(offsetof(PerfTimer::Frame, mAverageMs) == 32);
static_assert(offsetof(PerfTimer::Frame, mCount) == 48);
static_assert(offsetof(PerfTimer::Frame, mFrameParent) == 56);
static_assert(offsetof(PerfTimer, mFrames) == 56);
static_assert(offsetof(PerfTimer, mEnabled) == 200);
static_assert(offsetof(PerfTimer, mRunningTimers) == 208);
