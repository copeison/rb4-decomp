#include "math/matrix/Matrix3.h"

// The binary fills this in the math/Matrix3.o static initializer at 0x215960
// (the original constructs it through Matrix3's non-constexpr constructor);
// the aggregate holds the same identity values.
const Hmx::Matrix3 Hmx::Matrix3::sID = {
    {1.0F, 0.0F, 0.0F},
    {0.0F, 1.0F, 0.0F},
    {0.0F, 0.0F, 1.0F},
};

// Reconstructed from eboot.elf at 0x2150D0. The triple product of the rows.
bool IsRightHanded(const Hmx::Matrix3& matrix) {
    const Vector3& x = matrix.x;
    const Vector3& y = matrix.y;
    const Vector3& z = matrix.z;
    const float triple = (x.y * y.z - x.z * y.y) * z.x + (y.x * x.z - x.x * y.z) * z.y
        + (y.y * x.x - x.y * y.x) * z.z;
    return triple >= 0.0F;
}

// Reconstructed from eboot.elf at 0x2152E0. The binary sums the first and
// third terms before subtracting the second.
float Det(const Hmx::Matrix3& matrix) {
    const Vector3& x = matrix.x;
    const Vector3& y = matrix.y;
    const Vector3& z = matrix.z;
    return (x.x * (y.y * z.z - y.z * z.y) + x.z * (y.x * z.y - z.x * y.y)) -
        x.y * (y.x * z.z - z.x * y.z);
}

// Reconstructed from eboot.elf at 0x215340. The binary reads the input
// before it stores the rows they would overwrite, so the inverse may alias
// it.
void Invert(const Hmx::Matrix3& matrix, Hmx::Matrix3& inverse, float* det) {
    const Vector3& x = matrix.x;
    const Vector3& y = matrix.y;
    const Vector3& z = matrix.z;
    const float d = ((z.z * y.y - y.z * z.y) * x.x + (z.x * y.z - y.x * z.z) * x.y) +
        (y.x * z.y - z.x * y.y) * x.z;
    if (det != nullptr) {
        *det = d;
    }
    float invDet = 0.0F;
    if (d != 0.0F) {
        invDet = 1.0F / d;
    }

    Hmx::Matrix3 result;
    result.x.x = (z.z * y.y - z.y * y.z) * invDet;
    result.x.y = invDet * (x.z * z.y - x.y * z.z);
    result.x.z = (x.y * y.z - x.z * y.y) * invDet;
    result.y.x = invDet * (z.x * y.z - y.x * z.z);
    result.y.y = (x.x * z.z - z.x * x.z) * invDet;
    result.y.z = invDet * (y.x * x.z - x.x * y.z);
    result.z.x = (y.x * z.y - z.x * y.y) * invDet;
    result.z.y = invDet * (z.x * x.y - x.x * z.y);
    result.z.z = (x.x * y.y - y.x * x.y) * invDet;
    inverse = result;
}
