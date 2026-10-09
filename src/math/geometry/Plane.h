#pragma once

class Segment;
class Transform;
class Vector3;

// Plane ax + by + cz + d = 0. Field names are not in the reference map.
class Plane {
public:
    // A default plane faces +z through the origin, as Frustum's inlined
    // constructors store it.
    Plane() : a(0.0F), b(0.0F), c(1.0F), d(0.0F) {}

    float a;
    float b;
    float c;
    float d;
};

static_assert(sizeof(Plane) == 16);

// Moves the plane by the transform: the normal by the inverse rotation, and a
// point of the plane by the whole transform. A zero normal keeps the origin.
void Multiply(const Plane& plane, const Transform& xfm, Plane& result);  // 0x117A1D0
// The line where two planes meet, as two of its points.
void Intersect(const Plane& a, const Plane& b, Segment& line);  // 0x117A4D0
// The point where three planes meet.
void Intersect(
    const Plane& a,
    const Plane& b,
    const Plane& c,
    Vector3& point);  // 0x117A3D0
