#pragma once

#include <_pthread.h>
#include <atomic>
#include <cstddef>
#include <functional>

#include "entity/core/GameObject.h"
#include "os/memory/PoolAlloc.h"
#include "os/threading/Condition.h"
#include "os/threading/CritSec.h"
#include "utl/containers/LinkedList.h"
#include "utl/containers/List.h"
#include "utl/containers/Map.h"
#include "utl/containers/Vector.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"
#include "utl/threading/PollDep.h"

class Component;
class DataArray;
class DataNode;
class Entity;
class GameObject;
class NamedThread;
class PollGroup;
class TextStream;

namespace ThreadMap {
struct TaskDesc;
}

// The poll manager (utl/PollMgr.o, 0x24CB60-0x2518E0). The map's PollMgr
// is split in this build: a PollGroup holds a set of jobs between a head
// and a tail job, and the 88-byte PollMgr polls the groups' jobs on the
// main thread and a pool of worker threads, which the manager's statics
// keep. A thread polls for the manager it selects. The strings name the
// pieces: "PollMgr PollGroup", "<PollGroup Head>", "<PollGroup Tail>",
// "PollMgr Worker %d".

// A poll group's head or tail job: it remembers the group (vtable
// 0x18EF5B8, with ThreadPoll pure). Name not in the reference map.
class PollGroupJob : public PollDepBase {
public:
    explicit PollGroupJob(PollGroup* owner) : mOwner(owner) {
        SetThreaded(true);
    }
    // Slots 0-1: 0x24DF90, 0x24DFA0.
    ~PollGroupJob() override {}

    // Name not in the reference map.
    PollGroup* mOwner;
};

static_assert(offsetof(PollGroupJob, mOwner) == 144);
static_assert(sizeof(PollGroupJob) == 152);

// A set of jobs bracketed by two jobs: the head, which every job of the
// group waits for, and the tail, which waits for the group's leaf jobs
// (those nothing else waits for). Another group polls after this one
// through the head's dependency on the other's tail; a group is nested in
// another as a job through its head and tail. Its own jobs' post-polls run
// when the tail polls, or later on the manager for a threaded group. Its
// callbacks run when the head and the tail poll. The 544 bytes come from
// the small-block pool. Name not in the reference map; it follows the
// strings of its jobs and of PollMgr's group.
class PollGroup {
public:
    POOL_OVERLOAD(PollGroup)

    // The job every job of the group waits for (vtable 0x18EF4F8). Name
    // not in the reference map.
    class HeadJob : public PollGroupJob {
    public:
        explicit HeadJob(PollGroup* owner);  // 0x24DB60
        // Slots 0-1: 0x24CED0, 0x24DEF0.
        ~HeadJob() override {}
        // Slot 3: starts the group's poll.
        void ThreadPoll(const int& thread) override;  // 0x24DF10
        const char* GetPollName() const override;     // slot 5: 0x24DBB0
        PollGroupJob* AsPollGroupHead() override;     // slot 8: 0x24DF50
    };

    // The job that waits for the group's leaf jobs (vtable 0x18EF558). Name
    // not in the reference map.
    class TailJob : public PollGroupJob {
    public:
        explicit TailJob(PollGroup* owner);  // 0x24DBC0
        // Slots 0-1: 0x24CEC0, 0x24DF60.
        ~TailJob() override {}
        // Slot 3: ends the group's poll and, for a group polled on the
        // main thread, runs its post-polls.
        void ThreadPoll(const int& thread) override;  // 0x24DC20
        const char* GetPollName() const override;     // slot 5: 0x24DC10
        PollGroupJob* AsPollGroupTail() override;     // slot 9: 0x24DF80
    };

    // Reaches the link that queues a finished group on its manager
    // (PollMgr::mFinishedGroups). Name not in the reference map.
    class FinishedNode {
    public:
        static LinkedList::Node& ToNode(PollGroup& group) {
            return group.mFinishedNode;
        }
        static PollGroup* FromNode(LinkedList::Node* node) {
            return reinterpret_cast<PollGroup*>(
                reinterpret_cast<char*>(node) - offsetof(PollGroup, mFinishedNode));
        }
    };

