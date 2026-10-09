#pragma once

#include <cstddef>

#include "math/vector/Vector4.h"

namespace Hmx {

// Row-major 4x4 matrix.
class Matrix4 {
public:
    Vector4 x;
    Vector4 y;
    Vector4 z;
    Vector4 w;

    static const Matrix4 sID;  // 0x1B5D1EC
};

static_assert(offsetof(Matrix4, w) == 48);
static_assert(sizeof(Matrix4) == 64);

}  // namespace Hmx

float Det(const Hmx::Matrix4& matrix);  // 0x11798A0
// Inverts with the determinant the caller passes.
void Invert(const Hmx::Matrix4& matrix, Hmx::Matrix4& inverse, float det);  // 0x1179A80
