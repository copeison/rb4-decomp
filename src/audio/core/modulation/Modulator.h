#pragma once

#include <cstddef>

#include "audio/core/dsp/ParameterSpec.h"
#include "audio/core/modulation/ModulatorTarget.h"

// Adds a scaled control value to a ModulatorTarget (audio/Modulator.o,
// 0xBE8D0 to 0xBEBD2). FusionSampler keeps two vectors of them for the
// patch's velocity "modulators". The object is 80 bytes.
class Modulator {
public:
    // One element of a Fusion patch's "modulators" array. Field names follow
    // the "target", "range" and "depth" properties.
    struct Settings {
        ModulatorTarget::Target mTarget;
        float mRange;  // The range magnitude, in the target's units.
        float mDepth;
    };

    // Aims at kDummyTarget with the default range magnitude. At 0xBE8D0.
    Modulator();

    // Aims at the target, or at kDummyTarget for null, taking the target's
    // default range magnitude. At 0xBE9D0.
    void SetTarget(const ModulatorTarget* target);
    // Clamps the magnitude to [0, 1000000] and recomputes the range. At
    // 0xBEA40.
    void SetRangeMagnitude(float magnitude);
    const ModulatorTarget* GetTarget() const;  // 0xBEAA0
    float GetRangeMagnitude() const;           // 0xBEAB0
    float GetDepth() const;                    // 0xBEAC0
    // Clamps the depth to [0, 1]. At 0xBEAD0.
    void SetDepth(float depth);
    // Adds the target's depth-shaped value, mapped onto the range, to the
    // target's value. At 0xBEB00.
    void Modulate(float value);
    // Restores the default depth and aims at kDummyTarget. At 0xBEB40.
    void Reset();

    // Field names are not in the reference map.
    const ModulatorTarget* mTarget;
    SPL::Parameter mDepth;
    SPL::Parameter mRangeMagnitude;
    // The target's GetRangeWithMagnitude for the current magnitude.
    SPL::Range<float> mRange;
};

static_assert(offsetof(Modulator::Settings, mRange) == 4);
static_assert(sizeof(Modulator::Settings) == 12);
static_assert(offsetof(Modulator, mDepth) == 8);
static_assert(offsetof(Modulator, mRangeMagnitude) == 40);
static_assert(offsetof(Modulator, mRange) == 72);
static_assert(sizeof(Modulator) == 80);
