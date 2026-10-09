#include "math/geometry/Geo.h"

#include <cfloat>
#include <cmath>

#include "math/geometry/Sphere.h"
#include "math/matrix/Matrix3.h"

namespace {

// The component of each base-3 digit, and the digit of each negated
// component, indexed by the component plus one. Initialized data at
// 0x19BEF24 and 0x19BEF30. Names not in the reference map.
const float kDigitComponents[3] = {0.0F, 1.0F, -1.0F};
const unsigned long kComponentDigits[3] = {2, 0, 1};

constexpr unsigned long kNumDirections = 27;
constexpr float kPointMergeScale = 0.001F;
constexpr int kOptimizePasses = 10;

Vector3 Direction(unsigned long index) {
    return {
        kDigitComponents[index % 3],
        kDigitComponents[index / 3 % 3],
        kDigitComponents[index / 9 % 3],
    };
}

// The index of the direction pointing the other way.
unsigned long OppositeDirection(unsigned long index) {
    const Vector3 direction = Direction(index);
    return kComponentDigits[static_cast<int>(-direction.x) + 1] +
        kComponentDigits[static_cast<int>(-direction.y) + 1] * 3 +
        kComponentDigits[static_cast<int>(-direction.z) + 1] * 9;
}

float Dot(const Vector3& a, const Vector3& b) {
    return a.y * b.y + a.x * b.x + a.z * b.z;
}

float DistanceSquared(const Vector3& a, const Vector3& b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dz = a.z - b.z;
    return dy * dy + dx * dx + dz * dz;
}

float Max(float a, float b) {
    return a > b ? a : b;
}

}  // namespace

// Reconstructed from eboot.elf at 0x1173CC0.
BoundingHull::BoundingHull() {
    for (unsigned long i = 0; i < kNumDirections; ++i) {
        mExtents[i] = kEmptyExtent;
    }
}

// Reconstructed from eboot.elf at 0x1173D20.
void BoundingHull::GrowToContain(const Vector3* points, unsigned long numPoints) {
    if (numPoints == 0) {
        return;
    }
    for (unsigned long i = 1; i < kNumDirections; ++i) {
        const Vector3 direction = Direction(i);
        for (unsigned long p = 0; p < numPoints; ++p) {
            mExtents[i] = Max(Dot(points[p], direction), mExtents[i]);
        }
    }
}

// Reconstructed from eboot.elf at 0x11743C0. Only triples of directions that
// lean the same way meet at a corner. Corners closer than a thousandth of the
// hull's widest span on every axis are merged.
unsigned long BoundingHull::GeneratePoints(Vector3* points) const {
    if (!IsValid()) {
        return 0;
    }

    Vector3 candidates[140] = {};
    unsigned long numCandidates = 0;
    for (unsigned long i = 1; i < 25; ++i) {
        const Vector3 a = Direction(i);
        for (unsigned long j = i + 1; j < 26; ++j) {
            const Vector3 b = Direction(j);
            if (!(Dot(b, a) > 0.0F)) {
                continue;
            }
            for (unsigned long k = j + 1; k < kNumDirections; ++k) {
                const Vector3 c = Direction(k);
                if (!(Dot(c, a) > 0.0F) || !(Dot(c, b) > 0.0F)) {
                    continue;
                }
                // The point whose distances along a, b and c are the three
                // extents.
                Hmx::Matrix3 directions = {
                    {a.x, b.x, c.x},
                    {a.y, b.y, c.y},
                    {a.z, b.z, c.z},
                };
                Invert(directions, directions, nullptr);
                const float ea = mExtents[i];
                const float eb = mExtents[j];
                const float ec = mExtents[k];
                Vector3& point = candidates[numCandidates++];
                point.x = eb * directions.y.x + ea * directions.x.x + ec * directions.z.x;
                point.y = eb * directions.y.y + ea * directions.x.y + ec * directions.z.y;
                point.z = eb * directions.y.z + ea * directions.x.z + ec * directions.z.z;
            }
        }
    }

    float span = 0.0F;
    for (unsigned long i = 1; i < kNumDirections; ++i) {
        span = Max(mExtents[OppositeDirection(i)] + mExtents[i], span);
    }
    if (numCandidates == 0) {
        return 0;
    }

    const float epsilon = span * kPointMergeScale;
    for (unsigned long i = 0; i + 1 < numCandidates; ++i) {
        const Vector3& point = candidates[i];
        unsigned long j = i + 1;
        while (j < numCandidates) {
            const Vector3& other = candidates[j];
            if (std::fabs(point.x - other.x) > epsilon || std::fabs(point.y - other.y) > epsilon ||
                std::fabs(point.z - other.z) > epsilon) {
                ++j;
                continue;
            }
            candidates[j] = candidates[--numCandidates];
        }
    }
    if (numCandidates == 0) {
        return 0;
    }

    unsigned long numPoints = 0;
    for (unsigned long i = 0; i < numCandidates; ++i) {
        const Vector3& point = candidates[i];
        bool inside = true;
        for (unsigned long d = 1; d < kNumDirections; ++d) {
            if (Dot(point, Direction(d)) > epsilon + mExtents[d]) {
                inside = false;
                break;
            }
        }
        if (!inside) {
            continue;
        }
        if (numPoints == kMaxPoints) {
            return kMaxPoints;
        }
        points[numPoints++] = point;
    }
    return numPoints;
}

