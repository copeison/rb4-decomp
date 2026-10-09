#pragma once

#include "math/matrix/Matrix3.h"
#include "math/vector/Vector3.h"

// Rotation helpers (math/Rot.o). Only the matrix functions below have been
// reconstructed.

// The Euler angles, in radians, of a rotation: x is the pitch from y.z, z the
// heading of the y row and y the roll of the x and z rows. Near vertical the
// roll is zero and the heading comes from the x row. The binary also has a
// thunk at 0x218610 that takes the arguments in the other order.
void MakeEuler(const Hmx::Matrix3& matrix, Vector3& euler);  // 0x215B10
// Whether z is the world up axis and x and y are level.
bool IsVertical(const Hmx::Matrix3& matrix);  // 0x215BC0
