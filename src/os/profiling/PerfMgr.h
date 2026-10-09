#pragma once

#include "utl/text/Symbol.h"

class PerfTimer;

// The registry of CPU timers (os/PerfMgr.o). Only the lookup the debug
// overlays use is declared.
class PerfTimerMgr {
public:
    // The timer with the name, registering the name when it is new. Null
    // when the manager is disabled. Calls GetTimerIndex(Symbol) (0x2499D0)
    // and GetTimer(unsigned long) (0x249B70).
    PerfTimer* GetTimer(Symbol name);  // 0x2499B0
};

// The manager, at 0x19E7CD0. The map names the PerfMgr.o object
// thePerfMgr; that it is a PerfTimerMgr is inferred from the calls on it.
extern PerfTimerMgr thePerfMgr;
