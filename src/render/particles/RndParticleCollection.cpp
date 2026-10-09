#include "render/particles/RndParticleCollection.h"

namespace {

// The -1, 8, 4 triple of a shared header that every object's static
// initializer sets (here at 0x6F2900; see RndMeshCom.cpp). Unused here.
// Names not in the reference map.
[[maybe_unused]] unsigned int gNullObjectId = 0xFFFFFFFFU;  // 0x1AB1F5C
[[maybe_unused]] int gMeshGroupSize2D = 8;                // 0x1AB1F60
[[maybe_unused]] int gMeshGroupSize3D = 4;                // 0x1AB1F64

// Adds to theParticleSystemMgr's particle count and raises its peak, under
// its mutex. Inlined. Name not in the reference map.
void AddMgrParticles(unsigned long count) {
    RndParticleSystemMgr* mgr = theParticleSystemMgr;
    scePthreadMutexLock(&mgr->mCritSec.mCritSec);
    mgr->mNumParticles += count;
    if (mgr->mPeakParticles < mgr->mNumParticles) {
        mgr->mPeakParticles = mgr->mNumParticles;
    }
    scePthreadMutexUnlock(&mgr->mCritSec.mCritSec);
}

// Removes from theParticleSystemMgr's particle count. Inlined. Name not in
// the reference map.
void RemoveMgrParticles(unsigned long count) {
    RndParticleSystemMgr* mgr = theParticleSystemMgr;
    scePthreadMutexLock(&mgr->mCritSec.mCritSec);
    mgr->mNumParticles -= count;
    scePthreadMutexUnlock(&mgr->mCritSec.mCritSec);
}

}  // namespace

std::atomic<int> RndParticleCollection::gNextParticleID(0);

template <typename Op>
void RndParticleCollection::_ForEachAttr(Op op) {
    op(mIds);
    op(mColors);
    op(mPosX);
    op(mPosY);
    op(mPosZ);
    op(mVelX);
    op(mVelY);
    op(mVelZ);
    op(mDeathTimes);
    op(mBirthTimes);
    op(mLifeFractions);
    op(mSizeX);
    op(mSizeY);
    op(mSizeZ);
    op(mInitialSizes);
    op(mRotationX);
    op(mRotationY);
    op(mRotationZ);
    op(mPrev);
    op(mNext);
    op(mPivotX);
    op(mPivotY);
    op(mPivotZ);
    op(mExtraData0);
    op(mExtraData1);
    op(mExtraData2);
    for (RndParticleAttr<WaveformEvalData>& evalData : mEvalData) {
        op(evalData);
    }
}

// Reconstructed from eboot.elf at 0x6ED3D0.
RndParticleCollection::RndParticleCollection()
    : mMaxCount(0),
      mNumParticles(0),
      mNumChunks(0),
      mFirstParticle(-1UL),
      mLastParticle(-1UL),
      mIds(theParticleSystemMgr->mIntPool),
      mColors(theParticleSystemMgr->mColorPool),
      mPosX(theParticleSystemMgr->mFloatPool),
      mPosY(theParticleSystemMgr->mFloatPool),
      mPosZ(theParticleSystemMgr->mFloatPool),
      mVelX(theParticleSystemMgr->mFloatPool),
      mVelY(theParticleSystemMgr->mFloatPool),
      mVelZ(theParticleSystemMgr->mFloatPool),
      mDeathTimes(theParticleSystemMgr->mFloatPool),
      mBirthTimes(theParticleSystemMgr->mFloatPool),
      mLifeFractions(theParticleSystemMgr->mFloatPool),
      mSizeX(theParticleSystemMgr->mFloatPool),
      mSizeY(theParticleSystemMgr->mFloatPool),
      mSizeZ(theParticleSystemMgr->mFloatPool),
      mInitialSizes(theParticleSystemMgr->mFloatPool),
      mRotationX(theParticleSystemMgr->mFloatPool),
      mRotationY(theParticleSystemMgr->mFloatPool),
      mRotationZ(theParticleSystemMgr->mFloatPool),
      mPrev(theParticleSystemMgr->mULongPool),
      mNext(theParticleSystemMgr->mULongPool),
      mPivotX(theParticleSystemMgr->mFloatPool),
      mPivotY(theParticleSystemMgr->mFloatPool),
      mPivotZ(theParticleSystemMgr->mFloatPool),
      mExtraData0(theParticleSystemMgr->mFloatPool),
      mExtraData1(theParticleSystemMgr->mFloatPool),
      mExtraData2(theParticleSystemMgr->mFloatPool),
      mEvalData{
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
          RndParticleAttr<WaveformEvalData>(theParticleSystemMgr->mEvalDataPool),
      } {}

// Reconstructed from eboot.elf at 0x6EDC80.
// The attributes' destructors then return any chunks left to the pools.
RndParticleCollection::~RndParticleCollection() {
    DeleteAllParticles();
}

