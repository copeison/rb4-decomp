#pragma once

#include "math/matrix/Matrix3.h"
#include "math/vector/Vector3.h"

namespace Hmx {
class Quat;
}

// Rotation helpers (math/Rot.o). Only the matrix functions below have been
// reconstructed.

// The Euler angles, in radians, of a rotation: x is the pitch from y.z, z the
// heading of the y row and y the roll of the x and z rows. Near vertical the
// roll is zero and the heading comes from the x row. The binary also has a
// thunk at 0x218610 that takes the arguments in the other order.
void MakeEuler(const Hmx::Matrix3& matrix, Vector3& euler);  // 0x215B10
// The Euler angles, in radians, of the quaternion's rotation. Not
// reconstructed.
void MakeEuler(const Hmx::Quat& quat, Vector3& euler);  // 0x2179C0
// Whether z is the world up axis and x and y are level.
bool IsVertical(const Hmx::Matrix3& matrix);  // 0x215BC0
// The rotation of the Euler angles, in radians. With `useTable` the sines
// come from the interpolated table (Sine), otherwise from the C library.
// Not reconstructed.
void MakeRotMatrix(const Vector3& euler, Hmx::Matrix3& matrix, bool useTable);  // 0x217090
// The Euler angles, in radians, and the row lengths of a scaled rotation.
// Not reconstructed.
void MakeEulerScale(const Hmx::Matrix3& matrix, Vector3& euler, Vector3& scale);  // 0x215EE0
// The row lengths of a scaled rotation, the z scale negated for a
// left-handed one. Not reconstructed.
void MakeScale(const Hmx::Matrix3& matrix, Vector3& scale);  // 0x215D90

// Interpolates two sets of Euler angles through the rotations' quaternions,
// along the shorter arc. Declared for the state graph nodes; not
// reconstructed. Which of the two functions has the map's name is not
// certain; this one does the work that name suggests.
void InterpEuler(const Vector3& from, const Vector3& to, float t, Vector3& result);  // 0x218730
// Interpolates the angles one by one. Name not in the reference map.
// Declared for the state graph nodes; not reconstructed.
void InterpEulerLinear(const Vector3& from, const Vector3& to, float t, Vector3& result);  // 0x2186E0
