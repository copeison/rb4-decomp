#pragma once

#include <cstddef>

#include "math/vector/Vector3.h"

namespace Hmx {

// Row-major 3x3 rotation. The rows are the local right, forward and up axes.
class Matrix3 {
public:
    Vector3 x;
    Vector3 y;
    Vector3 z;
};

static_assert(offsetof(Matrix3, y) == 12);
static_assert(offsetof(Matrix3, z) == 24);
static_assert(sizeof(Matrix3) == 36);

}  // namespace Hmx
