#include "math/geometry/Frustum.h"

#include <cmath>

namespace {

// Plane slots and the corner pairs of the edges. Names not in the reference
// map.
enum FrustumPlane : unsigned long {
    kNearPlane = 0,
    kFarPlane = 1,
    kLeftPlane = 2,
    kRightPlane = 3,
    kTopPlane = 4,
    kBottomPlane = 5,
};

constexpr Frustum::Edge kEdges[12] = {
    {0, 2}, {1, 0}, {2, 3}, {3, 1},
    {4, 6}, {5, 4}, {6, 7}, {7, 5},
    {0, 4}, {1, 5}, {2, 6}, {3, 7},
};

// The near and far planes shared by both projections.
void SetDepthPlanes(Plane* planes, float nearPlane, float farPlane) {
    planes[kNearPlane].a = 0.0F;
    planes[kNearPlane].b = 1.0F;
    planes[kNearPlane].c = 0.0F;
    planes[kNearPlane].d = -nearPlane;
    planes[kFarPlane].a = 0.0F;
    planes[kFarPlane].b = -1.0F;
    planes[kFarPlane].c = 0.0F;
    planes[kFarPlane].d = farPlane;
}

void SetPlane(Plane& plane, float a, float b, float c, float d) {
    plane.a = a;
    plane.b = b;
    plane.c = c;
    plane.d = d;
}

// The plane through the origin corner and two others, facing along
// (a - origin) x (b - origin). The map's _DoSetPlaneFromCorners also takes
// the plane slot and a fourth corner; this build's inlined copies read three
// corners. Name not in the reference map.
void SetPlaneFromCorners(
    Plane& plane,
    const Vector3& origin,
    const Vector3& a,
    const Vector3& b) {
    const Vector3 edgeA = {a.x - origin.x, a.y - origin.y, a.z - origin.z};
    const Vector3 edgeB = {b.x - origin.x, b.y - origin.y, b.z - origin.z};
    const Vector3 normal = {
        edgeB.z * edgeA.y - edgeB.y * edgeA.z,
        edgeB.x * edgeA.z - edgeB.z * edgeA.x,
        edgeB.y * edgeA.x - edgeB.x * edgeA.y,
    };
    const float length =
        std::sqrt(normal.x * normal.x + normal.z * normal.z + normal.y * normal.y);
    float scale = 0.0F;
    if (length != 0.0F) {
        scale = 1.0F / length;
    }
    plane.a = scale * normal.x;
    plane.b = scale * normal.y;
    plane.c = scale * normal.z;
    plane.d = -(plane.b * origin.y + (plane.a * origin.x + plane.c * origin.z));
}

}  // namespace

// Reconstructed from eboot.elf at 0x1172F10.
void Frustum::SetPerspective(float nearPlane, float farPlane, const Fov& fov) {
    Plane* planes = mPlanes.mData;
    SetDepthPlanes(planes, nearPlane, farPlane);
    SetPlane(planes[kTopPlane], 0.0F, std::sin(fov.mUp), -std::cos(fov.mUp), 0.0F);
    SetPlane(planes[kBottomPlane], 0.0F, std::sin(fov.mDown), std::cos(fov.mDown), 0.0F);
    SetPlane(planes[kLeftPlane], std::cos(fov.mLeft), std::sin(fov.mLeft), 0.0F, 0.0F);
    SetPlane(planes[kRightPlane], -std::cos(fov.mRight), std::sin(fov.mRight), 0.0F, 0.0F);
    _DoUpdateHull();
}

// Reconstructed from eboot.elf at 0x1173280.
void Frustum::SetOrtho(float nearPlane, float farPlane, float height, float aspect) {
    Plane* planes = mPlanes.mData;
    SetDepthPlanes(planes, nearPlane, farPlane);
    const float halfHeight = height * 0.5F;
    SetPlane(planes[kTopPlane], 0.0F, 0.0F, -1.0F, halfHeight);
    SetPlane(planes[kBottomPlane], 0.0F, 0.0F, 1.0F, halfHeight);
    const float halfWidth = halfHeight * aspect;
    SetPlane(planes[kLeftPlane], 1.0F, 0.0F, 0.0F, halfWidth);
    SetPlane(planes[kRightPlane], -1.0F, 0.0F, 0.0F, halfWidth);
    _DoUpdateHull();
}

// Reconstructed from eboot.elf at 0x1173340. The corners only place the
// planes; _DoUpdateHull then rebuilds the corners from them.
void Frustum::SetCorners(const Vector3* corners) {
    Plane* planes = mPlanes.mData;
    SetPlaneFromCorners(planes[kNearPlane], corners[0], corners[1], corners[2]);
    SetPlaneFromCorners(planes[kFarPlane], corners[5], corners[4], corners[7]);
    SetPlaneFromCorners(planes[kLeftPlane], corners[4], corners[0], corners[6]);
    SetPlaneFromCorners(planes[kRightPlane], corners[1], corners[5], corners[3]);
    SetPlaneFromCorners(planes[kTopPlane], corners[4], corners[5], corners[0]);
    SetPlaneFromCorners(planes[kBottomPlane], corners[7], corners[6], corners[3]);
    _DoUpdateHull();
}

// Reconstructed from eboot.elf at 0x11730B0. Corners 0 to 3 lie on the near
// plane and 4 to 7 on the far plane; odd corners are on the right, and
// corners 2, 3, 6 and 7 on the bottom.
void Frustum::_DoUpdateHull() {
    mCorners.resize(8);
    const Plane* planes = mPlanes.mData;
    Vector3* corners = mCorners.mData;
    for (unsigned long i = 0; i < 8; ++i) {
        const Plane& depth = planes[(i & 4) != 0 ? kFarPlane : kNearPlane];
        const Plane& side = planes[(i & 1) != 0 ? kRightPlane : kLeftPlane];
        const Plane& cap = planes[(i & 2) != 0 ? kBottomPlane : kTopPlane];
        Intersect(depth, side, cap, corners[i]);
    }
    if (mEdges.mSize == 0) {
        mEdges.resize(12);
        for (unsigned long i = 0; i < 12; ++i) {
            mEdges.mData[i] = kEdges[i];
        }
    }
}
