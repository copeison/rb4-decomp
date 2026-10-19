#pragma once

#include <cstddef>

#include "anim/skeleton/CharBoneTransform.h"
#include "math/transform/Transform.h"
#include "utl/containers/Vector.h"

class CharBoneStateBuffer;
class CharSkeleton;

// A pose of a skeleton (anim/CharSkeletonPose.o, 0x2FA3E0-0x2FB6CF): each
// bone's transform relative to its parent and its object-space transform,
// computed on demand from the parents', under a root transform the
// animation's root motion moves. RndSkeletonPoseCom holds one at offset
// 152; CharAnimDriver writes the blended channels into it and the IK
// components edit it. The object is 120 bytes, without a vtable. Field
// names are not in the reference map.
class CharSkeletonPose {
public:
    // A bone of the pose (the map's CharSkeletonPose::BoneData; 104 bytes).
    struct BoneData {
        // An identity bone. Inlined into the vector growth (0x2FB460).
        BoneData()
            : mLocal(), mObject{{{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, {0.0f, 0.0f, 0.0f}},
              mDirty(false) {}

        // The transform relative to the parent.
        CharBoneTransform mLocal;
        // The transform in the pose's object space, valid unless mDirty.
        Transform mObject;
        bool mDirty;
    };

    CharSkeletonPose();   // 0x2FA3E0
    ~CharSkeletonPose();  // 0x2FA460

    // Forgets which bone each channel of the buffer drives.
    void Reset();  // 0x2FA4B0
    // Sets the root transform. Name not in the reference map.
    void SetRootXfm(const Transform& xfm);  // 0x2FA4C0
    // Makes the root transform the identity and records the render frame
    // it happened on. Name not in the reference map.
    void ResetRootXfm();  // 0x2FA510
    // Sizes the pose for the skeleton, maps each channel of the buffer's
    // format to the skeleton bone of its name (the bones of a kind are
    // searched in skeleton order) and resets to the bind pose.
    void Init(const CharSkeleton* skeleton, const CharBoneStateBuffer* buffer);  // 0x2FA550
    // Makes the root the identity and every bone its bind transform.
    void ResetToBindPose(const CharSkeleton* skeleton);  // 0x2FA820
    // Copies the buffer's weighted positions, scales, inverse parent scales
    // and rotations into the bones, moves the root by `deltaRoot` and
    // keeps it orthonormal; every bone becomes dirty. The skeleton is not
    // read.
    void UpdateTransforms(
        const CharSkeleton* skeleton,
        const CharBoneStateBuffer* buffer,
        const Transform& deltaRoot);  // 0x2FA960
    // The object-space transforms of bones given their parents and local
    // transforms; parents come before their children.
    static void FlattenHierarchy(
        const eastl::vector<unsigned int>& parents,
        const eastl::vector<CharBoneTransform>& bones,
        eastl::vector<Transform>& xfms);  // 0x2FACB0
    // The bone's object-space transform, computed through its dirty
    // ancestors; the identity for an empty pose.
    const Transform& GetObjectXfm(unsigned long bone, const CharSkeleton* skeleton) const;  // 0x2FAEB0
    const CharBoneTransform& GetLocalBoneTrans(unsigned long bone) const;  // 0x2FB0D0
    // Sets the bone's local transform; it and its descendants become dirty.
    void SetLocalCharBoneTrans(unsigned long bone, const CharBoneTransform& xfm, const CharSkeleton* skeleton);  // 0x2FB0E0
    // Marks the bone's descendants dirty, stopping at those already dirty.
    void _SetRecomputeChildren(unsigned long bone, const CharSkeleton* skeleton);  // 0x2FB150
    // Sets the bone's object-space transform; its descendants become dirty.
    void SetObjectXfm(unsigned long bone, const Transform& xfm, const CharSkeleton* skeleton);  // 0x2FB1F0
    // Recomputes the object-space transforms of the dirty bones in order.
    void RecomputeAllWorld(const CharSkeleton* skeleton);  // 0x2FB290

    mutable eastl::vector<BoneData> mBones;
    // The skeleton bone each channel of the buffer drives.
    eastl::vector<unsigned int> mChannelBones;
    // The pose's root transform.
    Transform mRootXfm;
    // The render frame (RndDevice::mFrameCount) of the last ResetRootXfm;
    // -1 until then.
    unsigned long mRootResetFrame;
};

static_assert(offsetof(CharSkeletonPose::BoneData, mObject) == 52);
static_assert(offsetof(CharSkeletonPose::BoneData, mDirty) == 100);
static_assert(sizeof(CharSkeletonPose::BoneData) == 104);
static_assert(offsetof(CharSkeletonPose, mChannelBones) == 32);
static_assert(offsetof(CharSkeletonPose, mRootXfm) == 64);
static_assert(offsetof(CharSkeletonPose, mRootResetFrame) == 112);
static_assert(sizeof(CharSkeletonPose) == 120);
