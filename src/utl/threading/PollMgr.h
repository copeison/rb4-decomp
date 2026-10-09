#pragma once

#include <_pthread.h>
#include <cstddef>

#include "os/threading/CritSec.h"
#include "utl/containers/LinkedList.h"
#include "utl/containers/Vector.h"
#include "utl/threading/PollDep.h"

class Entity;

// Runs the entity polls on the main thread and a pool of worker threads
// (utl/PollMgr.o). Only the members the reconstructed code uses are
// declared; the manager has not been reconstructed.
class PollMgr {
public:
    // Whether the thread is one of the poll workers, which the manager
    // keeps at 0x19E7EC0. Name not in the reference map.
    static bool IsWorkerThread(ScePthread thread);  // 0x24F6E0

    // Queues the released jobs of each bucket in the manager's lists. The
    // map's signature is QueueForPoll(LinkedList::List<PollDepBase,
    // PollDepBase::PollNode>&, LinkedList::List<PollDepBase,
    // PollDepBase::PollNode>*); this build passes only the buckets.
    void QueueForPoll(PollDepBase::PollList* buckets);  // 0x2501D0
    // Drops the entity's queued resource loads. The map's signature takes
    // an EntityPtr const&.
    void DequeueForLoadResourcesIfNeeded(Entity* entity);  // 0x250C40

    // The "toggle_early_free_to_poll" console switch
    // (PollDepBase::EarlyFreeToPoll). Name not in the reference map.
    static bool sEarlyFreeToPoll;  // 0x19E7EA1
    // The heaviest job a worker's light bucket takes, at least 1; set by
    // "set_max_weight" and 0x24F720. Name not in the reference map.
    static unsigned short sMaxPollWeight;  // 0x19B03D8
    // The worker threads. Name not in the reference map.
    static eastl::vector<ScePthread> sWorkerThreads;  // 0x19E7EC0

    // Field names are not in the reference map.
    unsigned char mPadding[438];  // Not modelled.
    // When set on a nested manager, the jobs it stands for keep the
    // releasing job's manager (PollDepBase::_FreeAfter). Cleared by the
    // constructor (0x24CB60) and set by no code found; the evidence for the
    // name is weak.
    bool mSharePollMgr;
    // The lock of the post-poll queue and the queue.
    CritSec mPostPollCrit;
    LinkedList::List<PollDepBase, PollDepBase::PostPollNode> mPostPolls;
};

static_assert(offsetof(PollMgr, mSharePollMgr) == 438);
static_assert(offsetof(PollMgr, mPostPollCrit) == 440);
static_assert(offsetof(PollMgr, mPostPolls) == 456);

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
