#pragma once

#include <cstddef>

#include "math/matrix/Matrix3.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector3.h"
#include "utl/text/Symbol.h"

// The transform component (entity/Utl.o in the map). Its Component base and
// most of its methods are not reconstructed; only the class id, the local
// transform that the renderer's default lighting sets, and the world
// transform that the renderer reads are modelled.
class TransCom {
public:
    // The local transform as scale, Euler angles and position. Name and
    // field names not in the reference map.
    struct LocalXfm {
        // Splits the rotation into the row scales and the Euler angles
        // (through MakeEuler at 0x215B10). RndDefaults passes true, which
        // takes the scale signs from the stored scale. Name not in the
        // reference map.
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

    // Assigned when the component class registers; GetId (0x1B4CB0) returns
    // it.
    static Symbol sId;  // 0x19E46E8

    // Field names are not in the reference map.
    unsigned char mUnknown0[24];
    LocalXfm mLocalXfm;
    unsigned char mUnknown60[12];
    unsigned char mDirtyFlags;
    unsigned char mUnknown73[7];
    Transform mWorldXfm;
};

static_assert(offsetof(TransCom, mLocalXfm) == 24);
static_assert(offsetof(TransCom::LocalXfm, mPos) == 24);
static_assert(offsetof(TransCom, mDirtyFlags) == 72);
static_assert(offsetof(TransCom, mWorldXfm) == 80);
