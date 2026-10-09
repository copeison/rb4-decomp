#include "utl/threading/PollDep.h"

#include <kernel.h>

#include "os/threading/CritSec.h"
#include "utl/threading/PollMgr.h"

// Reconstructed from eboot.elf at 0x24BC80. A job starts threaded, with
// weight 1 and no workers in its mask.
PollDepBase::PollDepBase()
    : mNumAfters(0),
      mPollWeight(1),
      mFlags(kFlagThreaded),
      mPollMgr(nullptr),
      mPostPollCrit(nullptr) {
    mDepCounts[0] = 0;
    mDepCounts[1] = 0;
}

// Reconstructed from eboot.elf at 0x24BD40.
PollDepBase::PollDepBase(const PollDepBase& other)
    : mNumAfters(0),
      mPollWeight(1),
      mFlags(other.mFlags & (kThreadMask | kFlagThreaded)),
      mPollMgr(nullptr),
      mPostPollCrit(nullptr) {
    mDepCounts[0] = 0;
    mDepCounts[1] = 0;
}

// Reconstructed from eboot.elf at 0x24BE10.
PollDepBase::~PollDepBase() {
    mAfters.clear();
    mNumAfters = 0;
}

// Reconstructed from eboot.elf at 0x24BF40.
void PollDepBase::SetPollWeight(unsigned int weight, bool exact) {
    if (!exact) {
        unsigned int clamped = weight != 0 ? weight : 1;
        if (clamped > PollMgr::sMaxPollWeight) {
            clamped = PollMgr::sMaxPollWeight;
        }
        weight = clamped;
    }
    mPollWeight = static_cast<unsigned short>(weight);
}

// Reconstructed from eboot.elf at 0x24BF70.
void PollDepBase::SetThreaded(bool threaded) {
    mFlags = static_cast<unsigned short>((threaded ? kFlagThreaded : 0) | (mFlags & ~kFlagThreaded));
}

// Reconstructed from eboot.elf at 0x24BF90.
void PollDepBase::SetThreadMask(unsigned int mask) {
    const unsigned int workers = static_cast<unsigned int>(PollMgr::sWorkerThreads.size());
    mask &= (1u << workers) - 1;
    mFlags = static_cast<unsigned short>((mFlags & ~kThreadMask) | (mask & kThreadMask));
}

// Reconstructed from eboot.elf at 0x24BFC0.
void PollDepBase::_OnPolled(bool canQueue) {
    _FreeAfters(canQueue);
    mFlags &= ~kFlagPollSkipped;
}

// Reconstructed from eboot.elf at 0x24BFE0. A job standing for a nested
// manager hands its afters to that manager.
void PollDepBase::_FreeAfters(bool canQueue) {
    PollList buckets[kNumPollBuckets];
    const bool queue = canQueue && IsPollEnabled() && (mFlags & kFlagPollSkipped) == 0;
    const unsigned long count = mNumAfters;
    bool queued = false;
    for (unsigned long index = 0; index != count; ++index) {
        queued |= _FreeAfter(mAfters[index], queue, buckets);
    }
    mNumAfters = mAfters.size();
    mFlags ^= kFlagOddFrame;
    if (AsPollMgrJob() != nullptr || AsPollMgrDep() != nullptr) {
        mPollMgr = _NestedPollMgr();
    } else {
        mPollMgr = nullptr;
    }
    if (queued) {
        gEntityThreadState.mPollMgr->QueueForPoll(buckets);
    }
}

// Reconstructed from eboot.elf at 0x24C1F0. The after leaves this frame's
// afters, which are swapped so the released one comes last.
void PollDepBase::EarlyFreeToPoll(PollDepBase* after) {
    if (!PollMgr::sEarlyFreeToPoll) {
        return;
    }
    if (!gEntityThreadState.mEnterImmediately || mNumAfters == 0) {
        return;
    }
    unsigned long index = 0;
    while (mAfters[index] != after) {
        if (++index == mNumAfters) {
            return;
        }
    }
    mAfters[index] = mAfters[mNumAfters - 1];
    mAfters[mNumAfters-- - 1] = after;
    PollList buckets[kNumPollBuckets];
    if (_FreeAfter(after, true, buckets)) {
        gEntityThreadState.mPollMgr->QueueForPoll(buckets);
    }
}

