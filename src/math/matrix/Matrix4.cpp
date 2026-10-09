#include "math/matrix/Matrix4.h"

#include <cmath>

#include "math/matrix/Matrix3.h"

// The binary fills this in the math/Matrix4.o static initializer at
// 0x1179FF0 (the original constructs it through Matrix4's non-constexpr
// constructor); the aggregate holds the same identity values.
const Hmx::Matrix4 Hmx::Matrix4::sID = {
    {1.0F, 0.0F, 0.0F, 0.0F},
    {0.0F, 1.0F, 0.0F, 0.0F},
    {0.0F, 0.0F, 1.0F, 0.0F},
    {0.0F, 0.0F, 0.0F, 1.0F},
};

// Reconstructed from eboot.elf at 0x11798A0. Expands along the first row
// through the 3x3 minors; the binary sums the even and the odd terms before
// subtracting them.
float Det(const Hmx::Matrix4& matrix) {
    const Vector4& x = matrix.x;
    const Vector4& y = matrix.y;
    const Vector4& z = matrix.z;
    const Vector4& w = matrix.w;

    Hmx::Matrix3 minor = {
        {y.y, y.z, y.w},
        {z.y, z.z, z.w},
        {w.y, w.z, w.w},
    };
    float det0 = Det(minor);
    minor = {
        {y.x, y.z, y.w},
        {z.x, z.z, z.w},
        {w.x, w.z, w.w},
    };
    float det1 = Det(minor);
    minor = {
        {y.x, y.y, y.w},
        {z.x, z.y, z.w},
        {w.x, w.y, w.w},
    };
    float det2 = Det(minor);
    minor = {
        {y.x, y.y, y.z},
        {z.x, z.y, z.z},
        {w.x, w.y, w.z},
    };
    float det3 = Det(minor);

    return (x.x * det0 + x.z * det2) - (x.y * det1 + x.w * det3);
}

// Reconstructed from eboot.elf at 0x1179A80. Every element is a 3x3
// cofactor written as three 2x2 minors times the remaining column, scaled by
// the inverse determinant. The binary computes all sixteen before storing,
// so the inverse may alias the input.
void Invert(
    const Hmx::Matrix4& matrix,
    Hmx::Matrix4& inverse,
    float epsilon) {
    float det = Det(matrix);
    float invDet = 0.0F;
    if (std::fabs(det) > epsilon) {
        invDet = 1.0F / det;
    }

    const float xx = matrix.x.x;
    const float xy = matrix.x.y;
    const float xz = matrix.x.z;
    const float xw = matrix.x.w;
    const float yx = matrix.y.x;
    const float yy = matrix.y.y;
    const float yz = matrix.y.z;
    const float yw = matrix.y.w;
    const float zx = matrix.z.x;
    const float zy = matrix.z.y;
    const float zz = matrix.z.z;
    const float zw = matrix.z.w;
    const float wx = matrix.w.x;
    const float wy = matrix.w.y;
    const float wz = matrix.w.z;
    const float ww = matrix.w.w;

    Hmx::Matrix4 result;
    result.x.x = invDet *
        (((yy * zz - zy * yz) * ww + (zw * yz - zz * yw) * wy) +
         (zy * yw - yy * zw) * wz);
    result.x.y = invDet *
        (((xz * zy - zz * xy) * ww + (xw * zz - xz * zw) * wy) +
         (xy * zw - xw * zy) * wz);
    result.x.z = invDet *
        (((xy * yz - xz * yy) * ww + (xz * yw - xw * yz) * wy) +
         (xw * yy - xy * yw) * wz);
    result.x.w = invDet *
        (((xz * yy - xy * yz) * zw + (xw * yz - xz * yw) * zy) +
         (xy * yw - xw * yy) * zz);
    result.y.x = invDet *
        (((zx * yz - yx * zz) * ww + (zz * yw - zw * yz) * wx) +
         (yx * zw - zx * yw) * wz);
    result.y.y = invDet *
        (((xx * zz - zx * xz) * ww + (xz * zw - xw * zz) * wx) +
         (zx * xw - zw * xx) * wz);
    result.y.z = invDet *
        (((yx * xz - xx * yz) * ww + (xw * yz - xz * yw) * wx) +
         (xx * yw - yx * xw) * wz);
    result.y.w = invDet *
        (((xx * yz - yx * xz) * zw + (xz * yw - xw * yz) * zx) +
         (yx * xw - xx * yw) * zz);
    result.z.x = (((yx * zy - zx * yy) * ww + wx * (yy * zw - zy * yw)) +
                  (zx * yw - yx * zw) * wy) *
        invDet;
    result.z.y = (((zx * xy - xx * zy) * ww + wx * (xw * zy - xy * zw)) +
                  (zw * xx - zx * xw) * wy) *
        invDet;
    result.z.z = (((xx * yy - yx * xy) * ww + wx * (xy * yw - xw * yy)) +
                  (yx * xw - xx * yw) * wy) *
        invDet;
    result.z.w = (((yx * xy - xx * yy) * zw + zx * (xw * yy - xy * yw)) +
                  (xx * yw - yx * xw) * zy) *
        invDet;
    result.w.x = (((zx * yy - yx * zy) * wz + wx * (zy * yz - yy * zz)) +
                  (yx * zz - zx * yz) * wy) *
        invDet;
    result.w.y = (((xx * zy - zx * xy) * wz + wx * (zz * xy - xz * zy)) +
                  (zx * xz - xx * zz) * wy) *
        invDet;
    result.w.z = (((yx * xy - xx * yy) * wz + wx * (xz * yy - xy * yz)) +
                  (xx * yz - yx * xz) * wy) *
        invDet;
    result.w.w = (((xx * yy - yx * xy) * zz + zx * (xy * yz - xz * yy)) +
                  (yx * xz - xx * yz) * zy) *
        invDet;
    inverse = result;
}