    // mState values; PollMgr::mState uses them too. Names not in the
    // reference map.
    enum State {
        kStateIdle = 0,
        kStatePolling = 1,
        kStatePostPolling = 2,
    };

    // The tail waits for the head. The name is truncated to 32 characters.
    explicit PollGroup(const char* name);  // 0x24CB60
    ~PollGroup();                          // 0x24CDA0

    // Sets mThreaded and lets the tail run on a worker. Name not in the
    // reference map.
    void SetThreaded(bool threaded);  // 0x24CEE0
    // Sets mPersistent. No caller is found. Name not in the reference map;
    // the evidence is weak.
    void SetPersistent(bool persistent);  // 0x24CF00
    // Copies the callbacks the head and the tail run; the callers pass
    // copies. Name not in the reference map.
    void SetCallbacks(std::function<void()> onPollStart, std::function<void()> onPollEnd);  // 0x24CF10
    // Makes the tail wait for the job's leaf jobs: the job when nothing
    // waits for it, else the leaves of its afters. A nested group stands
    // for its tail. Name not in the reference map.
    void _AddLeafJobs(PollDepBase* job);  // 0x24CFC0
    // Unlinks the post-polls, resets the head and the tail, forgets the
    // leaf jobs and makes the tail wait for the head. Name not in the
    // reference map.
    void Reset();  // 0x24D0D0
    // Stops the tail waiting for the leaf jobs, clears the head's
    // dependencies and makes the tail wait for the head. Name not in the
    // reference map.
    void ClearJobs();  // 0x24D190
    // As ClearJobs for the leaf jobs only; returns the tail's remaining
    // dependency count. No caller is found. Name not in the reference map.
    int _ClearLeafJobs();  // 0x24D270
    // Makes the head wait for, or stop waiting for, the other group's
    // tail; TryRemovePollAfter tells whether it waited. Names not in the
    // reference map.
    void PollAfter(PollGroup* other);           // 0x24D310
    void RemovePollAfter(PollGroup* other);     // 0x24D340
    bool TryRemovePollAfter(PollGroup* other);  // 0x24D370
    // Makes the job wait for the head and the tail for the job's leaves.
    // Name not in the reference map.
    void AddJob(PollDepBase* job);  // 0x24D3A0
    // Nests the groups whose head waits for nothing, in turn. No caller is
    // found. Name not in the reference map.
    void AddPollGroups(eastl::list<PollGroup*>& groups);  // 0x24D3F0
    // Nests the group: its head waits for this head, and this tail for its
    // tail. Name not in the reference map.
    void AddPollGroup(PollGroup* group);  // 0x24D490
    // Runs the post-polls queued on the group. Name not in the reference
    // map; the map's PollMgr::_PostPoll() did this.
    void _PostPoll();  // 0x24D4D0
    // Tells the thread's poll manager that the group's tail polled. Name
    // not in the reference map.
    void _Finish();  // 0x24D590
    // The number of jobs waiting for the head, without the tail. Name not
    // in the reference map.
    int GetNumJobs();  // 0x24D620
    // Debug reports with no caller in this build: the groups waiting for
    // the tail ("-- PrintAfters: "), and the nested groups and the groups'
    // dependencies under them. Names not in the reference map except
    // PrintAfters, which the report names.
    void PrintAfters(TextStream& stream);                                 // 0x24D650
    void Print(TextStream& stream);                                       // 0x24D720
    void _PrintWrapped(TextStream& stream, PollGroup* root, int indent);  // 0x24D7A0
    void _PrintDeps(TextStream& stream, PollGroup* root, int indent);     // 0x24D870
    // Searches the group's jobs for a dependency cycle; the search is
    // compiled out of this build. No caller is found. Name not in the
    // reference map.
    void CheckForCycles();  // 0x24DD00

