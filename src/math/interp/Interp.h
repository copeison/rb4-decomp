#pragma once

#include <cstddef>

#include "math/vector/Vector2.h"

class DataArray;

// A curve between two points (math/Interp.o). The out-of-line members have
// not been reconstructed; they are declared so that the users link.
class Interpolator {
public:
    virtual float Eval(float x) const = 0;              // slot 0
    virtual float ClampEval(float x) const = 0;         // slot 1
    virtual float ReverseEval(float y) const;           // slot 2
    virtual float ClampReverseEval(float y) const = 0;  // slot 3
    virtual void Reset(const DataArray* data) = 0;      // slot 4
    // Slots 5-6. Empty and inline: audio/FusionSampler.o emits the copy at
    // 0x95C30 for its static interpolators.
    virtual ~Interpolator() {}
};

// An exponential curve from mStart to mEnd. The vtable is at 0x18EEE70; the
// object is 32 bytes.
class ExpInterpolator : public Interpolator {
public:
    // The map's constructor takes two names, which this build ignores. At
    // 0x212DC0.
    ExpInterpolator();

    float Eval(float x) const override;              // slot 0: 0x213000
    float ClampEval(float x) const override;         // slot 1: 0x213090
    float ReverseEval(float y) const override;       // slot 2: 0x213040
    float ClampReverseEval(float y) const override;  // slot 3: 0x2130C0
    void Reset(const DataArray* data) override;      // slot 4: 0x212F00
    // Slots 5-6: inline, see Interpolator.
    ~ExpInterpolator() override {}

    // Takes the end points and the exponent. At 0x212EA0.
    void Reset(const Vector2& start, const Vector2& end, float exponent);

    // Field names are not in the reference map.
    Vector2 mStart;
    Vector2 mEnd;
    float mExponent;
    float mInvRange;  // One over the x span, or one for an empty span.
};

static_assert(offsetof(ExpInterpolator, mStart) == 8);
static_assert(offsetof(ExpInterpolator, mExponent) == 24);
static_assert(sizeof(ExpInterpolator) == 32);
