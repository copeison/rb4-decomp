#pragma once

#include <cstddef>

#include "math/geometry/Plane.h"
#include "math/vector/Vector3.h"
#include "utl/containers/FixedVector.h"

// View volume: its eight corners, its edges and its six bounding planes. The
// constructor is inlined into every owner; it leaves the corner and edge
// lists empty and stores six default planes. Field names are not in the
// reference map.
class Frustum {
public:
    // A pair of corner indices. The binary copies the eight-byte entries
    // whole; the name and fields are not in the reference map.
    struct Edge {
        int mCorner0;
        int mCorner1;
    };

    // Half-angles of a perspective frustum's sides, in radians. The camera
    // context builds them as a vertical and a horizontal pair. Name and
    // field names not in the reference map.
    struct Fov {
        float mUp;
        float mDown;
        float mLeft;
        float mRight;
    };

    Frustum() { mPlanes.resize(6); }

    // The planes face inward in camera space, with y forward and z up:
    // near, far, left, right, top and bottom. Each setter rebuilds the hull.
    // The map has SetPerspective(float, float, float, float); this build
    // takes the side angles.
    void SetPerspective(float nearPlane, float farPlane, const Fov& fov);  // 0x1172F10
    // The height spans the top and bottom planes; the aspect ratio scales
    // it to the width.
    void SetOrtho(float nearPlane, float farPlane, float height, float aspect);  // 0x1173280
    // Takes the eight corners in _DoUpdateHull's order and derives the
    // planes.
    void SetCorners(const Vector3* corners);  // 0x1173340
    // Intersects the planes into the eight corners and, the first time,
    // fills the twelve edges.
    void _DoUpdateHull();  // 0x11730B0

    FixedVector<Vector3, 8> mCorners;
    FixedVector<Edge, 12> mEdges;
    FixedVector<Plane, 6> mPlanes;
};

static_assert(offsetof(Frustum, mEdges) == 120);
static_assert(offsetof(Frustum, mPlanes) == 240);
static_assert(sizeof(Frustum) == 360);
