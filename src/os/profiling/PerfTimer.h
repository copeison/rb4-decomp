#pragma once

#include <cstddef>

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
    // Shows the timer without its parents, after a "~ ".
    bool mUnknown40;
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
static_assert(offsetof(PerfTimerBase, mUnknown40) == 40);
static_assert(offsetof(PerfTimerBase, mHasChildren) == 41);
static_assert(offsetof(PerfTimerBase, mExpanded) == 42);
static_assert(offsetof(PerfTimerBase, mIsolated) == 43);
static_assert(offsetof(PerfTimerBase, mBudget) == 44);
static_assert(offsetof(PerfTimerBase, mBudgetCategory) == 48);
static_assert(sizeof(PerfTimerBase) == 56);
