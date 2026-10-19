#pragma once

#include <cstddef>

#include "anim/char/CharBoneStateSetFormat.h"

// The blended channel data of a pose being built (anim/CharBoneStateBuffer.o,
// 0x3362C0-0x3368F4): a copy of the format, the channel data in its layout
// and one weight per channel after the data. Animations add their weighted
// channels into the buffer; NormalizeChannelData divides by the weights.
// The object is 104 bytes. Field names are not in the reference map.
class CharBoneStateBuffer {
public:
    CharBoneStateBuffer();   // 0x3362C0
    ~CharBoneStateBuffer();  // 0x3362E0

    // Takes the format and allocates zeroed data and weights for it.
    void Init(const CharBoneStateSetFormat& format);  // 0x336310
    // Zeroes the data and the weights.
    void ClearChannelData() const;  // 0x336490
    // Divides each weighted channel by its weight and sets the weight to 1;
    // rotations are normalized. With `additive` the positions keep their
    // sums, and the scales and rotations below full weight are completed
    // with the identity for the missing weight. The map's parameter name is
    // not known; the evidence for `additive` is weak.
    void NormalizeChannelData(bool additive) const;  // 0x3364B0

    CharBoneStateSetFormat mFormat;
    // The channel data, followed by the weights.
    unsigned char* mData;
    // One weight per channel, inside mData's allocation.
    float* mWeights;
};

static_assert(offsetof(CharBoneStateBuffer, mData) == 88);
static_assert(offsetof(CharBoneStateBuffer, mWeights) == 96);
static_assert(sizeof(CharBoneStateBuffer) == 104);
