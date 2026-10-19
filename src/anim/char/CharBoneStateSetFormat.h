#pragma once

#include <cstddef>

#include "entity/props/PropArray.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BinStream;
class CharBoneChannelMask;

// The kinds of bone channel an animation stores (the map's
// CharBoneChannel::Type). A bone's channels are stored by kind: positions,
// scales and inverse parent scales as three floats, full rotations as a
// quaternion and single-axis rotations in 8 bytes. Enumerator names are not
// in the reference map; they follow CharBoneChannelMask's flags.
struct CharBoneChannel {
    enum Type {
        kPosition = 0,
        kScale = 1,
        kInverseParentScale = 2,
        kRotation = 3,
        kRotationX = 4,
        kRotationY = 5,
        kRotationZ = 6,
        kNumTypes = 7,
    };
};

// The layout of a set of bone channels (anim/CharBoneStateSetFormat.o,
// 0x2E4ED0-0x2E5C8F): the bones of each channel kind, kind after kind, and
// where each kind's data ends. A CharBoneStateBuffer or a keyframe holds the
// channel data in this layout. The object is 88 bytes. Field names are not
// in the reference map.
class CharBoneStateSetFormat {
public:
    // No channels.
    CharBoneStateSetFormat();  // 0x2E4ED0
    // The channels the masks keep, each bone once per kind, in mask order.
    explicit CharBoneStateSetFormat(const PropArray<CharBoneChannelMask>& masks);  // 0x2E4F10
    ~CharBoneStateSetFormat();  // 0x2E5580

    // Reads the layout; false unless the revision is 0. The bone names are
    // stored once each, with an index per channel. The flag is not read.
    bool Load(BinStream& stream, bool cached);  // 0x2E55F0
    // Writes the layout at revision 0.
    void Save(BinStream& stream, bool cached) const;  // 0x2E58D0
    // Adds the name to the list unless it is there.
    void InsertBoneName(eastl::vector<Symbol>& names, const Symbol& name) const;  // 0x2E5470

    // The number of channels of the kinds up to and including each kind.
    unsigned int mBoneEnds[CharBoneChannel::kNumTypes];
    // The byte offset where each kind's data ends; the last is the data's
    // size.
    unsigned int mDataEnds[CharBoneChannel::kNumTypes];
    // The bone of each channel, kind after kind.
    eastl::vector<Symbol> mBoneNames;
};

static_assert(offsetof(CharBoneStateSetFormat, mDataEnds) == 28);
static_assert(offsetof(CharBoneStateSetFormat, mBoneNames) == 56);
static_assert(sizeof(CharBoneStateSetFormat) == 88);
