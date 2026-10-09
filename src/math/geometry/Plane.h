#pragma once

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
