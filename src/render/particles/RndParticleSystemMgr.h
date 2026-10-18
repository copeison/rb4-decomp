#pragma once

#include <cstddef>
#include <cstring>

#include "os/memory/MemMgr.h"
#include "os/threading/CritSec.h"
#include "utl/containers/Set.h"

class RndParticleSystem;

// A free list of fixed-size particle chunks carved out of one allocation
// ("RndParticleChunkPool"). Each free chunk stores the next free chunk in
// its first word. The pops and pushes lock sParticleSystemCritSec's mutex
// directly. Class and field names are not in the reference map; the class
// name follows the allocation label.
struct RndParticleChunkPool {
    // Carves `numChunks` zeroed chunks of `chunkSize` bytes out of one
    // allocation and links them in order. Inlined into the manager's
    // constructor (0x628ED0).
    RndParticleChunkPool(unsigned long numChunks, unsigned long chunkSize);
    // Inlined into the manager's destructor (0x6292E0).
    ~RndParticleChunkPool() {
        MemFree(mStorage);
    }

    RndParticleChunkPool(const RndParticleChunkPool&) = delete;
    RndParticleChunkPool& operator=(const RndParticleChunkPool&) = delete;

    // Pops a chunk; the caller has checked that one is free.
    void* Alloc();
    void Free(void* chunk);

    unsigned long mNumChunks;
    void* mStorage;
    void* mFreeList;
};

static_assert(sizeof(RndParticleChunkPool) == 24);

// The particle system manager (render/RndParticleSystemMgr.o, 0x628E60-
// 0x62949A; the static initializer at 0x6293F0). Init creates the one
// instance, theParticleSystemMgr. It sizes the chunk pools from the "rnd
// particlesys global_limit" config value and counts the particles and
// chunks in use. Field names are not in the reference map.
class RndParticleSystemMgr {
public:
    RndParticleSystemMgr();   // 0x628ED0
    ~RndParticleSystemMgr();  // 0x6292E0

    // Creates and destroys theParticleSystemMgr.
    static void Init();       // 0x628E60
    static void Terminate();  // 0x628E90

    // The map also lists CollectIntersectingParticleSystems(Sphere const&,
    // eastl::vector<RndParticleSystem*>&), an empty function this build
    // does not contain.

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
    // One pool per attribute element type. For every 16 particles of the
    // limit the pools hold 1 int and 18 float chunks of 64 bytes, 2
    // unsigned long and 11 WaveformEvalData chunks of 128 bytes and 1
    // Hmx::Color chunk of 256 bytes. mUnusedPool is built with no chunks.
    RndParticleChunkPool mIntPool;
    RndParticleChunkPool mFloatPool;
    RndParticleChunkPool mULongPool;
    RndParticleChunkPool mUnusedPool;
    RndParticleChunkPool mColorPool;
    RndParticleChunkPool mEvalDataPool;
    // The registered particle systems. Nothing in this build adds to it.
    eastl::set<RndParticleSystem*> mSystems;
};

static_assert(offsetof(RndParticleSystemMgr, mNumParticles) == 16);
static_assert(offsetof(RndParticleSystemMgr, mPeakChunks) == 40);
static_assert(offsetof(RndParticleSystemMgr, mCritSec) == 48);
static_assert(offsetof(RndParticleSystemMgr, mIntPool) == 64);
static_assert(offsetof(RndParticleSystemMgr, mFloatPool) == 88);
static_assert(offsetof(RndParticleSystemMgr, mULongPool) == 112);
static_assert(offsetof(RndParticleSystemMgr, mColorPool) == 160);
static_assert(offsetof(RndParticleSystemMgr, mEvalDataPool) == 184);
static_assert(offsetof(RndParticleSystemMgr, mSystems) == 208);
static_assert(sizeof(RndParticleSystemMgr) == 264);

extern CritSec sParticleSystemCritSec;              // 0x1AAA460
extern RndParticleSystemMgr* theParticleSystemMgr;  // 0x1AAA470

inline RndParticleChunkPool::RndParticleChunkPool(
    unsigned long numChunks,
    unsigned long chunkSize)
    : mNumChunks(numChunks) {
    const unsigned long bytes = numChunks * chunkSize;
    mStorage = MemAlloc(bytes, "RndParticleChunkPool", 16);
    memset(mStorage, 0, bytes);
    char* chunk = static_cast<char*>(mStorage);
    for (unsigned long left = bytes; left != 0; left -= chunkSize) {
        *reinterpret_cast<void**>(chunk) =
            left != chunkSize ? chunk + chunkSize : nullptr;
        chunk += chunkSize;
    }
    mFreeList = mStorage;
}

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
