#include "audio/core/modulation/ModulatorTarget.h"

#include <cstring>

const char* ModulatorTarget::kNoTargetStr = "None";
const char* ModulatorTarget::kStartPointStr = "Start Point";
const char* ModulatorTarget::kPitchStr = "Pitch";

namespace {

// What kDummyTarget adds to, at 0x19C9080. Name not in the reference map.
float sDummyTargetValue;

// The tables at 0x18E5A30 and 0x18E5A50. Names not in the reference map.
const char* const kDepthModeNames[] = {"Additive", "Subtractive", "Centered"};
const char* const kDepthModeSymbols[] = {"+", "-", "\xC2\xB1"};

}  // namespace

ModulatorTarget ModulatorTarget::kDummyTarget("None", 0.0F, "", kDepthModeAdditive, &sDummyTargetValue);

// Reconstructed from eboot.elf at 0xBEBE0.
const char* ModulatorTarget::TargetToString(Target target) {
    switch (target) {
    case kTargetNone:
        return kNoTargetStr;
    case kTargetStartPoint:
        return kStartPointStr;
    case kTargetPitch:
        return kPitchStr;
    default:
        return "";
    }
}

// Reconstructed from eboot.elf at 0xBEC20.
ModulatorTarget::Target ModulatorTarget::StringToTarget(const char* name) {
    if (std::strcmp(name, kNoTargetStr) == 0) {
        return kTargetNone;
    }
    if (std::strcmp(name, kStartPointStr) == 0) {
        return kTargetStartPoint;
    }
    if (std::strcmp(name, kPitchStr) == 0) {
        return kTargetPitch;
    }
    return kTargetInvalid;
}

// Reconstructed from eboot.elf at 0xBEC90.
eastl::vector<AllowedValue<unsigned char>> ModulatorTarget::GetAllowedTargetValues() {
    eastl::vector<AllowedValue<unsigned char>> values;
    values.emplace_back(
        AllowedValue<unsigned char>{kTargetNone, String("None"), String("No target.")});
    values.emplace_back(AllowedValue<unsigned char>{
        kTargetStartPoint, String("Start Point"), String("Randomize start point.")});
    values.emplace_back(
        AllowedValue<unsigned char>{kTargetPitch, String("Pitch"), String("Randomize pitch.")});
    return values;
}

// Reconstructed from eboot.elf at 0xBEE90.
const char* ModulatorTarget::GetStringForDepthMode(DepthMode mode) {
    if (static_cast<unsigned int>(mode) > kDepthModeCentered) {
        return "";
    }
    return kDepthModeNames[mode];
}

// Reconstructed from eboot.elf at 0xBEEB0.
const char* ModulatorTarget::GetMathSymbolForDepthMode(DepthMode mode) {
    if (static_cast<unsigned int>(mode) > kDepthModeCentered) {
        return "";
    }
    return kDepthModeSymbols[mode];
}

// Reconstructed from eboot.elf at 0xBEED0.
ModulatorTarget::ModulatorTarget(
    const char* name, float defaultRangeMagnitude, const char* units, DepthMode depthMode, float* value)
    : mName(name),
      mDefaultRangeMagnitude(defaultRangeMagnitude),
      mUnits(units),
      mValue(value),
      mDepthMode(depthMode) {}

// Reconstructed from eboot.elf at 0xBEEF0.
float ModulatorTarget::ApplyDepthToNormalizedValue(float value, float depth) const {
    switch (mDepthMode) {
    case kDepthModeAdditive:
        return depth * value;
    case kDepthModeSubtractive:
        return 1.0F - depth * value;
    case kDepthModeCentered:
        return depth * value + (1.0F - depth) * 0.5F;
    default:
        return value;
    }
}

// Reconstructed from eboot.elf at 0xBEF40.
SPL::Range<float> ModulatorTarget::GetRangeWithMagnitude(float magnitude) const {
    switch (mDepthMode) {
    case kDepthModeAdditive:
        return {0.0F, magnitude};
    case kDepthModeSubtractive:
        return {-magnitude, magnitude};
    case kDepthModeCentered:
        return {-magnitude, magnitude + magnitude};
    default:
        return {0.0F, 1.0F};
    }
}
