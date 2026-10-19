#pragma once

#include <cstddef>

#include "anim/char/CharBoneKeyframeSet.h"
#include "anim/char/CharBoneStateBuffer.h"
#include "entity/resources/Resource.h"
#include "math/transform/QuatXfm.h"
#include "math/transform/Transform.h"

class CharSkeleton;

// The buffer a CharAnimDriver blends its playbacks into each poll
// (anim/CharAnimBuffer.o, 0x331A50-0x3330FF): the channels of the absolute
// animations, those of the relative ones, and the root motion of the frame.
// EndFrame normalizes the channels and adds the relative ones onto the
// absolute ones. The object is 256 bytes. Field names are not in the
// reference map.
class CharAnimBuffer {
public:
    CharAnimBuffer();   // 0x331A50
    ~CharAnimBuffer();  // 0x331AB0

    // Lays both channel buffers out in the format, for the skeleton. The
    // map's signature is Init(CharBoneStateSetFormat const&, CharSkeleton
    // const*); this build adds the skeleton's file.
    void Init(
        const CharBoneStateSetFormat& format,
        const CharSkeleton* skeleton,
        ResourcePath skeletonPath);  // 0x331AD0
    // Clears the channels and the root motion for a new frame.
    void BeginFrame();  // 0x331B10
    // Normalizes both channel buffers and the root motion by their weights
    // and adds the relative channels onto the absolute ones (the map's
    // _ComposeBuffers is folded in).
    void EndFrame();  // 0x331B70
    // The root motion of the frame as a transform.
    void GetDeltaRootXfm(Transform& xfm);  // 0x331B40
    // The root bone's position and rotation in the absolute channels.
    void GrabBone(unsigned long bone, QuatXfm& xfm) const;  // 0x332340
    // Copies the root bones' positions and rotations out of the absolute
    // channels and zeroes them there; `xfms` has one entry per root.
    void GrabAndClearRootBones(QuatXfm* xfms);  // 0x332380
    // Writes the root bones' positions and rotations back.
    void SetRootBones(QuatXfm* xfms);  // 0x332460
    // Adds the set's channels with the weight into the absolute channels,
    // or into the relative ones for a relative set.
    void Apply(const CharBoneStateSet& set, float weight, bool relative);  // 0x332510
    // Scales the channels the set drives so that their weights become
    // `weight`; a layered animation passes 1 minus its weight to fade what
    // is under it.
    void ScaleDown(const CharBoneStateSet& set, float weight, bool relative);  // 0x332AE0
    // Adds the transform with the weight to the frame's root motion; a
    // rotation against the accumulated one is added negated, and a negative
    // weight applies the inverse rotation.
    void ApplyDeltaRootXfm(const QuatXfm& xfm, float weight);  // 0x332F90
    // Scales the frame's root motion and its weight.
    void ScaleDownDeltaRootXfm(float scale);  // 0x3330E0

    // The absolute channels.
    CharBoneStateBuffer mAbsolute;
    // The channels of the relative animations.
    CharBoneStateBuffer mRelative;
    // The frame's root motion and the weight accumulated into it.
    QuatXfm mDeltaRootXfm;
    float mDeltaRootWeight;
    const CharSkeleton* mSkeleton;
    // The file of the skeleton, for reports. Weakly supported.
    ResourcePath mSkeletonPath;
};

static_assert(offsetof(CharAnimBuffer, mRelative) == 104);
static_assert(offsetof(CharAnimBuffer, mDeltaRootXfm) == 208);
static_assert(offsetof(CharAnimBuffer, mDeltaRootWeight) == 236);
static_assert(offsetof(CharAnimBuffer, mSkeleton) == 240);
static_assert(offsetof(CharAnimBuffer, mSkeletonPath) == 248);
static_assert(sizeof(CharAnimBuffer) == 256);
