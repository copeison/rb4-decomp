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

    Frustum() { mPlanes.resize(6); }

    FixedVector<Vector3, 8> mCorners;
    FixedVector<Edge, 12> mEdges;
    FixedVector<Plane, 6> mPlanes;
};

static_assert(offsetof(Frustum, mEdges) == 120);
static_assert(offsetof(Frustum, mPlanes) == 240);
static_assert(sizeof(Frustum) == 360);
