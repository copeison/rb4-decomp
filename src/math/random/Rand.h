#pragma once

#include <cstddef>

// 64-bit xorshift generator (math/Rand.o). The draws are inline in the
// binary; only the out-of-line members are in the map.
class Rand {
public:
    Rand();                            // 0x2159A0
    explicit Rand(unsigned long seed);  // 0x2159B0
    // A zero seed selects the default one. At 0x2159D0.
    void Seed(unsigned long seed);

    static void Init();       // 0x215980, empty
    static void Terminate();  // 0x215990, empty

    // Advances the state with the 13, 7, 17 xorshift. Inlined, for example
    // into FusionSampler::_TryKeyOnZone at 0x982D0. Name not in the
    // reference map.
    unsigned long Int() {
        mState ^= mState << 13;
        mState ^= mState >> 7;
        mState ^= mState << 17;
        return mState;
    }
    // A float in [0, 1) from the low 23 bits of the next state. Inlined
    // with Int. Name not in the reference map.
    float Float() {
        union {
            unsigned int mBits;
            float mValue;
        } value;
        value.mBits = (static_cast<unsigned int>(Int()) & 0x7FFFFF) | 0x3F800000;
        return value.mValue + -1.0f;
    }

    // The seed used for zero. Name not in the reference map.
    static constexpr unsigned long kDefaultSeed = 0x139408DCBBF7A44UL;

    unsigned long mState;  // Name not in the reference map.
};

static_assert(sizeof(Rand) == 8);

// The shared generator, seeded from the clock by core_initialize.
extern Rand gRand;  // 0x19E6628
