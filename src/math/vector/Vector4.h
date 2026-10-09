#pragma once

// Four-component float vector.
class Vector4 {
public:
    float x;
    float y;
    float z;
    float w;

    static Vector4 sZero;  // 0x1B5D268
};

static_assert(sizeof(Vector4) == 16);