// Reconstructed from eboot.elf at 0x24C420. The after counts this release
// for the next frame at once. A light threaded job goes to the leaf bucket
// when nothing but a nested manager waits for it.
bool PollDepBase::_FreeAfter(PollDepBase* after, bool canQueue, PollList* buckets) {
    const bool nested = after->AsPollMgrJob() != nullptr || after->AsPollMgrDep() != nullptr;
    const unsigned short flags = after->mFlags;
    after->mFlags = static_cast<unsigned short>(flags | (canQueue ? 0 : kFlagPollSkipped));
    ++after->mDepCounts[(flags & kFlagOddFrame) == 0];
    if (after->mDepCounts[(flags & kFlagOddFrame) != 0]-- != 1) {
        return false;
    }
    if (nested) {
        if (after->_NestedPollMgr()->mSharePollMgr) {
            after->mPollMgr = mPollMgr;
        }
        after->mFlags &= ~kFlagPollSkipped;
    } else {
        after->mPollMgr = mPollMgr;
        if (!canQueue || !after->IsPollEnabled()) {
            after->mFlags &= ~kFlagPollSkipped;
            after->_FreeAfters(false);
            return false;
        }
        const unsigned short skipped = after->mFlags & kFlagPollSkipped;
        after->mFlags &= ~kFlagPollSkipped;
        if (skipped != 0) {
            after->_FreeAfters(false);
            return false;
        }
    }
    PollBucket bucket;
    if ((after->mFlags & kFlagThreaded) != 0) {
        if (after->mPollWeight > PollMgr::sMaxPollWeight) {
            bucket = kPollBucketHeavy;
        } else if (
            after->mAfters.empty() ||
            (after->mAfters.size() == 1 && after->mAfters[0]->AsPollMgrJob() != nullptr)) {
            bucket = kPollBucketThreadedLeaf;
        } else {
            bucket = kPollBucketThreaded;
        }
    } else {
        bucket = after->mAfters.empty() ? kPollBucketMainLeaf : kPollBucketMain;
    }
    buckets[bucket].push_back(*after);
    return true;
}

// Reconstructed from eboot.elf at 0x24C5D0.
void PollDepBase::AddPollDep(PollDepBase* dep) {
    _OnAddPollDep();
    dep->mAfters.push_back(this);
    dep->mNumAfters = dep->mAfters.size();
    ++_CurDepCount();
}

// Reconstructed from eboot.elf at 0x24C6E0.
bool PollDepBase::TryRemovePollDep(PollDepBase* dep) {
    _OnRemovePollDep();
    PollDepBase** const end = dep->mAfters.end();
    PollDepBase** found = dep->mAfters.begin();
    while (found != end && *found != this) {
        ++found;
    }
    if (found == end) {
        return false;
    }
    dep->mAfters.erase(found);
    dep->mNumAfters = dep->mAfters.size();
    --_CurDepCount();
    return true;
}

// Reconstructed from eboot.elf at 0x24C770. Only the names of the failure
// report remain.
void PollDepBase::RemovePollDep(PollDepBase* dep) {
    _OnRemovePollDep();
    PollDepBase** const end = dep->mAfters.end();
    PollDepBase** found = dep->mAfters.begin();
    while (found != end && *found != this) {
        ++found;
    }
    if (found == end) {
        static_cast<void>(dep->GetPollName());
        static_cast<void>(GetPollName());
        return;
    }
    dep->mAfters.erase(found);
    dep->mNumAfters = dep->mAfters.size();
    --_CurDepCount();
}

// Reconstructed from eboot.elf at 0x24C810.
bool PollDepBase::HasPollDep(PollDepBase* dep) const {
    for (const PollDepBase* after : dep->mAfters) {
        if (after == this) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x24C840. The binary inlines
// _RemovePostPoll.
void PollDepBase::ClearAllDeps() {
    _OnRemovePollDep();
    for (PollDepBase* after : mAfters) {
        --after->_CurDepCount();
    }
    mAfters.clear();
    mNumAfters = 0;
    mFlags &= ~kFlagPollSkipped;
    mRootNode.Remove();
    _RemovePostPoll();
}

// Reconstructed from eboot.elf at 0x24C970. A job recorded in a queue it
// is not linked in only has its name read for a report that is compiled out.
void PollDepBase::_RemovePostPoll() {
    CritSec* const crit = mPostPollCrit;
    if (crit == nullptr) {
        return;
    }
    if (mPostPollNode.mNext == &mPostPollNode || mPostPollNode.mPrev == &mPostPollNode) {
        static_cast<void>(GetPollName());
        mPostPollCrit = nullptr;
        return;
    }
    scePthreadMutexLock(&crit->mCritSec);
    mPostPollNode.Remove();
    mPostPollCrit = nullptr;
    scePthreadMutexUnlock(&crit->mCritSec);
}

// Reconstructed from eboot.elf at 0x24CA00.
void PollDepBase::_Reset() {
    mAfters.clear();
    mNumAfters = 0;
    mDepCounts[1] = 0;
    mDepCounts[0] = 0;
    mFlags &= ~(kFlagOddFrame | kFlagPollSkipped);
    mPollMgr = nullptr;
    mPostPollCrit = nullptr;
    mRootNode.Remove();
}

// Reconstructed from eboot.elf at 0x24CA60. The queue's lock is held
// without counting the entry.
void PollDepBase::_AddPostPoll() {
    PollMgr* const mgr = mPollMgr;
    scePthreadMutexLock(&mgr->mPostPollCrit.mCritSec);
    mPostPollNode.Remove();
    mgr->mPostPolls.push_back(*this);
    mPostPollCrit = &mgr->mPostPollCrit;
    scePthreadMutexUnlock(&mgr->mPostPollCrit.mCritSec);
}

// Reconstructed from eboot.elf at 0x24CB00.
void PollDepBase::_DoThreadPoll(int thread) {
    ThreadPoll(thread);
}
