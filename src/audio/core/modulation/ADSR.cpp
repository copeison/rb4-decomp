#include "audio/core/modulation/ADSR.h"

#include <cstring>

SPL::ParameterSpec kAttackTimeParamSpec = {0.0F, 0.0F, 10.0F};
SPL::ParameterSpec kDecayTimeParamSpec = {0.0F, 0.0F, 10.0F};
SPL::ParameterSpec kReleaseTimeParamSpec = {0.0F, 1.0F, 10.0F};
SPL::ParameterSpec kSustainLevelParamSpec = {0.0F, 1.0F, 1.0F};
SPL::ParameterSpec kDepthLevelParamSpec = {0.0F, 0.5F, 1.0F};

namespace {

// The table at 0x18E59C0. Name not in the reference map.
const char* const kTargetNames[] = {"none", "volume", "filter_freq"};

}  // namespace

// Reconstructed from eboot.elf at 0xBD740.
const char* ADSR::Settings::TargetToString(Target target) {
    if (target > kTargetFilterFreq) {
        return nullptr;
    }
    return kTargetNames[target];
}

// Reconstructed from eboot.elf at 0xBD760.
ADSR::Settings::Target ADSR::Settings::StringToTarget(const char* name) {
    if (std::strcmp(name, "none") == 0) {
        return kTargetNone;
    }
    if (std::strcmp(name, "volume") == 0) {
        return kTargetVolume;
    }
    if (std::strcmp(name, "filter_freq") == 0) {
        return kTargetFilterFreq;
    }
    return kTargetInvalid;
}

// Reconstructed from eboot.elf at 0xBD7C0.
eastl::vector<AllowedValue<unsigned char>> ADSR::Settings::GetVolumeAllowedTargetValues() {
    eastl::vector<AllowedValue<unsigned char>> values;
    values.emplace_back(
        AllowedValue<unsigned char>{kTargetVolume, String("Volume"), String("Target volume.")});
    return values;
}

// Reconstructed from eboot.elf at 0xBD8B0.
eastl::vector<AllowedValue<unsigned char>> ADSR::Settings::GetAssignableAllowedTargetValues() {
    eastl::vector<AllowedValue<unsigned char>> values;
    values.emplace_back(
        AllowedValue<unsigned char>{kTargetNone, String("None"), String("No target.")});
    values.emplace_back(AllowedValue<unsigned char>{
        kTargetFilterFreq, String("Filter Freq"), String("Target the filter frequency.")});
    return values;
}

// Reconstructed from eboot.elf at 0xBDA20.
void ADSR::State::Prepare(float sampleRate) {
    mSampleRate = sampleRate;
}

// Reconstructed from eboot.elf at 0xBDA30.
float ADSR::State::GetValue() const {
    float depth = mSettings->mEnabled ? mSettings->mDepth : 0.0f;
    switch (mDepthMode) {
    case 0:
        return depth * mLevel;
    case 1:
        return 1.0f + depth - depth * mLevel;
    case 2:
        return depth * mLevel + (1.0f - depth) * 0.5f;
    default:
        return mLevel;
    }
}

// Reconstructed from eboot.elf at 0xBDA90. The attack ends at the peak and
// the decay at the sustain level; the release stops the envelope at zero.
void ADSR::State::Advance(unsigned int numSamples) {
    switch (mStage) {
    case kStageAttack:
        mSamplesElapsed += numSamples;
        mLevel += numSamples * mAttackRate;
        if (mLevel >= 1.0f) {
            LeavePeak();
        }
        break;
    case kStageDecay:
        mSamplesElapsed += numSamples;
        mLevel -= numSamples * mDecayRate;
        if (mLevel <= mSettings->mSustain) {
            mLevel = mSettings->mSustain;
            mStage = kStageSustain;
        }
        break;
    case kStageSustain:
        mSamplesElapsed += numSamples;
        break;
    case kStageRelease:
        mSamplesElapsed += numSamples;
        mLevel -= numSamples * mReleaseRate;
        if (!(mLevel > 0.0f)) {
            Stop();
        }
        break;
    default:
        break;
    }
}