// Reconstructed from eboot.elf at 0x6EF130.
void RndParticleCollection::DeleteAllParticles() {
    _FreeAllChunks();
    RemoveMgrParticles(mNumParticles);
    mNumParticles = 0;
    mFirstParticle = -1UL;
    mLastParticle = -1UL;
}

// Reconstructed from eboot.elf at 0x6EF190.
void RndParticleCollection::SetMaxCount(unsigned long count) {
    mMaxCount = count;
    const unsigned long chunks = (count >> 4) + 1;
    _ForEachAttr([chunks](auto& attr) { attr.mChunks.reserve(chunks); });
}

// Reconstructed from eboot.elf at 0x6F0770.
// A new chunk set is only taken when the int pool has a free chunk; the
// other pools are assumed to have one too.
unsigned long RndParticleCollection::CreateParticle() {
    const unsigned long particle = mNumParticles;
    if ((particle >> 4) >= mNumChunks) {
        ScopedCritSec lock(sParticleSystemCritSec);
        if (theParticleSystemMgr->mIntPool.mFreeList == nullptr) {
            return -1UL;
        }
        _ForEachAttr([](auto& attr) { attr.AllocChunk(); });
        RndParticleSystemMgr* mgr = theParticleSystemMgr;
        scePthreadMutexLock(&mgr->mCritSec.mCritSec);
        ++mgr->mNumChunks;
        if (mgr->mPeakChunks < mgr->mNumChunks) {
            mgr->mPeakChunks = mgr->mNumChunks;
        }
        scePthreadMutexUnlock(&mgr->mCritSec.mCritSec);
        ++mNumChunks;
    }

    if (mLastParticle != -1UL) {
        mNext[mLastParticle] = particle;
    }
    mNext[particle] = -1UL;
    mPrev[particle] = mLastParticle;
    mLastParticle = particle;
    if (mFirstParticle == -1UL) {
        mFirstParticle = particle;
    }
    mIds[particle] = ++gNextParticleID;
    ++mNumParticles;
    AddMgrParticles(1);
    return particle;
}

// Reconstructed from eboot.elf at 0x6F0FA0.
void RndParticleCollection::DeleteParticle(unsigned long particle) {
    if (mFirstParticle == particle) {
        mFirstParticle = mNext[particle];
    }
    if (mLastParticle == particle) {
        mLastParticle = mPrev[particle];
    }
    if (mPrev[particle] != -1UL) {
        mNext[mPrev[particle]] = mNext[particle];
    }
    if (mNext[particle] != -1UL) {
        mPrev[mNext[particle]] = mPrev[particle];
    }

    // Move the last particle into the hole, relinking its neighbours.
    const unsigned long last = mNumParticles - 1;
    if (last != particle) {
        if (mPrev[last] != -1UL) {
            mNext[mPrev[last]] = particle;
        }
        if (mNext[last] != -1UL) {
            mPrev[mNext[last]] = particle;
        }
        _CopyParticle(last, particle);
        mNext[last] = -1UL;
        mPrev[last] = -1UL;
        if (mFirstParticle == last) {
            mFirstParticle = particle;
        }
        if (mLastParticle == last) {
            mLastParticle = particle;
        }
    }
    --mNumParticles;
    RemoveMgrParticles(1);
}

// Reconstructed from eboot.elf at 0x6F1170.
void RndParticleCollection::_CopyParticle(unsigned long from, unsigned long to) {
    _ForEachAttr([from, to](auto& attr) { attr[to] = attr[from]; });
}

// Reconstructed from eboot.elf at 0x6F1550.
void RndParticleCollection::_FreeAllChunks() {
    ScopedCritSec lock(sParticleSystemCritSec);
    RndParticleSystemMgr* mgr = theParticleSystemMgr;
    scePthreadMutexLock(&mgr->mCritSec.mCritSec);
    mgr->mNumChunks -= mNumChunks;
    scePthreadMutexUnlock(&mgr->mCritSec.mCritSec);
    for (; mNumChunks != 0; --mNumChunks) {
        _ForEachAttr([](auto& attr) { attr.FreeChunk(); });
    }
}

// Reconstructed from eboot.elf at 0x6F1F80.
// Keeps the chunk of the last live particle and frees the ones past it.
void RndParticleCollection::FreeUnusedChunks() {
    ScopedCritSec lock(sParticleSystemCritSec);
    if (mNumParticles == 0) {
        _FreeAllChunks();
        return;
    }
    const unsigned long lastChunk = (mNumParticles - 1) >> 4;
    while (lastChunk < mNumChunks - 1) {
        _ForEachAttr([](auto& attr) { attr.FreeChunk(); });
        --mNumChunks;
        RndParticleSystemMgr* mgr = theParticleSystemMgr;
        scePthreadMutexLock(&mgr->mCritSec.mCritSec);
        --mgr->mNumChunks;
        scePthreadMutexUnlock(&mgr->mCritSec.mCritSec);
    }
}
