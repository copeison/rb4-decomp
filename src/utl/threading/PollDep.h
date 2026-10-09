#pragma once

#include <atomic>
#include <cstddef>

#include "utl/containers/LinkedList.h"
#include "utl/containers/Vector.h"

class CritSec;
class PollMgr;

// A job the poll manager runs once per frame after the jobs it depends on
// (utl/PollDep.o, 0x24BC80-0x24CB60). Each job lists the jobs that must
// wait for it (its afters) and counts the jobs it still waits for; when the
// count drops to zero the job is queued in a poll bucket. Two counters
// alternate between frames, so a frame's releases do not disturb the
// next's. The vtable at 0x18EF498 has ten slots. Entity is built on it at
// +24.
class PollDepBase {
public:
    // Reaches the link that queues a job in a poll bucket; the map's
    // LinkedList::List<PollDepBase, PollDepBase::PollNode>.
    class PollNode {
    public:
        static LinkedList::Node& ToNode(PollDepBase& dep) {
            return dep.mPollNode;
        }
        static PollDepBase* FromNode(LinkedList::Node* node) {
            return reinterpret_cast<PollDepBase*>(
                reinterpret_cast<char*>(node) - offsetof(PollDepBase, mPollNode));
        }
    };

    // Reaches the link that queues a job for the post-poll. Name not in
    // the reference map.
    class PostPollNode {
    public:
        static LinkedList::Node& ToNode(PollDepBase& dep) {
            return dep.mPostPollNode;
        }
        static PollDepBase* FromNode(LinkedList::Node* node) {
            return reinterpret_cast<PollDepBase*>(
                reinterpret_cast<char*>(node) - offsetof(PollDepBase, mPostPollNode));
        }
    };

    using PollList = LinkedList::List<PollDepBase, PollNode>;

    // The buckets a released job is queued in, by where it may run and by
    // whether jobs wait for it. The map names PollBucket; the enumerator
    // names are not in the reference map.
    enum PollBucket {
        // A threaded job heavier than PollMgr::sMaxPollWeight.
        kPollBucketHeavy = 0,
        // A threaded job that other jobs wait for.
        kPollBucketThreaded = 1,
        // A threaded job nothing waits for, or only a nested manager.
        kPollBucketThreadedLeaf = 2,
        // A main-thread job that other jobs wait for.
        kPollBucketMain = 3,
        // A main-thread job nothing waits for.
        kPollBucketMainLeaf = 4,
        kNumPollBuckets = 5,
    };

    PollDepBase();  // 0x24BC80
    // Copies only the thread mask and the threaded flag.
    PollDepBase(const PollDepBase& other);  // 0x24BD40
    // Slots 0-1: 0x24BE10, 0x24BEA0.
    virtual ~PollDepBase();
    // Slot 2 at 0x24CB40: whether the job may poll now; a job that may not
    // releases its afters at once. True here. Name not in the reference
    // map.
    virtual bool IsPollEnabled() const {
        return true;
    }
    // Slot 3: runs the job. The map's signature is ThreadPoll(); this build
    // passes the worker's index, which the jobs seen ignore.
    virtual void ThreadPoll(const int& thread) = 0;
    // Slot 4 at 0x24CB50: runs after every job of the frame polled, for
    // jobs queued with _AddPostPoll; empty here. Name not in the reference
    // map.
    virtual void PostPoll() {}
    // Slot 5 at 0xF79B0: the job's name for reports. Name not in the
    // reference map.
    virtual const char* GetPollName() const {
        return "(un-named PollDep job)";
    }
    // Slots 6-7 at 0x1270F0 and 0x127100: told before the job gains or
    // loses a dependency; empty here. Names not in the reference map.
    virtual void _OnAddPollDep() {}
    virtual void _OnRemovePollDep() {}
    // Slots 8-9 at 0xF7A20 and 0xF7A30: the job when it stands for a
    // nested poll manager, which a class keeps right after the base
    // (_NestedPollMgr). Null here and in every class found. Names not in
    // the reference map; the evidence is weak.
    virtual PollDepBase* AsPollMgrDep() const {
        return nullptr;
    }
    virtual PollDepBase* AsPollMgrJob() const {
        return nullptr;
    }