    // Field names are not in the reference map.
    // Run by the head and the tail when they poll.
    std::function<void()> mOnPollStart;
    std::function<void()> mOnPollEnd;
    // Queues the group on its manager once its tail polled.
    LinkedList::Node mFinishedNode;
    // The jobs nothing waits for, which the tail waits for.
    LinkedList::List<PollDepBase, PollDepBase::LeafNode> mLeafJobs;
    HeadJob mHead;
    TailJob mTail;
    State mState;
    // The tail runs on a worker, and the manager runs the post-polls.
    bool mThreaded;
    // Set by the constructor; a finished group with it clear is destroyed
    // by its manager (0x24FC50). Weak evidence for the name.
    bool mPersistent;
    // When set on a nested group, its head and tail take the group of the
    // job that releases them (PollDepBase::_FreeAfter). Set on the scene's
    // split-poll groups (RndSceneCom::AddPollSplitJob).
    bool mShareParentGroup;
    // The lock of the post-poll queue and the queue.
    CritSec mPostPollCrit;
    LinkedList::List<PollDepBase, PollDepBase::PostPollNode> mPostPolls;
    // The group a post-polling job belongs to while its PostPoll runs.
    // Cleared by the constructor; nothing found writes it.
    PollGroup* mPostPollGroup;
    StackString<32> mName;
};

static_assert(offsetof(PollGroup, mOnPollEnd) == 48);
static_assert(offsetof(PollGroup, mFinishedNode) == 96);
static_assert(offsetof(PollGroup, mLeafJobs) == 112);
static_assert(offsetof(PollGroup, mHead) == 128);
static_assert(offsetof(PollGroup, mTail) == 280);
static_assert(offsetof(PollGroup, mState) == 432);
static_assert(offsetof(PollGroup, mThreaded) == 436);
static_assert(offsetof(PollGroup, mPersistent) == 437);
static_assert(offsetof(PollGroup, mShareParentGroup) == 438);
static_assert(offsetof(PollGroup, mPostPollCrit) == 440);
static_assert(offsetof(PollGroup, mPostPolls) == 456);
static_assert(offsetof(PollGroup, mPostPollGroup) == 472);
static_assert(offsetof(PollGroup, mName) == 480);
static_assert(sizeof(PollGroup) == 544);

// Poll groups by key, which a poll manager can be selected with
// (PollMgr::Select, ThreadPollContext::mPollGroups). The vtable at
// 0x18FFC60 has only Reset; RndSceneCom::PollGroups derives from it. All
// members are inline. Name not in the reference map; the evidence is weak.
class PollGroupMap {
public:
    PollGroupMap() {}
    // Destroys the groups. Not virtual: the vtable has no destructor.
    ~PollGroupMap() {
        for (auto it = mPollGroups.begin(); it != mPollGroups.end(); ++it) {
            delete it->second;
        }
    }

    // Slot 0 (0x412570 in RndSceneCom.o): resets each group.
    virtual void Reset() {
        for (auto it = mPollGroups.begin(); it != mPollGroups.end(); ++it) {
            it->second->Reset();
        }
    }

    // Field name not in the reference map.
    eastl::map<unsigned int, PollGroup*> mPollGroups;
};

static_assert(offsetof(PollGroupMap, mPollGroups) == 8);
static_assert(sizeof(PollGroupMap) == 64);

// Polls poll groups: the selected manager's jobs run on the calling
// thread and on the worker threads, and finished groups are queued on it.
// The main thread polls the main-thread jobs and shares the light threaded
// jobs; workers take the heavy jobs first. One worker may run only while
// the audio mixer is held (BlockAudioThread). The map's PollMgr; this
// build's manager is 88 bytes and owns a group ("PollMgr PollGroup").
class PollMgr {
public:
    // A worker thread's place: its index, its bit in thread masks, and a
    // lock and a list that nothing uses. 40 bytes. Field names are not in
    // the reference map.
    struct ThreadDesc {
        ThreadDesc() : mMask(0) {}
        ThreadDesc(int index, unsigned int mask) : mIndex(index), mMask(mask) {}
        // Copies only the index and the mask.
        ThreadDesc(const ThreadDesc& other) : mIndex(other.mIndex), mMask(other.mMask) {}
        ThreadDesc& operator=(const ThreadDesc& other) {
            mIndex = other.mIndex;
            mMask = other.mMask;
            return *this;
        }

