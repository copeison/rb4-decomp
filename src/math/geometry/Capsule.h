#pragma once

#include <cstddef>

#include "math/geometry/Sphere.h"
#include "math/vector/Vector3.h"

class Frustum;

// The hull of two spheres of different radii: a cone frustum with rounded
// ends. The name follows the map's Capsule.o, whose frustum tests take a
// Capsule; the binary's methods here are not in the reference map, and the
// evidence for matching them to the class is weak. RndLightSpotCom bounds
// its light volume with one. Field names are not in the reference map.
class Capsule {
public:
    // Zeroes every field.
    Capsule();  // 0x117E050

    // Stores the two spheres and derives the axis, its length and the slope
    // of the side. Name not in the reference map.
    void Set(const Vector3& start, const Vector3& end, float startRadius, float endRadius);  // 0x117E1E0

    Sphere mStart;
    Sphere mEnd;
    // The distance between the centers and the unit axis from the end to
    // the start.
    float mLength;
    Vector3 mAxis;
    // The side's slope: the radius change and the length over their
    // hypotenuse.
    float mSideSine;
    float mSideCosine;
};

static_assert(offsetof(Capsule, mEnd) == 16);
static_assert(offsetof(Capsule, mLength) == 32);
static_assert(offsetof(Capsule, mAxis) == 36);
static_assert(offsetof(Capsule, mSideCosine) == 52);
static_assert(sizeof(Capsule) == 56);
