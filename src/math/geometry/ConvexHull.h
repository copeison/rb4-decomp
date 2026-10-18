#pragma once

#include <cstddef>

#include "math/geometry/Plane.h"
#include "math/vector/Vector3.h"

class Sphere;

// The shared code of the convex hulls (math/ConvexHull.o). Each hull keeps
// its corners, edges and planes in fixed vectors and hands them to the
// base's tests as a ConvexHullInfo; the hull templates themselves are
// inlined. Only the sphere classification that the renderer calls is
// declared; ConvexHull.o is not reconstructed.
class ConvexHullBase {
public:
    // A pair of corner indices. Field names are not in the reference map.
    struct Edge {
        int mCorner0;
        int mCorner1;
    };

    // The hull's arrays, as the inlined templates copy them out of their
    // fixed vectors. Field names are not in the reference map.
    struct ConvexHullInfo {
        const Vector3* mCorners;
        unsigned long mNumCorners;
        const Edge* mEdges;
        unsigned long mNumEdges;
        const Plane* mPlanes;
        unsigned long mNumPlanes;
    };

    // The results of the classifications. Names not in the reference map.
    enum Classification : int {
        kInside = 0,
        kOutside = 1,
        kIntersects = 2,
    };

    // Inside when the sphere is behind no plane by less than its radius,
    // outside when it is in front of a plane by more than its radius;
    // otherwise the edges decide between outside and intersecting.
    Classification _ClassifySphereImpl(
        const Sphere& sphere,
        const ConvexHullInfo& info) const;  // 0x116E9A0
};

static_assert(sizeof(ConvexHullBase::Edge) == 8);
static_assert(sizeof(ConvexHullBase::ConvexHullInfo) == 48);
