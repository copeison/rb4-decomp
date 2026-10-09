#pragma once

#include <cstddef>

#include "math/vector/Vector2.h"
#include "math/vector/Vector3.h"
#include "utl/text/Symbol.h"

class BinStream;
class DataArray;

// A curve from one input range onto an output range (math/Interp.o, 0x212590
// to 0x214928). The vtables have no RTTI; Interpolator's own is at 0x18EEFD8.
//
// Each map constructor takes two names, which this build ignores; the
// constructors here take nothing or the curve's parameters. Script arrays
// list the outputs before the inputs: (type startY endY startX endX ...).
class Interpolator {
public:
    virtual float Eval(float x) const = 0;  // slot 0: __cxa_pure_virtual
    // Slot 1 at 0x214850. Evaluates without clamping.
    virtual float ClampEval(float x) const {
        return Eval(x);
    }
    // Slot 2 at 0x2125B0.
    virtual float ReverseEval(float y) const;
    // Slot 3 at 0x214860. Reverses without clamping.
    virtual float ClampReverseEval(float y) const {
        return ReverseEval(y);
    }
    // Slot 4 at 0x214870.
    virtual void Reset(const DataArray*) {}
    // Slots 5-6: 0x212590, 0x2125A0.
    virtual ~Interpolator();

    // A curve with default parameters for the type: linear, exp, invexp,
    // atan or cubic. Other types, piecewiselinear among them, give null. At
    // 0x2125C0.
    static Interpolator* New(Symbol type);
    // A curve of the array's type, reset from the array. At 0x212970.
    static Interpolator* New(const DataArray* data);
};

// A straight line through two points. The vtable is at 0x18EEE28; the object
// is 32 bytes.
class LinearInterpolator : public Interpolator {
public:
    // From (0, 0) to (1, 1). At 0x2129B0.
    LinearInterpolator();
    // Overload not in the reference map. At 0x2129E0.
    LinearInterpolator(const Vector2& start, const Vector2& end);

    float Eval(float x) const override;              // slot 0: 0x212BE0
    float ClampEval(float x) const override;         // slot 1: 0x212BF0
    float ReverseEval(float y) const override;       // slot 2: 0x212C20
    float ClampReverseEval(float y) const override;  // slot 3: 0x212C40
    void Reset(const DataArray* data) override;      // slot 4: 0x212B10
    // Slots 5-6: 0x214880, 0x214890.
    ~LinearInterpolator() override {}

    void Reset(const Vector2& start, const Vector2& end);  // 0x212A60
    // Recomputes the slope and offset from the end points. At 0x212AC0.
    void Sync();
    void Save(BinStream& stream);  // 0x212C70
    void Load(BinStream& stream);  // 0x212D10

    // Field names are not in the reference map.
    Vector2 mStart;
    Vector2 mEnd;
    float mSlope;   // Zero for an empty x span.
    float mOffset;
};

static_assert(offsetof(LinearInterpolator, mStart) == 8);
static_assert(offsetof(LinearInterpolator, mSlope) == 24);
static_assert(offsetof(LinearInterpolator, mOffset) == 28);
static_assert(sizeof(LinearInterpolator) == 32);

// An exponential curve from mStart to mEnd: the normalized x raised to the
// exponent. The vtable is at 0x18EEE70; the object is 32 bytes.
class ExpInterpolator : public Interpolator {
public:
    // From (0, 0) to (1, 1) with exponent 2. At 0x212DC0.
    ExpInterpolator();
    // Overload not in the reference map. At 0x212E30.
    ExpInterpolator(const Vector2& start, const Vector2& end, float exponent);

    float Eval(float x) const override;              // slot 0: 0x213000
    float ClampEval(float x) const override;         // slot 1: 0x213090
    float ReverseEval(float y) const override;       // slot 2: 0x213040
    float ClampReverseEval(float y) const override;  // slot 3: 0x2130C0
    void Reset(const DataArray* data) override;      // slot 4: 0x212F00
    // Slots 5-6: 0x95C30 (audio/FusionSampler.o's copy), 0x2148A0.
    ~ExpInterpolator() override {}