        int mIndex;
        unsigned int mMask;
        CritSec mCrit;
        PollDepBase::PollList mJobs;
    };

    // A component whose resources load once the manager's polls end
    // (QueueForLoadResources). The map's constructor takes an ObjPtr const&
    // and a Component&. Field names are not in the reference map.
    struct LoadResourcesEntry {
        Entity* mEntity;
        GameObjectId mObject;
        Symbol mComponent;
    };

    // The states of the audio mixer lock (sAudioLockState), named by the
    // strings of sAudioLockStateNames.
    enum AudioLockState : int {
        kUnlocked = 0,
        kNeedLock = 1,
        kLocking = 2,
        kLocked = 3,
        kNeedUnlock = 4,
        kLockingNeedUnlock = 5,
        kUnlocking = 6,
    };

    // Creates the manager's group, threaded.
    explicit PollMgr(const char* name);  // 0x24F740
    ~PollMgr();                          // 0x24F820

    // Make the manager the polled one and the thread's, with the poll
    // groups the thread's entities use. The map's Select().
    void Select(PollGroupMap* pollGroups);  // 0x24F900
    // As Select(PollGroupMap*) without groups, and wakes the workers.
    void Select();  // 0x24F9B0
    // Releases a mixer lock a worker still holds and clears the polled and
    // the thread's manager.
    static void Deselect();  // 0x24FA90
    // Polls until the group finished, or, without a group, until no job
    // is left anywhere; then loads the queued resources. The map's
    // Sync(bool).
    void Sync(PollGroup* group);  // 0x24FB80
    // Runs the jobs the main thread may take until the group finished,
    // or, without a group, until none is left, then frees the finished
    // groups and runs the manager group's post-polls. The map's _Poll().
    void _Poll(PollGroup* until);  // 0x24FC50
    // Whether a worker is busy or a job is queued. Inlined into Sync.
    // Name not in the reference map.
    bool _HasPendingJobs();  // 0x24FFA0
    // Loads the resources QueueForLoadResources queued, of the components
    // that still exist. Name not in the reference map.
    static void _LoadQueuedResources();  // 0x250020
    // Polls the group's head on the calling thread, releasing its jobs.
    // Name not in the reference map.
    void StartPollGroup(PollGroup* group);  // 0x250160
    // Queues the released jobs of each bucket: heavy and threaded jobs on
    // the workers' lists, main-thread jobs on the manager's; without
    // workers everything goes to the manager. The map's signature is
    // QueueForPoll(LinkedList::List<PollDepBase, PollDepBase::PollNode>&,
    // LinkedList::List<PollDepBase, PollDepBase::PollNode>*); this build
    // passes only the buckets.
    void QueueForPoll(PollDepBase::PollList* buckets);  // 0x2501D0
    // Unqueues the group when it finished. Inlined into _Poll. Name not in
    // the reference map.
    bool _TakeFinishedGroup(PollGroup* group);  // 0x250450
    // Moves the jobs the main thread may run to `jobs`: the manager's, else
    // one heavy job, else threaded jobs up to sMaxPollWeight. False once
    // nothing is queued and no worker is busy. The map's signature takes
    // only the list.
    bool _PullJobFromMainThread(PollDepBase::PollList& jobs);  // 0x2504C0
    // Polls the jobs in turn on the thread; a group's tail finishes its
    // group. Inlined into its callers. The map's signature is
    // _DoJob(LinkedList::List<PollDepBase, PollDepBase::PollNode>&,
    // unsigned long).
    static void _DoJob(PollDepBase::PollList& jobs, int thread);  // 0x2506F0
    // Destroys the finished groups that are not persistent. Inlined into
    // _Poll. Name not in the reference map.
    void _FreeFinishedGroups();  // 0x250790
    // Queues the finished group. Name not in the reference map.
    void _AddFinishedGroup(PollGroup* group);  // 0x250870
    // Moves the jobs the worker may run to `jobs`: one heavy job, else
    // threaded jobs for its mask up to sMaxPollWeight, and marks the worker
    // busy. False when there is nothing and the workers are terminating.
    // The map's signature is _PullJobFromWorker(LinkedList::List<PollDepBase,
    // PollDepBase::PollNode>&, int).
    static bool _PullJobFromWorker(ThreadDesc& desc, PollDepBase::PollList& jobs);  // 0x2508D0
    // Queues the component's resource load for the end of the polls, once.
    // The map's signature is QueueForLoadResources(ObjPtr const&,
    // Component&).
    void QueueForLoadResources(GameObject* object, Component* component);  // 0x250AD0
    // Drops the entity's queued resource loads. The map's signature takes
    // an EntityPtr const&.
    void DequeueForLoadResourcesIfNeeded(Entity* entity);  // 0x250C40

