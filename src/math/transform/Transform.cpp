#include "math/transform/Transform.h"

// Both are filled by the math/Transform.o static initializer at 0x219080
// (the original constructs them through Transform's non-constexpr
// constructors); the aggregates hold the same values.
const Transform Transform::sZero = {};
const Transform Transform::sID = {
    {
        {1.0F, 0.0F, 0.0F},
        {0.0F, 1.0F, 0.0F},
        {0.0F, 0.0F, 1.0F},
    },
    {0.0F, 0.0F, 0.0F},
};

// Reconstructed from eboot.elf at 0x2187E0. Row-vector convention: applies a
// and then b. Both parts are computed before they are stored, so the result
// may alias either input.
void Multiply(const Transform& a, const Transform& b, Transform& result) {
    Vector3 v;
    v.x = a.v.x * b.m.x.x + a.v.y * b.m.y.x + a.v.z * b.m.z.x + b.v.x;
    v.y = a.v.x * b.m.x.y + a.v.y * b.m.y.y + a.v.z * b.m.z.y + b.v.y;
    v.z = a.v.x * b.m.x.z + a.v.y * b.m.y.z + a.v.z * b.m.z.z + b.v.z;
    result.v = v;

    Hmx::Matrix3 m;
    m.x.x = a.m.x.x * b.m.x.x + a.m.x.y * b.m.y.x + a.m.x.z * b.m.z.x;
    m.x.y = a.m.x.x * b.m.x.y + a.m.x.y * b.m.y.y + a.m.x.z * b.m.z.y;
    m.x.z = a.m.x.x * b.m.x.z + a.m.x.y * b.m.y.z + a.m.x.z * b.m.z.z;
    m.y.x = a.m.y.x * b.m.x.x + a.m.y.y * b.m.y.x + a.m.y.z * b.m.z.x;
    m.y.y = a.m.y.x * b.m.x.y + a.m.y.y * b.m.y.y + a.m.y.z * b.m.z.y;
    m.y.z = a.m.y.x * b.m.x.z + a.m.y.y * b.m.y.z + a.m.y.z * b.m.z.z;
    m.z.x = a.m.z.x * b.m.x.x + a.m.z.y * b.m.y.x + a.m.z.z * b.m.z.x;
    m.z.y = a.m.z.x * b.m.x.y + a.m.z.y * b.m.y.y + a.m.z.z * b.m.z.y;
    m.z.z = a.m.z.x * b.m.x.z + a.m.z.y * b.m.y.z + a.m.z.z * b.m.z.z;
    result.m = m;
}
