#pragma once

#include <cstddef>

#include "math/matrix/Matrix3.h"
#include "math/vector/Vector3.h"

// Rotation plus translation.
class Transform {
public:
    Hmx::Matrix3 m;
    Vector3 v;
};

static_assert(offsetof(Transform, v) == 36);
static_assert(sizeof(Transform) == 48);
