#pragma once

// Three-component engine vector. This build stores it unpadded in 12 bytes,
// as the FMOD attribute conversion at 0x27ACB0 reads it.
class Vector3 {
public:
    float x;
    float y;
    float z;

    // The unit axes.
    static Vector3 sX;  // 0x19B0340
    static Vector3 sY;  // 0x19B034C
    static Vector3 sZ;  // 0x19B0358
};

static_assert(sizeof(Vector3) == 12);
