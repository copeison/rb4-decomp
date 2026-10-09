#pragma once

#include <atomic>
#include <cstddef>

#include "math/color/Color.h"
#include "render/particles/RndParticleSystemMgr.h"
#include "utl/containers/Vector.h"

// A particle's evaluation state for one of the system's waveforms
// (entity/Waveform.o, not modelled): 8 bytes, copied whole.
struct WaveformEvalData {
    unsigned long mData;  // Name not in the reference map.
};

static_assert(sizeof(WaveformEvalData) == 8);

// One per-particle attribute: a vector of 16-element chunks taken from one
// of theParticleSystemMgr's pools. Only the destructor is in the reference
// map (one copy per element type); the other members are inlined and their
// names are not in the map.
template <typename T>
class RndParticleAttr {
public:
    explicit RndParticleAttr(RndParticleChunkPool& pool) : mPool(&pool) {}
    RndParticleAttr(const RndParticleAttr&) = delete;
    RndParticleAttr& operator=(const RndParticleAttr&) = delete;
    // Returns every chunk to the pool (0x1039570 and siblings in the
    // reference map; inlined into ~RndParticleCollection here).
    ~RndParticleAttr() {
        for (T* chunk : mChunks) {
            mPool->Free(chunk);
        }
    }

    T& operator[](unsigned long particle) {
        return mChunks[particle >> 4][particle & 15];
    }
    const T& operator[](unsigned long particle) const {
        return mChunks[particle >> 4][particle & 15];
    }

    // Appends a chunk from the pool (0x6F0AF0 for int).
    void AllocChunk() {
        mChunks.push_back(static_cast<T*>(mPool->Alloc()));
    }
    // Returns the last chunk to the pool.
    void FreeChunk() {
        mPool->Free(mChunks.back());
        mChunks.pop_back();
    }

    eastl::vector<T*> mChunks;
    RndParticleChunkPool* mPool;
};

static_assert(sizeof(RndParticleAttr<float>) == 40);
static_assert(offsetof(RndParticleAttr<float>, mPool) == 32);

// A particle system's particles (render/RndParticleCollection.o, 0x6ED3D0 to
// 0x6F292A), stored as one RndParticleAttr per attribute, in chunks of 16.
// The live particles form a doubly linked list through mPrev and mNext.
// Attribute names are not in the reference map; they follow the readers in
// RndParticleCom (_UpdateParticles, _UpdateSpin, _UpdatePivot,
// _UpdateColorSize) and RndParticleBuffer.
class RndParticleCollection {
public:
    RndParticleCollection();   // 0x6ED3D0
    ~RndParticleCollection();  // 0x6EDC80

    // Deletes every particle and frees the chunks.
    void DeleteAllParticles();  // 0x6EF130
    // Reserves every chunk table for `count` particles.
    void SetMaxCount(unsigned long count);  // 0x6EF190
    // Appends a particle at the end of the arrays and the live list and
    // returns its index, or -1 when no chunk is free.
    unsigned long CreateParticle();  // 0x6F0770
    // Unlinks the particle and moves the last particle into its slot.
    void DeleteParticle(unsigned long particle);  // 0x6F0FA0
    // Returns the chunks past the live particles to the pools.
    void FreeUnusedChunks();  // 0x6F1F80

    // The source of each particle's id.
    static std::atomic<int> gNextParticleID;  // 0x1AB1F68

private:
    // Copies every attribute of particle `from` to particle `to`.
    void _CopyParticle(unsigned long from, unsigned long to);  // 0x6F1170
    // Returns every chunk to the pools.
    void _FreeAllChunks();  // 0x6F1550
    // Applies `op` to every attribute, in declaration order. Name not in the
    // reference map; the binary unrolls each loop.
    template <typename Op>
    void _ForEachAttr(Op op);

public:
    // Set by SetMaxCount.
    unsigned long mMaxCount;
    unsigned long mNumParticles;
    // Chunks held by each attribute.
    unsigned long mNumChunks;
    // The first and last live particles, or -1 when there are none.
    unsigned long mFirstParticle;
    unsigned long mLastParticle;

    // Attribute 0, from gNextParticleID.
    RndParticleAttr<int> mIds;
    RndParticleAttr<Hmx::Color> mColors;
    RndParticleAttr<float> mPosX;
    RndParticleAttr<float> mPosY;
    RndParticleAttr<float> mPosZ;
    RndParticleAttr<float> mVelX;
    RndParticleAttr<float> mVelY;
    RndParticleAttr<float> mVelZ;
    // The time the particle dies at, on the system's clock.
    RndParticleAttr<float> mDeathTimes;
    // The time the particle was born at; the sort key of the age orders.
    RndParticleAttr<float> mBirthTimes;
    // How far through its life the particle is, from 0 to 1.
    RndParticleAttr<float> mLifeFractions;
    RndParticleAttr<float> mSizeX;
    RndParticleAttr<float> mSizeY;
    RndParticleAttr<float> mSizeZ;
    // Read with the sizes by _UpdateColorSize; weak evidence for the name.
    RndParticleAttr<float> mInitialSizes;
    // Spin angles; sprites turn by mRotationZ.
    RndParticleAttr<float> mRotationX;
    RndParticleAttr<float> mRotationY;
    RndParticleAttr<float> mRotationZ;
    // The live list's links, or -1 at its ends.
    RndParticleAttr<unsigned long> mPrev;
    RndParticleAttr<unsigned long> mNext;
    // Where the quad sits around its position, from 0 to 1 on each axis.
    RndParticleAttr<float> mPivotX;
    RndParticleAttr<float> mPivotY;
    RndParticleAttr<float> mPivotZ;
    // The extra_data_0-2 values copied into each vertex.
    RndParticleAttr<float> mExtraData0;
    RndParticleAttr<float> mExtraData1;
    RndParticleAttr<float> mExtraData2;
    // Each particle's state for the per-particle waveforms.
    RndParticleAttr<WaveformEvalData> mEvalData[15];
};

static_assert(offsetof(RndParticleCollection, mNumParticles) == 8);
static_assert(offsetof(RndParticleCollection, mLastParticle) == 32);
static_assert(offsetof(RndParticleCollection, mIds) == 40);
static_assert(offsetof(RndParticleCollection, mColors) == 80);
static_assert(offsetof(RndParticleCollection, mInitialSizes) == 600);
static_assert(offsetof(RndParticleCollection, mRotationZ) == 720);
static_assert(offsetof(RndParticleCollection, mPrev) == 760);
static_assert(offsetof(RndParticleCollection, mNext) == 800);
static_assert(offsetof(RndParticleCollection, mPivotX) == 840);
static_assert(offsetof(RndParticleCollection, mExtraData0) == 960);
static_assert(offsetof(RndParticleCollection, mEvalData) == 1080);
static_assert(sizeof(RndParticleCollection) == 1680);