    // Sets the job's cost, clamped to 1 to PollMgr::sMaxPollWeight unless
    // `exact` is set. The map's signature is SetPollWeight(unsigned short).
    void SetPollWeight(unsigned int weight, bool exact);  // 0x24BF40
    // Lets the job run on a worker thread.
    void SetThreaded(bool threaded);  // 0x24BF70
    // The workers the job may run on, cut to the worker count.
    void SetThreadMask(unsigned int mask);  // 0x24BF90
    // Releases the job's afters after it polled; the job is then no longer
    // queued. Name not in the reference map.
    void _OnPolled(bool canQueue);  // 0x24BFC0
    // Releases each after of this frame and queues those left without
    // dependencies in the thread's poll manager; the afters may be queued
    // only when `canQueue` is set and the job may poll. Swaps the frame's
    // counters.
    void _FreeAfters(bool canQueue);  // 0x24BFE0
    // Releases one after before the job finishes polling, when
    // PollMgr::sEarlyFreeToPoll is set and the thread's poll context asks
    // for it. Name not in the reference map.
    void EarlyFreeToPoll(PollDepBase* after);  // 0x24C1F0
    // Releases the after; when no dependency is left it is queued in its
    // bucket, or its own afters are released when it may not poll. True
    // when it was queued. Name not in the reference map.
    bool _FreeAfter(PollDepBase* after, bool canQueue, PollList* buckets);  // 0x24C420
    // Makes the job wait for `dep`.
    void AddPollDep(PollDepBase* dep);  // 0x24C5D0
    // Stops the job waiting for `dep`; false when it did not. Name not in
    // the reference map.
    bool TryRemovePollDep(PollDepBase* dep);  // 0x24C6E0
    // As TryRemovePollDep; a missing dependency is reported, though the
    // report is compiled out.
    void RemovePollDep(PollDepBase* dep);  // 0x24C770
    // Whether the job waits for `dep`.
    bool HasPollDep(PollDepBase* dep) const;  // 0x24C810
    // Stops every job waiting for this one and leaves the root list and
    // the post-poll queue.
    void ClearAllDeps();  // 0x24C840
    // Leaves the post-poll queue.
    void _RemovePostPoll();  // 0x24C970
    // Forgets the afters, the counters and the manager, and leaves the
    // root list.
    void _Reset();  // 0x24CA00
    // Joins its manager's post-poll queue. Name not in the reference map.
    void _AddPostPoll();  // 0x24CA60
    // Runs the job on the worker. Name not in the reference map.
    void _DoThreadPoll(int thread);  // 0x24CB00

    // The manager a class that overrides slots 8-9 keeps right after the
    // base. Name not in the reference map.
    PollMgr* _NestedPollMgr() const {
        return *reinterpret_cast<PollMgr* const*>(
            reinterpret_cast<const char*>(this) + sizeof(PollDepBase));
    }

    // The counter of the jobs this frame's job still waits for, and the
    // next frame's. Name not in the reference map.
    std::atomic<int>& _CurDepCount() {
        return mDepCounts[(mFlags & kFlagOddFrame) != 0];
    }
    std::atomic<int>& _NextDepCount() {
        return mDepCounts[(mFlags & kFlagOddFrame) == 0];
    }

    // The flags in mFlags above the thread mask. Names not in the
    // reference map.
    enum {
        kThreadMask = 0xFF,
        kFlagThreaded = 0x100,
        // Selects which of mDepCounts is the current frame's.
        kFlagOddFrame = 0x200,
        // Set while a released job was kept from polling; _OnPolled clears
        // it.
        kFlagPollSkipped = 0x400,
    };

    // Field names are not in the reference map.
    LinkedList::Node mPollNode;
    // Linked only by the constructors and unlinked by the destructor here;
    // its users are not identified.
    LinkedList::Node mAuxNode;
    // Links a job without dependencies into its manager's root list
    // (0x24CFC0).
    LinkedList::Node mRootNode;
    LinkedList::Node mPostPollNode;
    // The jobs that wait for this one; the first mNumAfters wait this
    // frame.
    eastl::vector<PollDepBase*> mAfters;
    unsigned long mNumAfters;
    std::atomic<int> mDepCounts[2];
    unsigned short mPollWeight;
    unsigned short mFlags;
    unsigned char mPadding[4];  // Never read or written.
    // The manager that polls the job, which its afters inherit.
    PollMgr* mPollMgr;
    // The lock of the post-poll queue the job is in, or null.
    CritSec* mPostPollCrit;
};

static_assert(offsetof(PollDepBase, mPollNode) == 8);
static_assert(offsetof(PollDepBase, mRootNode) == 40);
static_assert(offsetof(PollDepBase, mPostPollNode) == 56);
static_assert(offsetof(PollDepBase, mAfters) == 72);
static_assert(offsetof(PollDepBase, mNumAfters) == 104);
static_assert(offsetof(PollDepBase, mDepCounts) == 112);
static_assert(offsetof(PollDepBase, mPollWeight) == 120);
static_assert(offsetof(PollDepBase, mFlags) == 122);
static_assert(offsetof(PollDepBase, mPollMgr) == 128);
static_assert(offsetof(PollDepBase, mPostPollCrit) == 136);
static_assert(sizeof(PollDepBase) == 144);
