#pragma once

// Integer three-component vector. Header-only in the original; the map has
// no out-of-line members.
class Vector3i {
public:
    int x;
    int y;
    int z;
};

static_assert(sizeof(Vector3i) == 12);