    // Reads the poll_mgr configuration, starts the "job_manager" workers
    // and registers the "pollmgr" script function.
    static void Init();  // 0x24E0B0
    // Starts a worker for each of the task's threads, replacing running
    // ones; one of them, on the "audio_render" processor or among the last,
    // is the audio worker when audio thread blocking is enabled. Without
    // workers, threading, audio blocking and early freeing are off. Name
    // not in the reference map.
    static void _StartWorkers(const ThreadMap::TaskDesc* desc);  // 0x24E1C0
    // The "pollmgr" script function: toggle_threads, set_max_weight,
    // toggle_sleep_post_job, set_thread_priority (ignored) and
    // toggle_early_free_to_poll. Name not in the reference map.
    static DataNode OnPollMgr(DataArray* msg);  // 0x24E800
    // Stops the workers. 0x24EC10 is a thunk to the body at 0x24EC20.
    static void Terminate();  // 0x24EC10
    // Name not in the reference map.
    static void _StopWorkers();  // 0x24EC20
    // The workers' entries: poll while a manager is selected, else wait.
    // The audio worker polls only while it holds the mixer lock. Names not
    // in the reference map.
    static int _WorkerMain(void* desc);       // 0x24EEA0
    static int _AudioWorkerMain(void* desc);  // 0x24F150
    // Wakes the workers, or only the plain ones while threading is on.
    // No caller is found. Name not in the reference map.
    static void WakeWorkers(bool all);  // 0x24F4C0
    // Sets sThreadsEnabled. Inlined into OnPollMgr. Name not in the
    // reference map.
    static void SetThreadsEnabled(bool enabled);  // 0x24F530
    // Requests or releases the audio mixer lock. While a manager is
    // selected the audio worker takes and drops it; otherwise the calling
    // thread does. Only with audio thread blocking and threading on. Name
    // not in the reference map.
    static void BlockAudioThread(bool block);  // 0x24F550
    // Whether the thread is one of the poll workers. Name not in the
    // reference map.
    static bool IsWorkerThread(ScePthread thread);  // 0x24F6E0
    // Sets sMaxPollWeight, at least 1. Name not in the reference map.
    static void SetMaxPollWeight(unsigned int weight);  // 0x24F720

