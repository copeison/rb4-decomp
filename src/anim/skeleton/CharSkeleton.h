#pragma once

#include <cstddef>

#include "anim/skeleton/CharBoneTransform.h"
#include "entity/props/PropMetadata.h"
#include "math/matrix/Matrix3.h"
#include "math/transform/Transform.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BinStream;

// A character's bones (anim/CharSkeleton.o, 0x2F7260-0x2FA3DB): each bone's
// parent, name, bind transform relative to its parent and mesh-to-bone
// transform, with the children of each bone and the roots derived from the
// parents. A root bone is its own parent; parents come before their
// children. CharSkeletonResource owns one. Field names are not in the
// reference map. The object is 248 bytes, without a vtable.
class CharSkeleton {
public:
    // Inlined into New (0x2F7300).
    CharSkeleton()
        : mParentIndices(),
          mChildIndices(),
          mLocalTransforms(),
          mNames(),
          mMeshToBoneTransforms(),
          mRootTransform(Transform::sID),
          mRootRotation(Hmx::Matrix3::sID),
          mUnitScale(1.0f) {}
    ~CharSkeleton();  // 0x2F7260

    // Allocates an empty skeleton.
    static CharSkeleton* New();  // 0x2F7300

    // The index of the bone with the name, or -1.
    long FindBoneIndex(Symbol name) const;  // 0x2F7400
    // The bone's transform in the skeleton's space: its bind transform
    // composed with its ancestors' up to its root.
    void ComputeProjectSpaceXfm(unsigned long bone, Transform& xfm) const;  // 0x2F7440
    // The bone names as allowed values for a bone property, sorted without
    // regard to case, after a "none" entry when `includeNone` is set.
    eastl::vector<AllowedValue<Symbol>> GetAllowedValueBones(bool includeNone) const;  // 0x2F7750
    // Reads the skeleton; false for a revision before 4.
    bool Load(BinStream& stream);  // 0x2F7A10
    // Writes the skeleton at revision 4.
    void Save(BinStream& stream) const;  // 0x2F7E80
    // Sets the parents and rebuilds the child lists.
    void SetParentIndices(const eastl::vector<unsigned int>& parents);  // 0x2F82E0
    // Rebuilds mChildIndices from the parents.
    void _ComputeChildIndices();  // 0x2F83E0
    // The bone's children and their number.
    const unsigned int* GetChildIndices(unsigned long bone, unsigned long& count) const;  // 0x2F8770
    // The root bones and their number.
    const unsigned int* GetRootIndices(unsigned long& count) const;  // 0x2F8790
    void SetLocalTransforms(const eastl::vector<CharBoneTransform>& transforms);  // 0x2F87D0
    void SetMeshToBoneTransforms(const eastl::vector<Transform>& transforms);    // 0x2F87F0
    void SetNames(const eastl::vector<Symbol>& names);                           // 0x2F8810

    // Each bone's parent; a root is its own parent.
    eastl::vector<unsigned int> mParentIndices;
    // For each bone the offset of its children, then the end offset, then
    // the children lists, then the roots.
    eastl::vector<unsigned int> mChildIndices;
    // Each bone's bind transform relative to its parent.
    eastl::vector<CharBoneTransform> mLocalTransforms;
    // The bone names, by bone index.
    eastl::vector<Symbol> mNames;
    // Each bone's transform from the mesh's space.
    eastl::vector<Transform> mMeshToBoneTransforms;
    // A transform, a rotation and a scale of the whole skeleton, identity
    // until loaded. No reader was found in this object; the names are
    // weakly supported.
    Transform mRootTransform;
    Hmx::Matrix3 mRootRotation;
    float mUnitScale;
};

static_assert(offsetof(CharSkeleton, mChildIndices) == 32);
static_assert(offsetof(CharSkeleton, mLocalTransforms) == 64);
static_assert(offsetof(CharSkeleton, mNames) == 96);
static_assert(offsetof(CharSkeleton, mMeshToBoneTransforms) == 128);
static_assert(offsetof(CharSkeleton, mRootTransform) == 160);
static_assert(offsetof(CharSkeleton, mRootRotation) == 208);
static_assert(offsetof(CharSkeleton, mUnitScale) == 244);
static_assert(sizeof(CharSkeleton) == 248);
