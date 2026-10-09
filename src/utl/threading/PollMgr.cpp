#include "utl/threading/PollMgr.h"

// The calling thread's poll state. The map's PollMgr::sThreadPolls; this
// build keeps it in the per-thread block at the offset recorded at
// 0x19B03D0.
thread_local ThreadPollContext gEntityThreadState;
