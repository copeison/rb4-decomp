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

// Cofactor expansion along the first row.
float Det(const Hmx::Matrix4& matrix);  // 0x11798A0
// Inverts through the adjugate. When |Det(matrix)| is not above epsilon the
// adjugate is scaled by zero instead of 1 / det.
void Invert(
    const Hmx::Matrix4& matrix,
    Hmx::Matrix4& inverse,
    float epsilon);  // 0x1179A80
