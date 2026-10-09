#pragma once

#include <cstddef>

// A particle system's particles, stored as one array per attribute. Each
// array is split into chunks of 16 particles. Only the count and the arrays
// that RndParticleBuffer reads are modelled; field and attribute names are
// not in the reference map.
class RndParticleCollection {
public:
    // One attribute's chunk table.
    struct AttributeArray {
        template <typename T>
        const T& Get(unsigned long particle) const {
            return static_cast<const T*>(mChunks[particle >> 4])[particle & 15];
        }

        void** mChunks;
        unsigned char mUnknown8[32];
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

    unsigned long mUnknown0;
    unsigned long mNumParticles;
    unsigned char mUnknown16[24];
    AttributeArray mAttributes[kNumModelledAttributes];
};

static_assert(sizeof(RndParticleCollection::AttributeArray) == 40);
static_assert(offsetof(RndParticleCollection, mNumParticles) == 8);
static_assert(offsetof(RndParticleCollection, mAttributes) == 40);
