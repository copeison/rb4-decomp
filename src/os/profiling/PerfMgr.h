#pragma once

#include <_pthread.h>
#include <cstddef>

#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

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

    // Field names are not in the reference map.
    unsigned char mUnknown0[64];
    eastl::vector<ThreadTimers*> mThreadTimers;
};

static_assert(offsetof(PerfTimerMgr, mThreadTimers) == 64);

// The manager, at 0x19E7CD0. The map names the PerfMgr.o object
// thePerfMgr; that it is a PerfTimerMgr is inferred from the calls on it.
extern PerfTimerMgr thePerfMgr;
