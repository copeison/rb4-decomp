#pragma once

#include <_pthread.h>

// Runs the entity polls on the main thread and a pool of worker threads
// (utl/PollMgr.o). Only the members the reconstructed code uses are
// declared; the manager has not been reconstructed.
class PollMgr {
public:
    // Whether the thread is one of the poll workers, which the manager
    // keeps at 0x19E7EC0. Name not in the reference map.
    static bool IsWorkerThread(ScePthread thread);  // 0x24F6E0
};
