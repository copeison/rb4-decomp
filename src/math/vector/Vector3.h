#pragma once

// Three-component engine vector. This build stores it unpadded in 12 bytes,
// as the FMOD attribute conversion at 0x27ACB0 reads it.
class Vector3 {
public:
    float x;
    float y;
    float z;
};

static_assert(sizeof(Vector3) == 12);
