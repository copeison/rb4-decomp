#include "math/rotation/Rot.h"

#include <math.h>

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
