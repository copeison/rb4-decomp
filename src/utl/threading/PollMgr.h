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
    // The managers the renderer nests are 544-byte small-pool blocks
    // (PoolAlloc), named for reports. The map has PollMgr(); this build
    // takes the name.
    explicit PollMgr(const char* name);  // 0x24CB60
    // Reads the job_manager and poll_mgr configuration and registers the
    // pollmgr script functions. Not reconstructed.
    static void Init();  // 0x24E0B0
    ~PollMgr();                          // 0x24CDA0
    // Sets the flag at +436 and lets the end job run on a worker. Name not
    // in the reference map.
    void SetThreaded(bool threaded);  // 0x24CEE0
    // Unlinks the queued jobs, resets the start and end jobs and makes the
    // end job wait for the start job. Name not in the reference map.
    void Reset();  // 0x24D0D0
    // Unlinks the jobs the end job waits for, clears the start job's
    // dependencies and makes the end job wait for the start job. Name not
    // in the reference map.
    void ClearJobs();  // 0x24D190
    // Makes the job wait for the start job and queues it. Name not in the
    // reference map.
    void AddJob(PollDepBase* job);  // 0x24D3A0
    // Makes the start job wait for, or stop waiting for, the other
    // manager's end job. Names not in the reference map.
    void PollAfter(PollMgr* other);        // 0x24D310
    void RemovePollAfter(PollMgr* other);  // 0x24D340

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

// A nested poll manager ("PollMgr PollGroup") with a lock, which a renderer
// object selects as the thread's manager around the polls it groups
// (0x24F740-0x24FA90). The scene component and the drawable entity
// resource own one each. Only its constructor and destructor are declared.
// Name not in the reference map; it follows the manager's name.
class PollGroup {
public:
    // The binary does not read the name.
    explicit PollGroup(const char* name);  // 0x24F740
    ~PollGroup();                          // 0x24F820

    // Makes the group the thread's poll manager (also stored at
    // 0x19E7F48). Name not in the reference map.
    void Select();  // 0x24F9B0
    // Polls the group's queued jobs on the calling thread until none is
    // left; a non-null job polls only up to it. Name not in the reference
    // map.
    void PollJobs(PollDepBase* until);  // 0x24FB80
    // Clears the thread's poll manager. Name not in the reference map.
    static void Deselect();  // 0x24FA90

    unsigned char mOpaque[88];  // Not modelled.
};

static_assert(sizeof(PollGroup) == 88);

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
