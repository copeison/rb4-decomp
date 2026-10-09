#include "math/geometry/TruncatedRoundedCone.h"

#include "math/scalar/Trig.h"

namespace {

constexpr float kHalfPi = 1.5707964F;

}  // namespace

// Reconstructed from eboot.elf at 0x117E340.
TruncatedRoundedCone::TruncatedRoundedCone()
    : mAngle(-1.0F),
      mTopRadius(-1.0F),
      mBottomRadius(-1.0F),
      mLength(-1.0F),
      mConeLength(-1.0F),
      mCapRadius(-1.0F),
      mCapRimHeight(-1.0F) {}

// Reconstructed from eboot.elf at 0x117E750. The cap is a sphere around the
// cone's apex; mCapRadius is the apex-to-cap distance. mBottomRadius is
// stored unclamped and then raised to at least the top radius, as in the
// binary.
void TruncatedRoundedCone::SetAngleTopRadiusAndLength(
    float angle,
    float topRadius,
    float length) {
    mLength = 0.0F > length ? 0.0F : length;

    float clampedAngle = 0.0F > angle ? 0.0F : angle;
    mAngle = kHalfPi < angle ? kHalfPi : clampedAngle;

    float cosine = Sine(mAngle + kHalfPi);
    cosine = cosine > 0.0F ? cosine : 0.0F;
    float sine = Sine(mAngle);
    sine = 0.0F > sine ? 0.0F : sine;

    if (0.0F >= mAngle) {
        mTopRadius = 0.0F;
        mCapRadius = mLength;
    } else {
        // The widest top that still leaves room for the cap.
        float maxTopRadius = mLength * sine / (1.0F - cosine);
        float clampedTopRadius = 0.0F > topRadius ? 0.0F : topRadius;
        mTopRadius =
            maxTopRadius < topRadius ? maxTopRadius : clampedTopRadius;
        if (mTopRadius > 0.0F) {
            // The apex lies mTopRadius / tan(mAngle) above the top.
            float angleSine = Sine(mAngle);
            mCapRadius =
                Sine(mAngle + kHalfPi) * mTopRadius / angleSine + mLength;
        } else {
            mCapRadius = mLength;
        }
    }

    mCapRimHeight = mCapRadius * cosine;
    mBottomRadius = mCapRadius * sine;
    mBottomRadius = mBottomRadius > mTopRadius ? mBottomRadius : mTopRadius;
    float coneLength = mCapRimHeight + (mLength - mCapRadius);
    mConeLength = 0.0F > coneLength ? 0.0F : coneLength;
}
