#pragma once

#include <cstddef>

#include "math/rotation/Quat.h"
#include "math/transform/Transform.h"
#include "math/vector/Vector3.h"

// A bone's transform relative to its parent (anim/CharBoneTransform.o; the
// map's object holds only inline copies): a rotation, a scale and a
// translation, with the parent's scale, which the bone does not inherit.
// Field names are not in the reference map. The object is 52 bytes.
class CharBoneTransform {
public:
    // No rotation, unit scales and no translation; the scales are copied
    // from Vector3's (1, 1, 1) at 0x19B0364. Inlined into the vector
    // growth at 0x2F99B0.
    CharBoneTransform()
        : mRotation{0.0f, 0.0f, 0.0f, 1.0f},
          mScale{1.0f, 1.0f, 1.0f},
          mTranslation(Vector3::sZero),
          mParentScale{1.0f, 1.0f, 1.0f} {}

    // The transform the bone applies: the rotation's rows scaled by the
    // bone's scale, its columns divided by the parent's scale, and the
    // translation. Inlined into CharSkeleton::ComputeProjectSpaceXfm
    // (0x2F7440) and the pose updates (0x2FACB0, 0x2FAEB0). Name not in
    // the reference map.
    void ToTransform(Transform& xfm) const {
        xfm.m = Hmx::Matrix3::sID;
        MakeRotMatrix(mRotation, xfm.m);
        const float columns[3] = {1.0f / mParentScale.x, 1.0f / mParentScale.y, 1.0f / mParentScale.z};
        Vector3* const rows[3] = {&xfm.m.x, &xfm.m.y, &xfm.m.z};
        const float scales[3] = {mScale.x, mScale.y, mScale.z};
        for (int row = 0; row < 3; ++row) {
            rows[row]->x = rows[row]->x * scales[row] * columns[0];
            rows[row]->y = rows[row]->y * scales[row] * columns[1];
            rows[row]->z = rows[row]->z * scales[row] * columns[2];
        }
        xfm.v = mTranslation;
    }

    Hmx::Quat mRotation;
    Vector3 mScale;
    Vector3 mTranslation;
    Vector3 mParentScale;
};

static_assert(offsetof(CharBoneTransform, mScale) == 16);
static_assert(offsetof(CharBoneTransform, mTranslation) == 28);
static_assert(offsetof(CharBoneTransform, mParentScale) == 40);
static_assert(sizeof(CharBoneTransform) == 52);
