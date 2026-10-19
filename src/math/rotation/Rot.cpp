#include "math/rotation/Rot.h"

#include <math.h>

#include "math/rotation/Quat.h"

// Reconstructed from eboot.elf at 0x215B10. The vertical threshold is
// 0x3F7FFFFE.
void MakeEuler(const Hmx::Matrix3& matrix, Vector3& euler) {
    if (fabsf(matrix.y.z) > 0.99999988F) {
        euler.x = matrix.y.z > 0.0F ? 1.5707964F : -1.5707964F;
        euler.z = atan2f(matrix.x.y, matrix.x.x);
        euler.y = 0.0F;
    } else {
        euler.z = atan2f(-matrix.y.x, matrix.y.y);
        euler.x = asinf(matrix.y.z);
        euler.y = atan2f(-matrix.x.z, matrix.z.z);
    }
}

// Reconstructed from eboot.elf at 0x215BC0.
bool IsVertical(const Hmx::Matrix3& matrix) {
    return matrix.z.x == 0.0F && matrix.z.y == 0.0F && matrix.z.z == 1.0F &&
        matrix.x.z == 0.0F && matrix.y.z == 0.0F;
}

// Reconstructed from eboot.elf at 0x216BF0.
void MakeRotMatrix(const Hmx::Quat& quat, Hmx::Matrix3& matrix) {
    const float x2 = quat.x + quat.x;
    const float y2 = quat.y + quat.y;
    const float z2 = quat.z + quat.z;
    const float xx = quat.x * x2;
    const float xy = x2 * quat.y;
    const float xz = quat.z * x2;
    const float yy = quat.y * y2;
    const float yz = y2 * quat.z;
    const float zz = quat.z * z2;
    const float wx = quat.w * x2;
    const float wy = quat.w * y2;
    const float wz = z2 * quat.w;
    matrix.x.x = 1.0F - yy - zz;
    matrix.x.y = wz + xy;
    matrix.x.z = xz - wy;
    matrix.y.x = xy - wz;
    matrix.y.y = 1.0F - xx - zz;
    matrix.y.z = wx + yz;
    matrix.z.x = wy + xz;
    matrix.z.y = yz - wx;
    matrix.z.z = 1.0F - xx - yy;
}
