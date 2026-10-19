#pragma once

#include <cstddef>

#include "anim/skeleton/CharSkeletonPose.h"
#include "entity/core/Component.h"
#include "entity/resources/Resource.h"
#include "math/transform/Transform.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class CharSkeleton;
class CharBoneStateBuffer;
class CharSkeletonResource;
class RndShaderCBuffer;

// The component that poses a skeleton on an entity's root object
// (render/RndSkeletonPoseCom.o; vtable 0x19019D8, 42 slots; destructor
// 0x43DDA0). Its class id is "Skeleton". Its methods are not reconstructed;
// only the members the mesh component reads are declared. Field names are
// not in the reference map.
class RndSkeletonPoseCom : public Component {
public:
    // The class symbol, "Skeleton", constructed by the static initializer at
    // 0x44141A.
    static Symbol sId;  // 0x1A73BE0

    // Slots 0-1: 0x43DDA0 and the deleting destructor. Not reconstructed.
    ~RndSkeletonPoseCom() override;

    // The pose's own skeleton, or else its skeleton resource's while that
    // has not failed. Not reconstructed.
    const CharSkeleton* GetSkeleton() const;  // 0x43EC60
    // The file of the skeleton GetSkeleton returns: the own skeleton's
    // (offset 104) or the "skeleton_file" (offset 40). CharAnimDriver hands
    // it to its CharAnimBuffer. Name not in the reference map; the evidence
    // is weak. Not reconstructed.
    ResourcePath GetSkeletonPath() const;  // 0x43ED30
    // Lays the pose out for the buffer's channels (CharSkeletonPose::Init
    // with GetSkeleton). Not reconstructed.
    void Init(const CharBoneStateBuffer* buffer);  // 0x43EE90
    // Writes the buffer's channels into the pose and moves its root
    // (CharSkeletonPose::UpdateTransforms with GetSkeleton). Not
    // reconstructed.
    void UpdateTransforms(const CharBoneStateBuffer* buffer, const Transform& deltaRoot);  // 0x43EEF0
    // Recomputes the pose's object transforms and the render transforms,
    // and fills the bone constant buffer. The map's signature takes an
    // ObjPtr const&. Not reconstructed.
    void UpdateRenderTransforms();  // 0x43F070

    // The members from 22 to the skeleton resource; not modelled.
    unsigned char mMembers[66];
    // The skeleton resource, whose skeleton GetSkeleton falls back on while
    // it has not failed. CharClipCom builds its clip on it. The name is
    // weak.
    ResourcePtr<CharSkeletonResource> mSkeletonResource;
    // The pose's own skeleton, which GetSkeleton prefers. The name is weak.
    CharSkeleton* mSkeleton;
    // Not modelled.
    unsigned char mMembers2[8];
    // The bones' render transforms in object space, which
    // RndMeshCom::_PollSkinnedSphere reads (the map's
    // UpdateRenderTransforms fills them). The name is weak.
    eastl::vector<Transform> mRenderXfms;
    // The bone constant buffer the skinned draw instances bind
    // (RndDrawInstance::mInstanceCBuffer).
    RndShaderCBuffer* mBonesCBuffer;
    // The bones' pose; CharClipCom's preview returns it to the bind pose
    // when it stops. The name is weak.
    CharSkeletonPose mPose;
};

static_assert(offsetof(RndSkeletonPoseCom, mRenderXfms) == 112);
static_assert(offsetof(RndSkeletonPoseCom, mSkeletonResource) == 88);
static_assert(offsetof(RndSkeletonPoseCom, mSkeleton) == 96);
static_assert(offsetof(RndSkeletonPoseCom, mBonesCBuffer) == 144);
static_assert(offsetof(RndSkeletonPoseCom, mPose) == 152);