    // Statics are in the order of the object's static initializer
    // (0x251230). Names not in the reference map unless noted.
    // Woken when a manager is selected, bound to sCS; and woken for the
    // audio worker, bound to sAudioWorkerCS.
    static Condition sWorkerCondition;       // 0x19E7E70
    static Condition sAudioWorkerCondition;  // 0x19E7E88
    // Threading is on: Init sets it, workers or not decide.
    static bool sThreadsEnabled;  // 0x19E7EA0
    // The "toggle_early_free_to_poll" console switch
    // (PollDepBase::EarlyFreeToPoll).
    static bool sEarlyFreeToPoll;  // 0x19E7EA1
    // The lock of the job lists and the busy mask, made with the workers.
    // The map's PollMgr::sCS.
    static CritSec* sCS;              // 0x19E7EA8
    static CritSec* sAudioWorkerCS;   // 0x19E7EB0
    // The masks of the workers that took jobs.
    static unsigned int sBusyMask;  // 0x19E7EB8
    static bool sTerminating;       // 0x19E7EBC
    static eastl::vector<NamedThread*> sWorkerThreads;  // 0x19E7EC0
    static eastl::vector<ThreadDesc> sThreadDescs;      // 0x19E7EE0
    // The light threaded jobs and the heavy jobs.
    static PollDepBase::PollList sThreadedJobs;  // 0x19E7F00
    static PollDepBase::PollList sHeavyJobs;     // 0x19E7F10
    // The "toggle_sleep_post_job" switch: workers yield after their jobs.
    static bool sSleepPostJob;  // 0x19E7F20
    // poll_mgr's "enabled_audio_thread_blocking".
    static bool sAudioThreadBlocking;  // 0x19E7F21
    // The last BlockAudioThread request.
    static bool sAudioBlocked;  // 0x19E7F22
    // The AudioLockState names; nothing reads them.
    static eastl::vector<const char*> sAudioLockStateNames;  // 0x19E7F28
    // The selected manager.
    static PollMgr* sCurrent;  // 0x19E7F48
    static eastl::vector<LoadResourcesEntry> sLoadResourcesQueue;  // 0x19E7F50
    static CritSec sLoadResourcesCrit;  // 0x19E7F70
    // The heaviest job a worker's light take reaches, at least 1.
    static unsigned short sMaxPollWeight;  // 0x19B03D8
    static std::atomic<int> sAudioLockState;  // 0x19E7FD0

    // Field names are not in the reference map.
    // The main-thread jobs.
    PollDepBase::PollList mJobs;
    // The lock of mFinishedGroups.
    CritSec mCrit;
    LinkedList::List<PollGroup, PollGroup::FinishedNode> mFinishedGroups;
    PollGroup* mPollGroup;
    // kStatePolling once constructed; kStatePostPolling while Sync loads
    // the queued resources.
    PollGroup::State mState;
    // Set to -1 by the constructor; nothing found reads them.
    long mReserved[3];
};

static_assert(offsetof(PollMgr, mCrit) == 16);
static_assert(offsetof(PollMgr, mFinishedGroups) == 32);
static_assert(offsetof(PollMgr, mPollGroup) == 48);
static_assert(offsetof(PollMgr, mState) == 56);
static_assert(offsetof(PollMgr, mReserved) == 64);
static_assert(sizeof(PollMgr) == 88);
static_assert(sizeof(PollMgr::ThreadDesc) == 40);
static_assert(sizeof(PollMgr::LoadResourcesEntry) == 24);

// The calling thread's poll state (the map's ThreadPollContext).
// RecordingAudioRenderTarget and TransEntityResource clear the flag while
// they enter an entity. Field names are not in the reference map.
struct ThreadPollContext {
    // The poll manager the thread is polling for, set by the manager's
    // select (0x24F900, 0x24F9B0) and cleared by its deselect (0x24FA90);
    // a worker sets it around its jobs. Entities check it with the flag
    // before deferring their enter and poll work (0xF53A0).
    PollMgr* mPollMgr;
    // While it and mPollMgr are set, an entity that becomes ready on this
    // thread enters and polls at once (0xF53A0); TransEntityResource clears
    // it around EnterEntity (0x1BB580).
    bool mEnterImmediately;
    // The poll groups the thread's entities poll under, given to
    // PollMgr::Select (0x24F900) and cleared by the other select and the
    // deselect.
    PollGroupMap* mPollGroups;
};

static_assert(offsetof(ThreadPollContext, mPollGroups) == 16);

// The map's PollMgr::sThreadPolls, a TLSValue<ThreadPollContext> whose
// descriptor and offset are at 0x19B03C8. The name is kept until its users
// move to the member. Name not in the reference map.
extern thread_local ThreadPollContext gEntityThreadState;

// Polls the child entity through its resource under the parent entity's
// poll group, which the child borrows for the poll; InstanceCom polls a
// child this way when the thread does not enter entities at once. Emitted
// after the poll manager (0x2514C0). Name not in the reference map.
void PollChildEntity(Entity* parent, Entity* child);  // 0x2514C0
