// entity/Key.o (part): the float spline sampling and the interpolation
// upgrade.
#include "entity/waveform/Key.h"

#include <math.h>

namespace {

// The automatic tangent at the key: the slope from the key before it to the
// key after it (the key itself at either end), or 0 when they share a frame.
// Inlined twice into the spline sampling. The map has the Vector3 overload,
// SplineTangent(TKeyTrack<Vector3> const&, unsigned long, Vector3&).
float SplineTangent(const TKeyTrack<ValueTangent<float>>& track, unsigned long index) {
    const unsigned long before = index == 0 ? 0 : index - 1;
    const unsigned long after = index + (index < track.NumKeys() - 1 ? 1 : 0);
    if (track.GetKeyFrameAtIndex(after) == track.GetKeyFrameAtIndex(before)) {
        return 0.0f;
    }
    const TKey<ValueTangent<float>>* const keys = static_cast<const TKey<ValueTangent<float>>*>(track.mKeys.mData);
    const float rise = keys[after].mValue.mValue - keys[before].mValue.mValue;
    return rise / (track.GetKeyFrameAtIndex(after) - track.GetKeyFrameAtIndex(before));
}

}  // namespace

// Reconstructed from eboot.elf at 0x1DCA50.
template <>
void TKeyTrack<ValueTangent<float>>::_SampleSplineInterpValue(
    const TKey<ValueTangent<float>>* prev,
    const TKey<ValueTangent<float>>* next,
    float t,
    ValueTangent<float>& value) const {
    const float from = prev->mValue.mValue;
    const float to = next->mValue.mValue;
    if ((!prev->mValue.mAutoTangents && fabsf(prev->mValue.mOutTangent) > 500.0f) ||
        (!next->mValue.mAutoTangents && fabsf(next->mValue.mInTangent) > 500.0f)) {
        value.mValue = from;
        return;
    }
    const unsigned long index = prev - Keys();
    const float outTangent = prev->mValue.mAutoTangents ? SplineTangent(*this, index) : prev->mValue.mOutTangent;
    const float outSlope = (next->mFrame - prev->mFrame) * outTangent;
    const float inTangent = next->mValue.mAutoTangents ? SplineTangent(*this, index + 1) : next->mValue.mInTangent;
    const float inSlope = (next->mFrame - prev->mFrame) * inTangent;
    const float cubic = ((from - to) + (from - to) + outSlope + inSlope) * t;
    const float quadratic = outSlope * -2.0f - inSlope + cubic + (to - from) * 3.0f;
    value.mValue = ((quadratic * t + outSlope) * t) + from;
}

// Reconstructed from eboot.elf at 0x1DC930.
void UpgradeInterpolation(int oldInterpolation, KeyTrackBase::Interpolation* interpolation) {
    const unsigned int kept = static_cast<unsigned int>(*interpolation) & 0xDFFF0000U;
    unsigned int upgraded;
    switch (oldInterpolation) {
    case 0:
    case 1:
    case 2:
        upgraded = static_cast<unsigned int>(oldInterpolation) & 0xDFFFFFFFU;
        break;
    case 3:
        upgraded = KeyTrackBase::kInterpolationLinear;
        break;
    case 4:
        upgraded = KeyTrackBase::kInterpolationHermite;
        break;
    case 5:
        upgraded = KeyTrackBase::kInterpolationCubicIn;
        break;
    case 6:
        upgraded = KeyTrackBase::kInterpolationCubicOut;
        break;
    case 7:
        upgraded = KeyTrackBase::kInterpolationLinear | 0x20000000U;
        break;
    case 8:
        upgraded = KeyTrackBase::kInterpolationHermite | 0x20000000U;
        break;
    case 9:
    case 10:
        upgraded = static_cast<unsigned int>(oldInterpolation) | 0x20000000U;
        break;
    default:
        upgraded = 0xDFFFFFFFU;
        break;
    }
    *interpolation = static_cast<KeyTrackBase::Interpolation>(kept | upgraded);
}