// Reconstructed from eboot.elf at 0x1174B80.
void BoundingHull::GenerateSphere(Sphere& sphere) const {
    sphere.radius = 0.0F;
    Vector3 points[kMaxPoints] = {};
    const unsigned long numPoints = GeneratePoints(points);
    if (numPoints == 0) {
        sphere.center = Vector3::sZero;
        return;
    }

    Vector3 sum = points[0];
    for (unsigned long i = 1; i < numPoints; ++i) {
        sum.x += points[i].x;
        sum.y += points[i].y;
        sum.z += points[i].z;
    }
    const float count = static_cast<float>(numPoints);
    sphere.center = {sum.x / count, sum.y / count, sum.z / count};

    float maxDistanceSquared = sphere.radius;
    for (unsigned long i = 0; i < numPoints; ++i) {
        maxDistanceSquared = Max(DistanceSquared(points[i], sphere.center), maxDistanceSquared);
    }
    sphere.radius = std::sqrt(maxDistanceSquared);
    OptimizeSphere(sphere, points, numPoints);
}

// Reconstructed from eboot.elf at 0x1175080. Each pass tries the centers of
// the box's eight octants, keeps the best, and recenters a box of half the
// size on it.
void BoundingHull::OptimizeSphere(
    Sphere& sphere,
    const Vector3* points,
    unsigned long numPoints) const {
    const float radius = sphere.radius;
    Vector3 boxMin = {
        sphere.center.x - radius, sphere.center.y - radius, sphere.center.z - radius};
    Vector3 boxMax = {
        sphere.center.x + radius, sphere.center.y + radius, sphere.center.z + radius};
    Vector3 bestCenter = sphere.center;
    float bestRadius = radius;

    for (int pass = 0; pass < kOptimizePasses; ++pass) {
        const Vector3 size = {boxMax.x - boxMin.x, boxMax.y - boxMin.y, boxMax.z - boxMin.z};
        const Vector3 step = {size.x * 0.25F, size.y * 0.25F, size.z * 0.25F};
        Vector3 passCenter = {step.x + boxMin.x, step.y + boxMin.y, step.z + boxMin.z};
        float passRadius = FLT_MAX;
        for (unsigned long octant = 0; octant < 8; ++octant) {
            const Vector3 center = {
                (static_cast<float>(octant >> 2) * 0.5F + 0.25F) * size.x + boxMin.x,
                (static_cast<float>((octant >> 1) & 1) * 0.5F + 0.25F) * size.y + boxMin.y,
                (static_cast<float>(octant & 1) * 0.5F + 0.25F) * size.z + boxMin.z,
            };
            float maxDistanceSquared = 0.0F;
            for (unsigned long i = 0; i < numPoints; ++i) {
                maxDistanceSquared =
                    Max(DistanceSquared(points[i], center), maxDistanceSquared);
            }
            const float octantRadius = std::sqrt(maxDistanceSquared);
            if (octantRadius < passRadius) {
                passRadius = octantRadius;
                passCenter = center;
                if (octantRadius < bestRadius) {
                    bestRadius = octantRadius;
                    bestCenter = center;
                }
            }
        }
        boxMin = {passCenter.x - step.x, passCenter.y - step.y, passCenter.z - step.z};
        boxMax = {passCenter.x + step.x, passCenter.y + step.y, passCenter.z + step.z};
    }

    if (bestRadius < radius) {
        sphere.center = bestCenter;
        sphere.radius = bestRadius;
    }
}
