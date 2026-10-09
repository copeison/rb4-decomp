// The billboard transform (0x441E50 to 0x44239B), emitted among the
// RndUtl helpers.
#include "render/drawing/RndBillboard.h"

#include <cmath>

namespace {

// Positions closer than this on every axis count as the same.
constexpr float kSamePositionEpsilon = 1e-4F;

Vector3 Cross(const Vector3& a, const Vector3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

// A zero vector stays zero.
Vector3 Normalize(const Vector3& v) {
    const float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    const float scale = length != 0.0F ? 1.0F / length : 0.0F;
    return {scale * v.x, scale * v.y, scale * v.z};
}

}  // namespace

// Reconstructed from eboot.elf at 0x441E50. An unknown type keeps the
// object's X and Z rows and puts the unnormalized direction in Y.
Transform ComputeBillboardXfm(
    const Transform& xfm,
    const Transform& camera,
    int type,
    int axis) {
    if (type == kBillboardNone) {
        return xfm;
    }
    Vector3 toCamera;
    if (std::fabs(xfm.v.x - camera.v.x) > kSamePositionEpsilon ||
        std::fabs(xfm.v.y - camera.v.y) > kSamePositionEpsilon ||
        std::fabs(xfm.v.z - camera.v.z) > kSamePositionEpsilon) {
        toCamera = {camera.v.x - xfm.v.x, camera.v.y - xfm.v.y, camera.v.z - xfm.v.z};
    } else {
        toCamera = {-camera.m.y.x, -camera.m.y.y, -camera.m.y.z};
    }
    if (axis == kBillboardAxisNegY) {
        toCamera = {-toCamera.x, -toCamera.y, -toCamera.z};
    }

    Transform result;
    result.v = xfm.v;
    switch (type) {
    case kBillboardCamera:
    case kBillboardCameraNoRoll: {
        const Vector3& up = type == kBillboardCamera ? camera.m.z : Vector3::sZ;
        result.m.y = Normalize(toCamera);
        result.m.x = Normalize(Cross(result.m.y, up));
        result.m.z = Cross(result.m.x, result.m.y);
        break;
    }
    case kBillboardCameraXY:
        result.m.z = Normalize(Vector3::sZ);
        result.m.x = Normalize(Cross(toCamera, result.m.z));
        result.m.y = Cross(result.m.z, result.m.x);
        break;
    case kBillboardCameraKeepZ: {
        result.m.z = Normalize(xfm.m.z);
        const Vector3 side = Cross(toCamera, result.m.z);
        if (side.z == Vector3::sZero.z && side.x == Vector3::sZero.x &&
            side.y == Vector3::sZero.y) {
            return xfm;
        }
        result.m.x = Normalize(side);
        result.m.y = Cross(result.m.z, result.m.x);
        break;
    }
    default:
        result.m.x = xfm.m.x;
        result.m.y = toCamera;
        result.m.z = xfm.m.z;
        break;
    }
    return result;
}
