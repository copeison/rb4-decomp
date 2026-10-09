#pragma once

#include "math/vector/Vector3.h"

// Bounding sphere. Field names are not in the reference map.
class Sphere {
public:
    Vector3 center;
    float radius;
};

static_assert(sizeof(Sphere) == 16);