    // Takes the end points and the exponent. At 0x212EA0.
    void Reset(const Vector2& start, const Vector2& end, float exponent);
    // Recomputes mInvRange. At 0x212DF0.
    void Sync();
    void Save(BinStream& stream);  // 0x2130F0
    void Load(BinStream& stream);  // 0x2131A0

    // Field names are not in the reference map.
    Vector2 mStart;
    Vector2 mEnd;
    float mExponent;
    float mInvRange;  // One over the x span, or one for an empty span.
};

static_assert(offsetof(ExpInterpolator, mStart) == 8);
static_assert(offsetof(ExpInterpolator, mExponent) == 24);
static_assert(sizeof(ExpInterpolator) == 32);

// The exponential curve mirrored through the middle: one minus the remaining
// normalized x raised to the exponent. The vtable is at 0x18EEEB8; the object
// is 32 bytes.
class InvExpInterpolator : public Interpolator {
public:
    // From (0, 0) to (1, 1) with exponent 2. At 0x213240.
    InvExpInterpolator();
    // Overload not in the reference map. At 0x2132B0.
    InvExpInterpolator(const Vector2& start, const Vector2& end, float exponent);

    float Eval(float x) const override;              // slot 0: 0x213480
    float ClampEval(float x) const override;         // slot 1: 0x213540
    float ReverseEval(float y) const override;       // slot 2: 0x2134E0
    float ClampReverseEval(float y) const override;  // slot 3: 0x213570
    void Reset(const DataArray* data) override;      // slot 4: 0x213380
    // Slots 5-6: 0x2148B0, 0x2148C0.
    ~InvExpInterpolator() override {}

    void Reset(const Vector2& start, const Vector2& end, float exponent);  // 0x213320
    // Recomputes mInvRange. At 0x213270.
    void Sync();
    void Save(BinStream& stream);  // 0x2135A0
    void Load(BinStream& stream);  // 0x213650

    // Field names are not in the reference map.
    Vector2 mStart;
    Vector2 mEnd;
    float mExponent;
    float mInvRange;  // One over the x span, or one for an empty span.
};

static_assert(offsetof(InvExpInterpolator, mExponent) == 24);
static_assert(offsetof(InvExpInterpolator, mInvRange) == 28);
static_assert(sizeof(InvExpInterpolator) == 32);

// A curve that is linear in log space: y plus the bias grows exponentially
// with x. Eval and ReverseEval clamp to the end points. New cannot make one.
// The vtable is at 0x18EEF00; the object is 48 bytes.
class LogInterpolator : public Interpolator {
public:
    // Zero end points and bias; the logarithms are left unset. At 0x2136F0.
    LogInterpolator();
    // Overload not in the reference map. At 0x213710.
    LogInterpolator(const Vector2& start, const Vector2& end, float bias);

    float Eval(float x) const override;              // slot 0: 0x2138E0
    float ClampEval(float x) const override;         // slot 1: 0x213A20
    float ReverseEval(float y) const override;       // slot 2: 0x213990
    float ClampReverseEval(float y) const override;  // slot 3: 0x213A30
    void Reset(const DataArray* data) override;      // slot 4: 0x2137E0
    // Slots 5-6: 0x2148D0, 0x2148E0.
    ~LogInterpolator() override {}

    void Reset(const Vector2& start, const Vector2& end, float bias);  // 0x213780
    // The bias that takes the curve through `mid` halfway along x, for the
    // output range from `start` to `end`. At 0x213A40.
    static float CalculateBiasForMid(float start, float mid, float end);

    // Field names are not in the reference map.
    Vector2 mStart;
    Vector2 mEnd;
    float mBias;       // Added to y before the logarithm.
    double mLogStart;  // log(mBias)
    double mLogEnd;    // log(mEnd.y - mStart.y + mBias)
};

