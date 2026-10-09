#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "math/transform/Transform.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

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

    // The members from 22 to the bone transforms; not modelled.
    unsigned char mMembers[90];
    // The bones' render transforms in object space, which
    // RndMeshCom::_PollSkinnedSphere reads (the map's
    // UpdateRenderTransforms fills them). The name is weak.
    eastl::vector<Transform> mRenderXfms;
    // The bone constant buffer the skinned draw instances bind
    // (RndDrawInstance::mInstanceCBuffer).
    RndShaderCBuffer* mBonesCBuffer;
};

static_assert(offsetof(RndSkeletonPoseCom, mRenderXfms) == 112);
static_assert(offsetof(RndSkeletonPoseCom, mBonesCBuffer) == 144);
