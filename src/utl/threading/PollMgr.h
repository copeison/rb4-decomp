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

// The calling thread's poll state (the map's ThreadPollContext).
// RecordingAudioRenderTarget and TransEntityResource clear the flag while
// they enter an entity. Field names are not in the reference map.
struct ThreadPollContext {
    // The poll manager the thread is polling for, set by the manager's
    // select (0x24F900, 0x24F9B0) and cleared by its deselect (0x24FA90).
    // Entities check it with the flag before deferring their enter and poll
    // work (0xF53A0).
    PollMgr* mPollMgr;
    // While it and mPollMgr are set, an entity that becomes ready on this
    // thread enters and polls at once (0xF53A0); TransEntityResource clears
    // it around EnterEntity (0x1BB580).
    bool mEnterImmediately;
};

// The map's PollMgr::sThreadPolls, a TLSValue<ThreadPollContext> whose
// descriptor and offset are at 0x19B03C8. The name is kept until its users
// move to the member. Name not in the reference map.
extern thread_local ThreadPollContext gEntityThreadState;
