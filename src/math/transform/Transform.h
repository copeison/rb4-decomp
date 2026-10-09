#pragma once

#include <cstddef>

#include "math/matrix/Matrix3.h"
#include "math/vector/Vector3.h"

// Rotation plus translation.
class Transform {
public:
    Hmx::Matrix3 m;
    Vector3 v;

    static const Transform sZero;  // 0x19E663C
    static const Transform sID;    // 0x19E666C
};

static_assert(offsetof(Transform, v) == 36);
static_assert(sizeof(Transform) == 48);

// Composes the two transforms into the result.
void Multiply(const Transform& a, const Transform& b, Transform& result);  // 0x2187E0

// Inverts the rotation and moves the negated translation into the inverse
// frame. Inlined by its users (RndCameraContext::_CalcWorldXfms at
// 0x3DA67C). Name not in the reference map.
inline void Invert(const Transform& xfm, Transform& inverse) {
    Invert(xfm.m, inverse.m, nullptr);
    const Vector3 v = xfm.v;
    const Hmx::Matrix3& m = inverse.m;
    inverse.v.x = -(v.x * m.x.x + v.y * m.y.x + v.z * m.z.x);
    inverse.v.y = -(v.x * m.x.y + v.y * m.y.y + v.z * m.z.y);
    inverse.v.z = -(v.x * m.x.z + v.y * m.y.z + v.z * m.z.z);
}

// Transforms a point: applies the rotation and adds the translation. The
// result may alias the point. Inlined by its users
// (RndMeshUtl::ComputeBoundingSphere at 0x5D86E0). Name not in the
// reference map.
inline void Multiply(const Vector3& point, const Transform& xfm, Vector3& result) {
    const Vector3 p = point;
    const Hmx::Matrix3& m = xfm.m;
    const float x = p.x * m.x.x + p.z * m.z.x + p.y * m.y.x + xfm.v.x;
    const float y = p.x * m.x.y + p.z * m.z.y + p.y * m.y.y + xfm.v.y;
    const float z = p.x * m.x.z + p.z * m.z.z + p.y * m.y.z + xfm.v.z;
    result = {x, y, z};
}
