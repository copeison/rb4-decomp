#pragma once

#include <cstddef>

#include "math/matrix/Matrix3.h"
#include "math/vector/Vector3.h"

// Rotation plus translation.
class Transform {
public:
    Hmx::Matrix3 m;
    Vector3 v;

    static const Transform sID;  // 0x19E666C
};

static_assert(offsetof(Transform, v) == 36);
static_assert(sizeof(Transform) == 48);

// Composes the two transforms into the result.
void Multiply(const Transform& a, const Transform& b, Transform& result);  // 0x2187E0
