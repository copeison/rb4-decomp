// The capsule's constructor and setter (0x117E050 to 0x117E2E6).
#include "math/geometry/Capsule.h"

#include <cmath>

// Reconstructed from eboot.elf at 0x117E050.
Capsule::Capsule()
    : mStart{Vector3::sZero, 0.0F},
      mEnd{Vector3::sZero, 0.0F},
      mLength(0.0F),
      mAxis(Vector3::sZero),
      mSideSine(0.0F),
      mSideCosine(0.0F) {}

// Reconstructed from eboot.elf at 0x117E1E0. The axis divides by the length
// without a zero check.
void Capsule::Set(const Vector3& start, const Vector3& end, float startRadius, float endRadius) {
    mStart.center = start;
    mStart.radius = startRadius;
    mEnd.center = end;
    mEnd.radius = endRadius;
    const Vector3 delta = {start.x - end.x, start.y - end.y, start.z - end.z};
    const float lengthSq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
    mLength = lengthSq != 0.0F ? std::sqrt(lengthSq) : 0.0F;
    const float inverseLength = 1.0F / std::sqrt(lengthSq);
    mAxis = {delta.x * inverseLength, delta.y * inverseLength, delta.z * inverseLength};
    const float radiusChange = endRadius - startRadius;
    const float inverseSide = 1.0F / std::sqrt(lengthSq + radiusChange * radiusChange);
    mSideSine = radiusChange * inverseSide;
    mSideCosine = mLength * inverseSide;
}
