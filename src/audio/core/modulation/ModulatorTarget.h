#pragma once

#include <cstddef>

#include "audio/core/dsp/ParameterSpec.h"
#include "utl/containers/Vector.h"
#include "utl/text/Str.h"

// One value a property editor offers, with its label and help text. The
// template belongs to entity/PropMetadata.o; it is declared here until that
// header holds it. The map instantiates AllowedValue<unsigned char> for the
// modulation enums. Field names are not in the reference map.
template <class T>
struct AllowedValue {
    T mValue;
    String mName;
    String mDescription;
};

// A value a Modulator can drive (audio/ModulatorTarget.o, 0xBEBE0 to
// 0xBEF83): the float it adds to, and how a modulator's depth and range
// apply to it. FusionSampler's constructor (0x95C40) builds its start-point
// and pitch targets. The object is 40 bytes.
class ModulatorTarget {
public:
    // A modulator's "target" property. Value names are not in the reference
    // map.
    enum Target : unsigned char {
        kTargetNone = 0,
        kTargetStartPoint = 1,
        kTargetPitch = 2,
        kTargetInvalid = 3,  // StringToTarget's result for an unknown name.
    };
    // How a depth scales a normalized value, named by
    // GetStringForDepthMode. Value names are not in the reference map.
    enum DepthMode : int {
        kDepthModeAdditive = 0,
        kDepthModeSubtractive = 1,
        kDepthModeCentered = 2,
    };

    // The names TargetToString returns: "None", "Start Point" and "Pitch".
    static const char* kNoTargetStr;     // 0x19B01A8
    static const char* kStartPointStr;   // 0x19B01B0
    static const char* kPitchStr;        // 0x19B01B8
    // The target of a modulator with none. At 0x19B0180.
    static ModulatorTarget kDummyTarget;

    // The name, or "" for an unknown target. At 0xBEBE0.
    static const char* TargetToString(Target target);
    // kTargetInvalid for an unknown name. At 0xBEC20.
    static Target StringToTarget(const char* name);
    // The "target" property's values with their help. At 0xBEC90.
    static eastl::vector<AllowedValue<unsigned char>> GetAllowedTargetValues();
    // "Additive", "Subtractive" or "Centered"; "" otherwise. At 0xBEE90.
    static const char* GetStringForDepthMode(DepthMode mode);
    // "+", "-" or "±"; "" otherwise. At 0xBEEB0.
    static const char* GetMathSymbolForDepthMode(DepthMode mode);

    // At 0xBEED0.
    ModulatorTarget(
        const char* name, float defaultRangeMagnitude, const char* units, DepthMode depthMode, float* value);

    // The value scaled by the depth: the depth itself when additive, one
    // minus it when subtractive, and around one half when centered. At
    // 0xBEEF0.
    float ApplyDepthToNormalizedValue(float value, float depth) const;
    // The span a normalized value maps onto: [0, magnitude] when additive,
    // [-magnitude, 0] when subtractive and [-magnitude, magnitude] when
    // centered. At 0xBEF40.
    SPL::Range<float> GetRangeWithMagnitude(float magnitude) const;

    // Field names are not in the reference map.
    const char* mName;
    float mDefaultRangeMagnitude;  // The range a modulator takes on SetTarget.
    const char* mUnits;            // "cents" for the pitch target.
    float* mValue;                 // The sum the modulators add to.
    DepthMode mDepthMode;
};

static_assert(offsetof(ModulatorTarget, mDefaultRangeMagnitude) == 8);
static_assert(offsetof(ModulatorTarget, mUnits) == 16);
static_assert(offsetof(ModulatorTarget, mValue) == 24);
static_assert(offsetof(ModulatorTarget, mDepthMode) == 32);
static_assert(sizeof(ModulatorTarget) == 40);
