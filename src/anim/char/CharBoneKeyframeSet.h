#pragma once

#include <cstddef>

#include "anim/char/CharBoneStateSetFormat.h"
#include "anim/core/AnimResource.h"
#include "math/transform/QuatXfm.h"

class BinStream;
class CharAnimBuffer;

// One set of channel data in a format (the map's CharBoneStateSet): what
// CharAnimBuffer::Apply and ScaleDown read. Field names are not in the
// reference map. The object is 16 bytes.
struct CharBoneStateSet {
    const CharBoneStateSetFormat* mFormat;
    const unsigned char* mData;
};

static_assert(sizeof(CharBoneStateSet) == 16);

// The keyframes of a character animation (anim/CharBoneKeyframeSet.o,
// 0x334DB0-0x3361CF): the channel format, one block of channel data per
// frame and, optionally, the root bones' transform at each frame for root
// motion. CharAnim holds one. The object is 112 bytes. Field names are not
// in the reference map.
class CharBoneKeyframeSet {
public:
    // A position between two keyframes: the first and how far towards the
    // next (the map's CharBoneKeyframeSet::FramePos).
    struct FramePos {
        unsigned int mFrame;
        float mFraction;
    };

    CharBoneKeyframeSet();   // 0x334DB0
    ~CharBoneKeyframeSet();  // 0x334DE0

    // Takes the format and the data, which the set then owns.
    void Init(
        const CharBoneStateSetFormat& format,
        const unsigned char* keyframes,
        const QuatXfm* rootXfms,
        unsigned long numKeyframes,
        bool relative);  // 0x334E30
    // Reads the set at revision 2; false for any other revision or a bad
    // format.
    bool Load(BinStream& stream, bool cached);  // 0x334F70
    void Save(BinStream& stream, bool cached) const;  // 0x3350E0
    // Adds the frame's channels to the buffer with the weight, sampled
    // between the keyframes around it, and the root motion over the
    // frame delta (through the loop's end when `wrapped`); a layered
    // animation first scales down what is in the buffer. The map's
    // signature is Apply(float, float, float, AnimResource::BlendMode,
    // CharAnimBuffer&) const; this build adds `wrapped`.
    void Apply(
        float weight,
        float frame,
        float delta,
        bool wrapped,
        AnimResource::BlendMode mode,
        CharAnimBuffer& buffer) const;  // 0x335220
    // Scales the buffer's channels that the set drives down by the weight.
    void ScaleDown(float weight, CharAnimBuffer& buffer) const;  // 0x336180
    // The root transform at the position, blended between the keyframes.
    void GetRootXfm(const FramePos& pos, QuatXfm& xfm) const;  // 0x3361D0

    CharBoneStateSetFormat mFormat;
    unsigned int mNumKeyframes;
    // Whether the channels are relative to another pose: CharAnimBuffer
    // applies them additively. Weakly supported.
    bool mRelative;
    // mNumKeyframes blocks of the format's data size.
    const unsigned char* mKeyframes;
    // The root bones' transform at each keyframe, or null.
    const QuatXfm* mRootXfms;
};

static_assert(offsetof(CharBoneKeyframeSet, mNumKeyframes) == 88);
static_assert(offsetof(CharBoneKeyframeSet, mRelative) == 92);
static_assert(offsetof(CharBoneKeyframeSet, mKeyframes) == 96);
static_assert(offsetof(CharBoneKeyframeSet, mRootXfms) == 104);
static_assert(sizeof(CharBoneKeyframeSet) == 112);
