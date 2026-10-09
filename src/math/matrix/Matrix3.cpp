#include "math/matrix/Matrix3.h"

// The binary fills this in the math/Matrix3.o static initializer at 0x215960
// (the original constructs it through Matrix3's non-constexpr constructor);
// the aggregate holds the same identity values.
const Hmx::Matrix3 Hmx::Matrix3::sID = {
    {1.0F, 0.0F, 0.0F},
    {0.0F, 1.0F, 0.0F},
    {0.0F, 0.0F, 1.0F},
};

// Reconstructed from eboot.elf at 0x2152E0. The binary sums the first and
// third terms before subtracting the second.
float Det(const Hmx::Matrix3& matrix) {
    const Vector3& x = matrix.x;
    const Vector3& y = matrix.y;
    const Vector3& z = matrix.z;
    return (x.x * (y.y * z.z - y.z * z.y) + x.z * (y.x * z.y - z.x * y.y)) -
        x.y * (y.x * z.z - z.x * y.z);
}
