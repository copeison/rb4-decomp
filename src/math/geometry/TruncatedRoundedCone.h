#pragma once

#include <cstddef>

// A cone whose apex is cut off by a flat top at z = 0 and whose base is a
// spherical cap, opening along -z. Field names are not in the reference map;
// the setter derives every field from the angle, top radius and length.
class TruncatedRoundedCone {
public:
    // Every field starts at -1.
    TruncatedRoundedCone();  // 0x117E340

    // Clamps the half-angle to [0, pi/2] and the top radius to what the
    // length allows. Matched to this map name by its arguments and size.
    void SetAngleTopRadiusAndLength(
        float angle,
        float topRadius,
        float length);  // 0x117E750

    float mAngle;         // Half-angle.
    float mTopRadius;     // Radius of the flat top at z = 0.
    float mBottomRadius;  // Radius where the side meets the cap.
    float mLength;        // Depth from the top to the bottom of the cap.
    float mConeLength;    // Depth from the top to the cap's rim.
    float mCapRadius;     // Radius of the spherical cap.
    float mCapRimHeight;  // mCapRadius * cos(mAngle).
};

static_assert(offsetof(TruncatedRoundedCone, mLength) == 12);
static_assert(offsetof(TruncatedRoundedCone, mCapRimHeight) == 24);
static_assert(sizeof(TruncatedRoundedCone) == 28);
