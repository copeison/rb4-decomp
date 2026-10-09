#pragma once

#include <cstddef>

#include "os/threading/CritSec.h"

// A free list of fixed-size particle chunks carved out of one allocation
// ("RndParticleChunkPool"). Each free chunk stores the next free chunk in
// its first word. The pops and pushes lock sParticleSystemCritSec's mutex
// directly. Class and field names are not in the reference map; the class
// name follows the allocation label.
struct RndParticleChunkPool {
    // Pops a chunk; the caller has checked that one is free.
    void* Alloc();
    void Free(void* chunk);

    unsigned long mNumChunks;
    void* mStorage;
    void* mFreeList;
};

static_assert(sizeof(RndParticleChunkPool) == 24);

// The particle system manager (render/RndParticleSystemMgr.o, constructor
// 0x628ED0, created by 0x628E60 into theParticleSystemMgr). It sizes the
// chunk pools from the "rnd particlesys global_limit" config value and
// counts the particles and chunks in use. Only the layout that
// RndParticleCollection uses is modelled; field names are not in the
// reference map.
class RndParticleSystemMgr {
public:
    // The configured global particle limit and the same rounded up to whole
    // chunks of 16.
    unsigned long mGlobalLimit;
    unsigned long mChunkedLimit;
    // Live particles and chunk sets (one chunk per attribute) over all
    // collections, with their peaks; updated under mCritSec's mutex.
    unsigned long mNumParticles;
    unsigned long mPeakParticles;
    unsigned long mNumChunks;
    unsigned long mPeakChunks;
    CritSec mCritSec;
    // One pool per attribute element type: int and float chunks are 64
    // bytes, unsigned long and WaveformEvalData chunks 128 and Hmx::Color
    // chunks 256. mUnusedPool is built with no chunks.
    RndParticleChunkPool mIntPool;
    RndParticleChunkPool mFloatPool;
    RndParticleChunkPool mULongPool;
    RndParticleChunkPool mUnusedPool;
    RndParticleChunkPool mColorPool;
    RndParticleChunkPool mEvalDataPool;
    // The eastl::set<RndParticleSystem*> of registered systems; not
    // modelled.
    unsigned char mSystems[56];
};

static_assert(offsetof(RndParticleSystemMgr, mNumParticles) == 16);
static_assert(offsetof(RndParticleSystemMgr, mPeakChunks) == 40);
static_assert(offsetof(RndParticleSystemMgr, mCritSec) == 48);
static_assert(offsetof(RndParticleSystemMgr, mIntPool) == 64);
static_assert(offsetof(RndParticleSystemMgr, mFloatPool) == 88);
static_assert(offsetof(RndParticleSystemMgr, mULongPool) == 112);
static_assert(offsetof(RndParticleSystemMgr, mColorPool) == 160);
static_assert(offsetof(RndParticleSystemMgr, mEvalDataPool) == 184);
static_assert(sizeof(RndParticleSystemMgr) == 264);

extern CritSec sParticleSystemCritSec;              // 0x1AAA460
extern RndParticleSystemMgr* theParticleSystemMgr;  // 0x1AAA470

// Inlined into every RndParticleAttr chunk allocation (0x6F0AF0).
inline void* RndParticleChunkPool::Alloc() {
    scePthreadMutexLock(&sParticleSystemCritSec.mCritSec);
    void* chunk = mFreeList;
    mFreeList = *static_cast<void**>(chunk);
    scePthreadMutexUnlock(&sParticleSystemCritSec.mCritSec);
    return chunk;
}

// Inlined into RndParticleAttr's destructor (0x6EDC80) and the chunk frees.
inline void RndParticleChunkPool::Free(void* chunk) {
    scePthreadMutexLock(&sParticleSystemCritSec.mCritSec);
    *static_cast<void**>(chunk) = mFreeList;
    mFreeList = chunk;
    scePthreadMutexUnlock(&sParticleSystemCritSec.mCritSec);
}
