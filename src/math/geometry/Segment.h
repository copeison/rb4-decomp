#pragma once

#include "math/vector/Vector3.h"

// Line segment between two points. Field names are not in the reference
// map.
class Segment {
public:
    Vector3 start;
    Vector3 end;
};

static_assert(sizeof(Segment) == 24);
