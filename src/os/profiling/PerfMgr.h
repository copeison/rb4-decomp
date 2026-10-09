#pragma once

#include <_pthread.h>
#include <cstddef>

#include "os/threading/CritSec.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataArray.h"
#include "utl/text/Symbol.h"

class PerfTimer;
class PerfTimerBase;

// The registry of CPU timers (os/PerfMgr.o). Each thread keeps its own
// timers, indexed by the timer name's index in mTimerNames.
class PerfTimerMgr {
public:
    // The timers of one thread. Name not in the reference map, whose build
    // keeps an eastl::pair<pthread*, PerfTimer**> per thread; the field
    // names are not in it either.
    struct ThreadTimers {
        // Unreferenced in this build. Name not in the reference map.
        ThreadTimers(ScePthread thread, eastl::vector<PerfTimerBase*>* timers);  // 0x249970

        ScePthread mThread;
        eastl::vector<PerfTimerBase*>* mTimers;  // Null entries are unused.
    };

    // Inlined into the static initializer at 0x24A620.
    PerfTimerMgr() : mEnabled(false) {}
    ~PerfTimerMgr();  // 0x2498B0

    // Enables the manager with the timer configuration and registers the
    // calling thread. The map's build splits it into Init and _Init.
    void Init(DataArray* config);  // 0x249650

    // Guard the thread tables while the timer overlays read them.
    void Lock();    // 0x249980
    void Unlock();  // 0x2499A0

    // The calling thread's timer with the name, registering the name when it
    // is new. Null when the manager is disabled.
    PerfTimer* GetTimer(Symbol name);  // 0x2499B0
    // The name's index, registering it when it is new.
    unsigned long GetTimerIndex(Symbol name);  // 0x2499D0
    // The calling thread's timer with the name index, created on first use.
    // Null when the manager is disabled.
    PerfTimer* GetTimer(unsigned long index);  // 0x249B70

    // Sets every timer's expanded flag. Called by the timer script commands
    // (0x24BAE0). Name not in the reference map.
    void SetAllExpanded(bool expanded);  // 0x249CA0
    // Ends the frame of every timer; `reset` also resets their worst times
    // and averages. RndDevice calls it as a frame begins and ends. The map's
    // signature is Poll().
    void Poll(bool reset);  // 0x249D40

    // Creates the calling thread's timer for the name, unless its
    // configuration disables it. The map's signature is
    // _AddTimer(PerfTimerCfg const&).
    PerfTimer* _AddTimer(Symbol name, DataArray* config);  // 0x249E50
    // Registers the calling thread and creates its configured timers.
    void _ThreadInit();  // 0x24A0B0
    // Deletes the thread's timers and forgets the thread; its thread data
    // calls it as the thread exits. Name not in the reference map.
    void _ThreadTerminate(ScePthread thread);  // 0x24A330

    // Field names are not in the reference map.
    // Set by Init; GetTimer(unsigned long) returns null while it is clear.
    bool mEnabled;
    // Guards the tables; Lock and Unlock enter and leave it.
    CritSec mCritSec;
    // The timer configuration; _ThreadInit registers a timer for each of
    // its entries.
    DataArrayPtr mConfig;
    // The registered timer names; GetTimerIndex(Symbol) returns the index.
    eastl::vector<Symbol> mTimerNames;
    eastl::vector<ThreadTimers*> mThreadTimers;
};

static_assert(offsetof(PerfTimerMgr, mCritSec) == 8);
static_assert(offsetof(PerfTimerMgr, mConfig) == 24);
static_assert(offsetof(PerfTimerMgr, mTimerNames) == 32);
static_assert(offsetof(PerfTimerMgr, mThreadTimers) == 64);
static_assert(sizeof(PerfTimerMgr) == 96);

// The manager, at 0x19E7CD0. The map names the PerfMgr.o object
// thePerfMgr; that it is a PerfTimerMgr is inferred from the calls on it.
extern PerfTimerMgr thePerfMgr;

// Starts the timers: the cycle timer, the budget categories and thePerfMgr
// with the "timer" configuration. Name not in the reference map.
void PerfInit(DataArray* config);  // 0x249610
