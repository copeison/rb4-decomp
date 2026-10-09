#pragma once

#include <cstddef>

#include "math/vector/Vector3.h"

class Sphere;

// A convex bound made of 26 slabs: for each direction whose components are
// 0, 1 or -1, the farthest extent of the contained points. Direction i has
// the base-3 digits of i as components, with the digits 0, 1 and 2 standing
// for 0, 1 and -1; direction 0 is unused. Field names are not in the
// reference map.
class BoundingHull {
public:
    // Extents start below any real distance. Name not in the reference map.
    static constexpr float kEmptyExtent = -1.0e30F;
    // The most corner points GeneratePoints returns. Name not in the
    // reference map.
    static constexpr unsigned long kMaxPoints = 49;

    BoundingHull();  // 0x1173CC0

    // Inlined into GeneratePoints at 0x11743ED; the map's out-of-line copy is
    // not located in this build.
    bool IsValid() const { return mExtents[1] != kEmptyExtent; }

    // Pushes each slab out to the points.
    void GrowToContain(const Vector3* points, unsigned long numPoints);  // 0x1173D20
    // Intersects the slab planes three at a time and keeps the distinct
    // intersections inside every slab, up to kMaxPoints. Returns how many it
    // wrote.
    unsigned long GeneratePoints(Vector3* points) const;  // 0x11743C0
    // Centers the sphere on the average hull corner, sizes it to reach every
    // corner, then lets OptimizeSphere shrink it. An empty hull gives a zero
    // sphere at the origin.
    void GenerateSphere(Sphere& sphere) const;  // 0x1174B80
    // Searches a shrinking box around the center for a smaller sphere that
    // still holds the points, and keeps it when it beats the given radius.
    void OptimizeSphere(
        Sphere& sphere,
        const Vector3* points,
        unsigned long numPoints) const;  // 0x1175080

    float mExtents[27];
};

static_assert(sizeof(BoundingHull) == 108);
