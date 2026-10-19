#pragma once

#include <cstddef>

namespace Hmx {

class Matrix3;

// A rotation quaternion (the map's Hmx::Quat, used by math/Rot.o and the
// animation code). Only the layout the reconstructed code reads is
// modelled.
class Quat {
public:
    // The rotation of the matrix (math/Rot.o). Not reconstructed.
    void Set(const Matrix3& matrix);  // 0x2165F0

    float x;
    float y;
    float z;
    float w;
};

static_assert(offsetof(Quat, w) == 12);
static_assert(sizeof(Quat) == 16);

}  // namespace Hmx

// The rotation matrix of the quaternion (math/Rot.o).
void MakeRotMatrix(const Hmx::Quat& quat, Hmx::Matrix3& matrix);  // 0x216BF0
// The unit quaternion of `quat` in `result`; a zero quaternion gives the
// identity (math/Rot.o). Not reconstructed.
void Normalize(const Hmx::Quat& quat, Hmx::Quat& result);  // 0x215A30
// The normalized linear blend from `a` to `b` by `t`, taking `b` on `a`'s
// hemisphere; `a` at 0 and `b` at 1 exactly (math/Rot.o). The evidence for
// the name, among the map's Interp and FastInterp, is weak. Not
// reconstructed.
void FastInterp(const Hmx::Quat& a, const Hmx::Quat& b, float t, Hmx::Quat& result);  // 0x216890
