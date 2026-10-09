#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "math/matrix/Matrix3.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector3.h"
#include "utl/text/Symbol.h"

// The transform component (entity/TransCom.o). Its Component base and most
// of its methods are not reconstructed; only the class symbols, the local
// transform that the renderer's default lighting sets, the parent and the
// world transform that the renderer reads are modelled.
class TransCom {
public:
    // The local transform as scale, Euler angles and position. Name and
    // field names not in the reference map.
    struct LocalXfm {
        // Splits the rotation into the row scales and the Euler angles
        // (through MakeEuler at 0x215B10). With `keepScaleSigns`, which
        // RndDefaults passes, the scale signs come from the stored scale.
        // Name not in the reference map.
        void SetRotation(const Hmx::Matrix3& rotation, bool keepScaleSigns);  // 0x127730

        Vector3 mScale;
        Vector3 mEuler;
        Vector3 mPos;
    };

    // Sets the local transform and marks it dirty. Inlined into
    // RndDefaults::_SyncSpotlight (0x6BFD40) and
    // RndDefaults::_CreateBackupLighting (0x6BF4F0). Name not in the
    // reference map.
    void SetLocalXfm(const Transform& xfm) {
        mLocalXfm.SetRotation(xfm.m, true);
        mLocalXfm.mPos = xfm.v;
        mDirtyFlags |= 1;
    }

    // Makes the object the parent, or the invalid id for none. `keepWorld`
    // also marks the world transform for keeping (flag 0x10).
    void SetTransParent(GameObjectId parent, bool keepWorld);  // 0x1B3A90

    // Assigned when the component class registers (0x1AFFB0); the class's
    // virtual at slot 4 (0x1B4CB0) returns it.
    static Symbol sId;  // 0x19E46E8
    // The class symbol that GameObject::CreateComponent takes, returned by
    // the virtual at slot 5 (0x1B4CC0). Name not in the reference map.
    static Symbol sClassName;  // 0x19E46F0

    // Field names are not in the reference map. The property registration
    // (0x1AFFB0) builds a prototype whose fields give the defaults below.
    // The Component base (vtable, owning object and flags).
    unsigned char mComponentBase[24];
    LocalXfm mLocalXfm;
    // The parent object, or the invalid id (-1). The map has
    // TransCom::SetTransParent(GameObjectId, bool); the poll (0x1B4140)
    // reads it.
    GameObjectId mTransParent;
    // Zeroed by the prototype; no TransCom method reads it.
    unsigned int mReserved;
    // Which parent attributes are inherited: 0 the entire transform, 1 scale
    // and rotation, 2 translation. Registered as "inherit_type".
    int mInheritType;
    // Flags: 1 marks the local transform dirty, 4 the parent changed and
    // 0x10 a parent change that keeps the world transform.
    unsigned int mDirtyFlags;
    // Makes the entity's parent object (Entity+304) the parent instead of
    // mTransParent. Name inferred from its use in the poll (0x1B4140).
    bool mUseEntityParent;
    unsigned char mPadding[3];  // Never read or written.
    Transform mWorldXfm;
};

static_assert(offsetof(TransCom, mLocalXfm) == 24);
static_assert(offsetof(TransCom::LocalXfm, mPos) == 24);
static_assert(offsetof(TransCom, mTransParent) == 60);
static_assert(offsetof(TransCom, mInheritType) == 68);
static_assert(offsetof(TransCom, mDirtyFlags) == 72);
static_assert(offsetof(TransCom, mUseEntityParent) == 76);
static_assert(offsetof(TransCom, mWorldXfm) == 80);
