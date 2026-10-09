#pragma once

#include <cstddef>

// A particle system's particles, stored as one array per attribute. Each
// array is split into chunks of 16 particles, and the live particles form a
// doubly linked list through two of the attributes. Only the counts and the
// arrays that RndParticleBuffer reads are modelled; field and attribute names
// are not in the reference map. The layout follows SetMaxCount (0x6EF190),
// DeleteAllParticles (0x6EF130) and CreateParticle (0x6F0770).
class RndParticleCollection {
public:
    // One attribute's chunk table: an EASTL vector of chunk pointers and
    // the pool its chunks come from. Field names are not in the reference
    // map.
    struct AttributeArray {
        template <typename T>
        const T& Get(unsigned long particle) const {
            return static_cast<const T*>(mChunks[particle >> 4])[particle & 15];
        }

        // The vector's begin, end and capacity end, and its allocator.
        void** mChunks;
        void** mChunksEnd;
        void** mChunksCapacityEnd;
        void* mChunksAllocator;
        // The shared chunk pool; a new chunk is popped from the free list at
        // +16 under the particle mutex (0x6F0AF0).
        void* mChunkPool;
    };

    // Indices into mAttributes. The color is a Hmx::Color; the rest are
    // floats.
    enum Attribute : unsigned long {
        kColor = 1,
        kPositionX = 2,
        kPositionY = 3,
        kPositionZ = 4,
        kVelocityX = 5,
        kVelocityY = 6,
        kVelocityZ = 7,
        kAge = 9,  // The sort key of the age orders.
        kSizeX = 11,
        kSizeY = 12,
        kRotation = 17,
        // Where the quad sits around its position, from 0 to 1 on each
        // axis.
        kPivotX = 20,
        kPivotY = 21,
        // Copied into each vertex's particle data when the buffer asks.
        kData0 = 23,
        kData1 = 24,
        kData2 = 25,
        kNumModelledAttributes = 26,
    };

    const AttributeArray& operator[](Attribute attribute) const {
        return mAttributes[attribute];
    }

    // Set by SetMaxCount, which grows every chunk table to hold it.
    unsigned long mMaxCount;
    unsigned long mNumParticles;
    // Chunks allocated in each attribute array.
    unsigned long mNumChunks;
    // The first and last live particles, or -1 when there are none.
    unsigned long mFirstParticle;
    unsigned long mLastParticle;
    AttributeArray mAttributes[kNumModelledAttributes];
};

static_assert(sizeof(RndParticleCollection::AttributeArray) == 40);
static_assert(offsetof(RndParticleCollection::AttributeArray, mChunkPool) == 32);
static_assert(offsetof(RndParticleCollection, mNumParticles) == 8);
static_assert(offsetof(RndParticleCollection, mLastParticle) == 32);
static_assert(offsetof(RndParticleCollection, mAttributes) == 40);
