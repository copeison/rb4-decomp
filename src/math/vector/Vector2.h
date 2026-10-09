#pragma once

// Two-component float vector.
class Vector2 {
public:
    float x;
    float y;

    static Vector2 sX;        // 0x19BEF48
    static Vector2 sY;        // 0x19BEF50
    static Vector2 sUnitAll;  // 0x19BEF58
    static Vector2 sZero;     // 0x1B5D260
};

static_assert(sizeof(Vector2) == 8);