static_assert(offsetof(LogInterpolator, mBias) == 24);
static_assert(offsetof(LogInterpolator, mLogStart) == 32);
static_assert(offsetof(LogInterpolator, mLogEnd) == 40);
static_assert(sizeof(LogInterpolator) == 48);

// An arctangent S-curve from mStart to mEnd, steeper for a larger sharpness.
// It has no reverse. The vtable is at 0x18EEF48; the object is 48 bytes.
class ATanInterpolator : public Interpolator {
public:
    // From (0, 0) to (1, 1) with sharpness 2. At 0x213A60.
    ATanInterpolator();
    // Overload not in the reference map. At 0x213B30.
    ATanInterpolator(const Vector2& start, const Vector2& end, float sharpness);

    float Eval(float x) const override;          // slot 0: 0x213E20
    // Slots 1-3: Interpolator's.
    void Reset(const DataArray* data) override;  // slot 4: 0x213CD0
    // Slots 5-6: 0x2148F0, 0x214900.
    ~ATanInterpolator() override {}

    void Reset(const Vector2& start, const Vector2& end, float sharpness);  // 0x213C10
    // Recomputes the scales and offsets. At 0x213A90.
    void Sync();
    void Save(BinStream& stream);  // 0x213E50
    void Load(BinStream& stream);  // 0x213F00

    // Field names are not in the reference map.
    Vector2 mStart;
    Vector2 mEnd;
    // The atan argument runs from -mSharpness to mSharpness over the x span.
    float mSharpness;
    float mXScale;
    float mXOffset;
    float mYScale;
    float mYOffset;
};

static_assert(offsetof(ATanInterpolator, mSharpness) == 24);
static_assert(offsetof(ATanInterpolator, mYOffset) == 40);
static_assert(sizeof(ATanInterpolator) == 48);

// An odd cubic response with a dead zone, as for an analog stick: zero up to
// mDeadZone, then a cubic in the distance past it that reaches mMax and keeps
// the input's sign. The vtable is at 0x18EEF90; the object is 48 bytes.
class CubicInterpolator : public Interpolator {
public:
    // A dead zone of zero, a knee at one and a zero maximum. At 0x213FF0.
    CubicInterpolator();
    // Overload not in the reference map. At 0x2141A0.
    CubicInterpolator(
        float deadZone, float knee, float kneeSlope, const Vector2& max, float maxSlope);

    float Eval(float x) const override;          // slot 0: 0x214590
    float ClampEval(float x) const override;     // slot 1: 0x2145E0
    // Slots 2-3: Interpolator's.
    void Reset(const DataArray* data) override;  // slot 4: 0x2143E0
    // Slots 5-6: 0x214910, 0x214920.
    ~CubicInterpolator() override {}

    void Reset(
        float deadZone,
        float knee,
        float kneeSlope,
        const Vector2& max,
        float maxSlope);  // 0x2142D0
    // Solves for mCoeffs. At 0x2140A0.
    void Sync();
    void Save(BinStream& stream);  // 0x214610
    void Load(BinStream& stream);  // 0x2146E0

    // Field names are not in the reference map, and the evidence for them is
    // the solve in Sync alone: the cubic p(t) in t = |x| - mDeadZone has
    // p(0) = 0 and p(mMax.x - mDeadZone) = mMax.y, and its slope at the knee
    // and at mMax.x is mKneeSlope and mMaxSlope times its slope at zero.
    float mDeadZone;
    float mKnee;  // An x, raised to at least mDeadZone.
    float mKneeSlope;
    Vector2 mMax;  // mMax.x is raised to at least mDeadZone.
    float mMaxSlope;
    Vector3 mCoeffs;  // The t^3, t^2 and t coefficients.
};

static_assert(offsetof(CubicInterpolator, mKneeSlope) == 16);
static_assert(offsetof(CubicInterpolator, mMax) == 20);
static_assert(offsetof(CubicInterpolator, mMaxSlope) == 28);
static_assert(offsetof(CubicInterpolator, mCoeffs) == 32);
static_assert(sizeof(CubicInterpolator) == 48);
