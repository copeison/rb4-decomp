#pragma once

#include <cstddef>

#include "entity/props/PropMetadata.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BinStream;

// Which channels of one bone an animation drives (anim/CharBoneChannelMask.o,
// 0x2E14E0-0x2E1AEF): the position, a rotation, the scale and the inverse
// parent scale, for the bone with the name. CharBoneMaskCom keeps one per
// skeleton bone as "bone_channel_masks". The object is 16 bytes, without a
// vtable. Field names follow the registry's ("use_position", "rotation",
// "use_scale", "use_inverse_parent_scale", "name").
class CharBoneChannelMask {
public:
    // The rotation a mask keeps, the "rotation" property's allowed values.
    // The enumerator names follow their labels and are not in the reference
    // map.
    enum Rotation : int {
        // "This bone contributes no rotation information."
        kRotationNone = 0,
        // "All of this bone's rotation information is used (i.e. XYZ)."
        kRotationFull = 1,
        // "This bone's X rotation is used." and likewise for Y and Z.
        kRotationX = 2,
        kRotationY = 3,
        kRotationZ = 4,
    };

    // The channel bits GetBitfield returns. Names not in the reference
    // map.
    enum Channel : unsigned int {
        kChannelPosition = 1 << 0,
        kChannelScale = 1 << 1,
        kChannelInverseParentScale = 1 << 2,
        kChannelRotationFull = 1 << 3,
        kChannelRotationX = 1 << 4,
        kChannelRotationY = 1 << 5,
        kChannelRotationZ = 1 << 6,
    };

    // Masks every channel.
    CharBoneChannelMask();  // 0x2E14E0
    ~CharBoneChannelMask();  // 0x2E1510

    // Keeps the position, the full rotation, the scale and the inverse
    // parent scale ("SRT IPS").
    void SetSRTIPS();  // 0x2E1520
    // Masks every channel.
    void SetDisable();  // 0x2E1540
    // The kept channels as Channel bits.
    unsigned int GetBitfield() const;  // 0x2E1560
    // Whether the masks differ in their name or a channel. Its caller is
    // at 0x2F0163. Name not in the reference map.
    bool operator!=(const CharBoneChannelMask& other) const;  // 0x2E15F0

    // The labels of Rotation for the "rotation" property, in the order
    // None, X, Y, Z, Full. CharBoneMaskCom::_Init and the registration at
    // 0x2E6360 use it. Name not in the reference map.
    static eastl::vector<AllowedValue<int>> GetRotationAllowedValues();  // 0x2E1620

    bool mUsePosition;
    bool mUseScale;
    bool mUseInverseParentScale;
    Rotation mRotation;
    // The bone's name.
    Symbol mName;
};

static_assert(offsetof(CharBoneChannelMask, mUseScale) == 1);
static_assert(offsetof(CharBoneChannelMask, mUseInverseParentScale) == 2);
static_assert(offsetof(CharBoneChannelMask, mRotation) == 4);
static_assert(offsetof(CharBoneChannelMask, mName) == 8);
static_assert(sizeof(CharBoneChannelMask) == 16);

// Write and read a mask: a revision (0), the name, the three flags as
// bytes and the rotation. The callers are at 0x2F032A and 0x2F055D. Names
// not in the reference map.
BinStream& operator<<(BinStream& stream, const CharBoneChannelMask& mask);  // 0x2E1970
BinStream& operator>>(BinStream& stream, CharBoneChannelMask& mask);        // 0x2E1A30
