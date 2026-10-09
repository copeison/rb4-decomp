#include "audio/core/modulation/Modulator.h"

namespace {

// The depth and range magnitude limits, at 0x19B0168 and 0x19B0174. Names
// not in the reference map.
SPL::ParameterSpec kDepthParamSpec = {0.0F, 0.5F, 1.0F};
SPL::ParameterSpec kRangeMagnitudeParamSpec = {0.0F, 1.0F, 1000000.0F};

}  // namespace

// Reconstructed from eboot.elf at 0xBE8D0. The range starts as [0, 1];
// SetTarget then takes the dummy target's magnitude, which the default
// magnitude replaces.
Modulator::Modulator()
    : mTarget(&ModulatorTarget::kDummyTarget),
      mDepth(kDepthParamSpec),
      mRangeMagnitude(kRangeMagnitudeParamSpec),
      mRange{0.0F, 1.0F} {
    SetTarget(nullptr);
    SetRangeMagnitude(mRangeMagnitude.mSpec.mDefault);
}

// Reconstructed from eboot.elf at 0xBE9D0.
void Modulator::SetTarget(const ModulatorTarget* target) {
    if (target == nullptr) {
        target = &ModulatorTarget::kDummyTarget;
    }
    mTarget = target;
    mRangeMagnitude.Set(target->mDefaultRangeMagnitude);
    mRange = mTarget->GetRangeWithMagnitude(mRangeMagnitude.Get());
}

// Reconstructed from eboot.elf at 0xBEA40.
void Modulator::SetRangeMagnitude(float magnitude) {
    mRangeMagnitude.Set(magnitude);
    mRange = mTarget->GetRangeWithMagnitude(mRangeMagnitude.Get());
}

// Reconstructed from eboot.elf at 0xBEAA0.
const ModulatorTarget* Modulator::GetTarget() const {
    return mTarget;
}

// Reconstructed from eboot.elf at 0xBEAB0.
float Modulator::GetRangeMagnitude() const {
    return mRangeMagnitude.Get();
}

// Reconstructed from eboot.elf at 0xBEAC0.
float Modulator::GetDepth() const {
    return mDepth.Get();
}

// Reconstructed from eboot.elf at 0xBEAD0.
void Modulator::SetDepth(float depth) {
    mDepth.Set(depth);
}

// Reconstructed from eboot.elf at 0xBEB00.
void Modulator::Modulate(float value) {
    if (mTarget == nullptr) {
        return;
    }
    float* sum = mTarget->mValue;
    float shaped = mTarget->ApplyDepthToNormalizedValue(value, mDepth.Get());
    *sum += shaped * mRange.mLength + mRange.mStart;
}

// Reconstructed from eboot.elf at 0xBEB40.
void Modulator::Reset() {
    mDepth.Set(mDepth.mSpec.mDefault);
    SetTarget(nullptr);
}
