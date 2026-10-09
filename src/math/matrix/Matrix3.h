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

    static const Matrix3 sID;  // 0x19E65E0
};

static_assert(offsetof(Matrix3, y) == 12);
static_assert(offsetof(Matrix3, z) == 24);
static_assert(sizeof(Matrix3) == 36);

}  // namespace Hmx

// Cofactor expansion along the first row.
float Det(const Hmx::Matrix3& matrix);  // 0x2152E0
// Inverts through the adjugate and stores the determinant when det is not
// null. A zero determinant gives a zero matrix. The inverse may alias the
// input. The map has Invert(Hmx::Matrix3 const&, Hmx::Matrix3&); this build
// adds the determinant.
void Invert(
    const Hmx::Matrix3& matrix,
    Hmx::Matrix3& inverse,
    float* det);  // 0x215340

// Inlined everywhere it is used. Name not in the reference map.
inline void Transpose(const Hmx::Matrix3& matrix, Hmx::Matrix3& transpose) {
    const Hmx::Matrix3 m = matrix;
    transpose.x = {m.x.x, m.y.x, m.z.x};
    transpose.y = {m.x.y, m.y.y, m.z.y};
    transpose.z = {m.x.z, m.y.z, m.z.z};
}
