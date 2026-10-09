#pragma once

// Integer two-component vector. Header-only in the original; the map has no
// out-of-line members.
class Vector2i {
public:
    int x;
    int y;
};

static_assert(sizeof(Vector2i) == 8);
