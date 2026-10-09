#pragma once

#include "math/vector/Vector3.h"

class Transform;

// Bounding sphere. Field names are not in the reference map.
class Sphere {
public:
    // Encloses the box between the two corners: centered between them,
    // with half the diagonal as the radius. RndParticleCom::
    // _UpdateBoundingSphere calls it. Name not in the reference map.
    void SetFromBox(const Vector3& min, const Vector3& max);  // 0x117D0A0
    // Grows the sphere to enclose the other; an empty (zero-radius) other
    // leaves it, and an empty sphere becomes the other.
    void GrowToContain(const Sphere& other);  // 0x117D500

    // The zero sphere, zeroed by Sphere.o's initializer at 0x117DCA0.
    static const Sphere sZero;  // 0x1B5D250

    Vector3 center;
    float radius;
};

static_assert(sizeof(Sphere) == 16);

// Moves the sphere by the transform: the center by the whole transform, the
// radius scaled by the largest axis length. RndMeshCom::_PollSkinnedSphere
// calls it.
void Multiply(const Sphere& sphere, const Transform& xfm, Sphere& result);  // 0x117D670
// The same with the radius scaled by the shortest axis length, taken as 1
// within 0.0001 of it. RndDrawNodeCom moves world spheres into its object's
// space with it. Name not in the reference map.
Sphere MultiplyMinScale(const Sphere& sphere, const Transform& xfm);  // 0x117D7B0
