#pragma once

#include <cstddef>

namespace Hmx {

class Matrix3;

// A rotation quaternion (the map's Hmx::Quat, used by math/Rot.o and the
// animation code). Only the layout the reconstructed code reads is
// modelled.
class Quat {
public:
    float x;
    float y;
    float z;
    float w;
};

static_assert(offsetof(Quat, w) == 12);
static_assert(sizeof(Quat) == 16);

}  // namespace Hmx

// The rotation matrix of the quaternion. Not reconstructed.
void MakeRotMatrix(const Hmx::Quat& quat, Hmx::Matrix3& matrix);  // 0x216BF0
