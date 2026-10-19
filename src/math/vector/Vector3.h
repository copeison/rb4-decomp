#pragma once

// Three-component engine vector. This build stores it unpadded in 12 bytes,
// as the FMOD attribute conversion at 0x27ACB0 reads it.
class Vector3 {
public:
    float x;
    float y;
    float z;

    static Vector3 sZero;  // 0x19E76F0

    // The unit axes.
    static Vector3 sX;  // 0x19B0340
    static Vector3 sY;  // 0x19B034C
    static Vector3 sZ;  // 0x19B0358
};

static_assert(sizeof(Vector3) == 12);

// The linear blend from `a` to `b` by `t` (the map's Interp). Inlined into
// its users, such as the vector keys' sampling (0x324D40).
inline void Interp(const Vector3& a, const Vector3& b, float t, Vector3& result) {
    result.x = (b.x - a.x) * t + a.x;
    result.y = (b.y - a.y) * t + a.y;
    result.z = (b.z - a.z) * t + a.z;
}
