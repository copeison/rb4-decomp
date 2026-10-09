#pragma once

// Two-component float vector.
class Vector2 {
public:
    float x;
    float y;

    static Vector2 sZero;  // 0x1B5D260
};

static_assert(sizeof(Vector2) == 8);
