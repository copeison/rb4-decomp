#pragma once

#include <cstddef>

#include "math/transform/Transform.h"
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

// The transform as a 4x4 matrix with a (0, 0, 0, 1) column, times the
// matrix: each rotation row combines the matrix's first three rows and the
// translation row adds the fourth. The binary keeps an out-of-line copy at
// 0x3DBE40 among RndCameraContext.o's functions. Name not in the reference
// map.
inline Hmx::Matrix4 operator*(const Transform& xfm, const Hmx::Matrix4& matrix) {
    const Vector4& r0 = matrix.x;
    const Vector4& r1 = matrix.y;
    const Vector4& r2 = matrix.z;
    const Vector4& r3 = matrix.w;
    const Vector3* rows[] = {&xfm.m.x, &xfm.m.y, &xfm.m.z};
    Hmx::Matrix4 result = {};
    Vector4* out[] = {&result.x, &result.y, &result.z};
    for (int i = 0; i < 3; ++i) {
        const Vector3& row = *rows[i];
        *out[i] = {
            row.x * r0.x + row.y * r1.x + row.z * r2.x,
            row.x * r0.y + row.y * r1.y + row.z * r2.y,
            row.x * r0.z + row.y * r1.z + row.z * r2.z,
            row.x * r0.w + row.y * r1.w + row.z * r2.w,
        };
    }
    const Vector3& v = xfm.v;
    result.w = {
        v.x * r0.x + v.y * r1.x + v.z * r2.x + r3.x,
        v.x * r0.y + v.y * r1.y + v.z * r2.y + r3.y,
        v.x * r0.z + v.y * r1.z + v.z * r2.z + r3.z,
        v.x * r0.w + v.y * r1.w + v.z * r2.w + r3.w,
    };
    return result;
}
