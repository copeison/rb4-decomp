#pragma once

#include "math/vector/Vector2.h"
#include "math/vector/Vector3.h"

// Line segment between two points. Field names are not in the reference
// map.
class Segment {
public:
    Vector3 start;
    Vector3 end;
};

static_assert(sizeof(Segment) == 24);

// Line segment in the plane. Field names are not in the reference map.
class Segment2D {
public:
    Vector2 start;
    Vector2 end;
};

static_assert(sizeof(Segment2D) == 16);
