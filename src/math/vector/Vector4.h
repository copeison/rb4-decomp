#pragma once

// Four-component float vector.
class Vector4 {
public:
    float x;
    float y;
    float z;
    float w;

    static Vector4 sZero;
};

static_assert(sizeof(Vector4) == 16);
