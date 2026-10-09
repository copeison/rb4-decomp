#pragma once

#include <_pthread.h>
#include <cstddef>

#include "os/threading/CritSec.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class DataArray;
class PerfTimer;
class PerfTimerBase;

// The registry of CPU timers (os/PerfMgr.o). Only the members the debug
// overlays use are declared.
class PerfTimerMgr {
public:
    // The timers of one thread. Name not in the reference map, whose build
    // keeps an eastl::pair<pthread*, PerfTimer**> per thread; the field
    // names are not in it either.
    struct ThreadTimers {
        ScePthread mThread;
        eastl::vector<PerfTimerBase*>* mTimers;  // Null entries are unused.
    };

    // Guard the thread tables while the timer overlays read them.
    void Lock();    // 0x249980
    void Unlock();  // 0x2499A0

    // The timer with the name, registering the name when it is new. Null
    // when the manager is disabled. Calls GetTimerIndex(Symbol) (0x2499D0)
    // and GetTimer(unsigned long) (0x249B70).
    PerfTimer* GetTimer(Symbol name);  // 0x2499B0

    // Field names are not in the reference map; the static constructor at
    // 0x24A630 and PerfTimerMgr::Init(DataArray*) (0x249650) give them.
    // Set by Init; GetTimer(unsigned long) returns null while it is clear.
    bool mEnabled;
    // Guards the tables; Lock and Unlock enter and leave it.
    CritSec mCritSec;
    // The timer configuration Init keeps a reference to; _ThreadInit
    // (0x24A0B0) registers a timer for each of its entries.
    DataArray* mConfig;
    // The registered timer names; GetTimerIndex(Symbol) returns the index.
    eastl::vector<Symbol> mTimerNames;
    eastl::vector<ThreadTimers*> mThreadTimers;
};

static_assert(offsetof(PerfTimerMgr, mCritSec) == 8);
static_assert(offsetof(PerfTimerMgr, mConfig) == 24);
static_assert(offsetof(PerfTimerMgr, mTimerNames) == 32);
static_assert(offsetof(PerfTimerMgr, mThreadTimers) == 64);

// The manager, at 0x19E7CD0. The map names the PerfMgr.o object
// thePerfMgr; that it is a PerfTimerMgr is inferred from the calls on it.
extern PerfTimerMgr thePerfMgr;
