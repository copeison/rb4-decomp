#include "math/geometry/Plane.h"

#include "math/geometry/Segment.h"
#include "math/matrix/Matrix3.h"
#include "math/transform/Transform.h"

// Reconstructed from eboot.elf at 0x117A1D0. The point on the plane nearest
// the origin is -d * n / |n|^2.
void Multiply(const Plane& plane, const Transform& xfm, Plane& result) {
    Hmx::Matrix3 inverse = Hmx::Matrix3::sID;
    Invert(xfm.m, inverse, nullptr);

    const float a = plane.a;
    const float b = plane.b;
    const float c = plane.c;
    const float na = a * inverse.x.x + b * inverse.x.y + c * inverse.x.z;
    const float nb = a * inverse.y.x + b * inverse.y.y + c * inverse.y.z;
    const float nc = a * inverse.z.x + b * inverse.z.y + c * inverse.z.z;

    const float lengthSquared = a * a + b * b + c * c;
    float scale = 0.0F;
    if (lengthSquared != 0.0F) {
        scale = -plane.d / lengthSquared;
    }
    const float px = scale * a;
    const float py = scale * b;
    const float pz = scale * c;
    const Hmx::Matrix3& m = xfm.m;
    const float x = px * m.x.x + py * m.y.x + (pz * m.z.x + xfm.v.x);
    const float y = px * m.x.y + py * m.y.y + (pz * m.z.y + xfm.v.y);
    const float z = px * m.x.z + py * m.y.z + (pz * m.z.z + xfm.v.z);

    result.a = na;
    result.b = nb;
    result.c = nc;
    result.d = -(x * na + y * nb + z * nc);
}

// Reconstructed from eboot.elf at 0x117A4D0. The line runs along the cross
// product of the normals. Its start is found by walking from the point of the
// first plane nearest the origin, across that plane, until the second plane;
// the binary inlines that segment-plane test, which leaves the parameter
// unset when the walk runs parallel to the second plane.
void Intersect(const Plane& a, const Plane& b, Segment& line) {
    const Vector3 direction = {
        b.c * a.b - b.b * a.c,
        b.a * a.c - a.a * b.c,
        a.a * b.b - b.a * a.b,
    };
    const Vector3 across = {
        direction.y * a.c - direction.z * a.b,
        direction.z * a.a - direction.x * a.c,
        direction.x * a.b - direction.y * a.a,
    };
    const float lengthSquared = a.b * a.b + a.a * a.a + a.c * a.c;
    float scale = 0.0F;
    if (lengthSquared != 0.0F) {
        scale = -a.d / lengthSquared;
    }
    line.start = {scale * a.a, scale * a.b, scale * a.c};
    line.end = {
        line.start.x + across.x, line.start.y + across.y, line.start.z + across.z};

    const Vector3& start = line.start;
    const Vector3& end = line.end;
    const float startDistance = start.y * b.b + start.x * b.a + start.z * b.c;
    const float endDistance = end.y * b.b + end.x * b.a + end.z * b.c;
    const float denominator = startDistance - endDistance;
    float t = startDistance;
    if (denominator != 0.0F) {
        t = (startDistance + b.d) / denominator;
    }
    const Vector3 point = {
        t * across.x + start.x,
        start.y + t * across.y,
        start.z + t * across.z,
    };
    line.start = point;
    line.end = {point.x + direction.x, point.y + direction.y, point.z + direction.z};
}

// Reconstructed from eboot.elf at 0x117A3D0. Intersects the line of the first
// two planes with the third; the binary inlines the segment-plane test, which
// leaves the parameter unset when the line runs parallel to the plane.
void Intersect(const Plane& a, const Plane& b, const Plane& c, Vector3& point) {
    Segment line = {};
    Intersect(a, b, line);

    const Vector3& start = line.start;
    const Vector3& end = line.end;
    const float startDistance = start.x * c.a + start.y * c.b + start.z * c.c;
    const float endDistance = end.x * c.a + end.y * c.b + end.z * c.c;
    const float denominator = startDistance - endDistance;
    float t = startDistance;
    if (denominator != 0.0F) {
        t = (startDistance + c.d) / denominator;
    }
    point.x = start.x + (end.x - start.x) * t;
    point.y = start.y + (end.y - start.y) * t;
    point.z = start.z + (end.z - start.z) * t;
}
