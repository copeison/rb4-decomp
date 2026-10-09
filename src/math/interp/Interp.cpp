#include "math/interp/Interp.h"

#include <math.h>

#include "math/matrix/Matrix3.h"
#include "utl/data/DataArray.h"
#include "utl/streams/BinStream.h"

namespace {

// Below this x span (0x358637BD) a curve treats its range as empty. Name not
// in the reference map.
const float kEmptySpan = 1e-06F;

// The engine's comparison templates; their header has not been
// reconstructed. Names not in the reference map.
template <class T>
inline T Min(T x, T y) {
    return x > y ? y : x;
}

template <class T>
inline T Max(T x, T y) {
    return x < y ? y : x;
}

template <class T>
inline T Clamp(T min, T max, T value) {
    if (value > max) {
        return max;
    }
    return value < min ? min : value;
}

// BinStream's float insertion, inlined in the binary. Name not in the
// reference map.
inline void WriteFloat(BinStream& stream, float value) {
    stream.WriteEndian(&value, sizeof(value));
}

}  // namespace

// Reconstructed from eboot.elf at 0x212590. The deleting destructor is at
// 0x2125A0.
Interpolator::~Interpolator() {}

// Reconstructed from eboot.elf at 0x2125B0. The map's copy is 0x7F bytes;
// this build only returns zero.
float Interpolator::ReverseEval(float) const {
    return 0.0F;
}

// Reconstructed from eboot.elf at 0x2125C0. The symbols are function
// statics at 0x19E6580 to 0x19E65D0; piecewiselinear is initialized but never
// matched.
Interpolator* Interpolator::New(Symbol type) {
    static Symbol sLinear("");
    if (sLinear == Symbol("")) {
        sLinear = Symbol("linear");
    }
    static Symbol sExp("");
    if (sExp == Symbol("")) {
        sExp = Symbol("exp");
    }
    static Symbol sInvExp("");
    if (sInvExp == Symbol("")) {
        sInvExp = Symbol("invexp");
    }
    static Symbol sATan("");
    if (sATan == Symbol("")) {
        sATan = Symbol("atan");
    }
    static Symbol sPiecewiseLinear("");
    if (sPiecewiseLinear == Symbol("")) {
        sPiecewiseLinear = Symbol("piecewiselinear");
    }
    static Symbol sCubic("");
    if (sCubic == Symbol("")) {
        sCubic = Symbol("cubic");
    }

    if (type == sLinear) {
        return new LinearInterpolator();
    }
    if (type == sExp) {
        return new ExpInterpolator();
    }
    if (type == sInvExp) {
        return new InvExpInterpolator();
    }
    if (type == sATan) {
        return new ATanInterpolator();
    }
    if (type == sCubic) {
        return new CubicInterpolator();
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x212970. The type must be known.
Interpolator* Interpolator::New(const DataArray* data) {
    Interpolator* interp = New(data->Sym(0));
    interp->Reset(data);
    return interp;
}

// Reconstructed from eboot.elf at 0x2129B0.
LinearInterpolator::LinearInterpolator() : mStart{0.0F, 0.0F}, mEnd{1.0F, 1.0F} {
    Sync();
}

// Reconstructed from eboot.elf at 0x2129E0.
LinearInterpolator::LinearInterpolator(const Vector2& start, const Vector2& end)
    : mStart(), mEnd() {
    Reset(start, end);
}

// Reconstructed from eboot.elf at 0x212A60.
void LinearInterpolator::Reset(const Vector2& start, const Vector2& end) {
    mStart = start;
    mEnd = end;
    Sync();
}

// Reconstructed from eboot.elf at 0x212AC0.
void LinearInterpolator::Sync() {
    const float span = mEnd.x - mStart.x;
    mSlope = 0.0F;
    if (fabsf(span) >= kEmptySpan) {
        mSlope = (mEnd.y - mStart.y) / span;
    }
    mOffset = mStart.y - mSlope * mStart.x;
}

// Reconstructed from eboot.elf at 0x212B10.
void LinearInterpolator::Reset(const DataArray* data) {
    Reset(Vector2{data->Float(3), data->Float(1)}, Vector2{data->Float(4), data->Float(2)});
}

// Reconstructed from eboot.elf at 0x212BE0.
float LinearInterpolator::Eval(float x) const {
    return x * mSlope + mOffset;
}

// Reconstructed from eboot.elf at 0x212BF0.
float LinearInterpolator::ClampEval(float x) const {
    return Eval(Clamp(Min(mStart.x, mEnd.x), Max(mStart.x, mEnd.x), x));
}

// Reconstructed from eboot.elf at 0x212C20. A flat line gives zero.
float LinearInterpolator::ReverseEval(float y) const {
    if (mSlope == 0.0F) {
        return 0.0F;
    }
    return (y - mOffset) / mSlope;
}

// Reconstructed from eboot.elf at 0x212C40.
float LinearInterpolator::ClampReverseEval(float y) const {
    return ReverseEval(Clamp(Min(mStart.y, mEnd.y), Max(mStart.y, mEnd.y), y));
}

// Reconstructed from eboot.elf at 0x212C70.
void LinearInterpolator::Save(BinStream& stream) {
    WriteFloat(stream, mStart.x);
    WriteFloat(stream, mStart.y);
    WriteFloat(stream, mEnd.x);
    WriteFloat(stream, mEnd.y);
}

// Reconstructed from eboot.elf at 0x212D10.
void LinearInterpolator::Load(BinStream& stream) {
    stream.ReadEndian(&mStart.x, sizeof(float));
    stream.ReadEndian(&mStart.y, sizeof(float));
    stream.ReadEndian(&mEnd.x, sizeof(float));
    stream.ReadEndian(&mEnd.y, sizeof(float));
    Sync();
}

// Reconstructed from eboot.elf at 0x212DC0.
ExpInterpolator::ExpInterpolator() : mStart{0.0F, 0.0F}, mEnd{1.0F, 1.0F}, mExponent(2.0F) {
    Sync();
}

// Reconstructed from eboot.elf at 0x212DF0.
void ExpInterpolator::Sync() {
    const float span = mEnd.x - mStart.x;
    mInvRange = fabsf(span) >= kEmptySpan ? 1.0F / span : 1.0F;
}

// Reconstructed from eboot.elf at 0x212E30.
ExpInterpolator::ExpInterpolator(const Vector2& start, const Vector2& end, float exponent)
    : mStart(), mEnd() {
    Reset(start, end, exponent);
}

// Reconstructed from eboot.elf at 0x212EA0.
void ExpInterpolator::Reset(const Vector2& start, const Vector2& end, float exponent) {
    mStart = start;
    mEnd = end;
    mExponent = exponent;
    Sync();
}

// Reconstructed from eboot.elf at 0x212F00. The exponent defaults to 2.
void ExpInterpolator::Reset(const DataArray* data) {
    Reset(
        Vector2{data->Float(3), data->Float(1)},
        Vector2{data->Float(4), data->Float(2)},
        data->Size() > 5 ? data->Float(5) : 2.0F);
}

// Reconstructed from eboot.elf at 0x213000.
float ExpInterpolator::Eval(float x) const {
    return (mEnd.y - mStart.y) * powf((x - mStart.x) * mInvRange, mExponent) + mStart.y;
}

// Reconstructed from eboot.elf at 0x213040.
float ExpInterpolator::ReverseEval(float y) const {
    return powf((y - mStart.y) / (mEnd.y - mStart.y), 1.0F / mExponent) / mInvRange +
        mStart.x;
}

// Reconstructed from eboot.elf at 0x213090.
float ExpInterpolator::ClampEval(float x) const {
    return Eval(Clamp(Min(mStart.x, mEnd.x), Max(mStart.x, mEnd.x), x));
}

// Reconstructed from eboot.elf at 0x2130C0.
float ExpInterpolator::ClampReverseEval(float y) const {
    return ReverseEval(Clamp(Min(mStart.y, mEnd.y), Max(mStart.y, mEnd.y), y));
}

// Reconstructed from eboot.elf at 0x2130F0.
void ExpInterpolator::Save(BinStream& stream) {
    WriteFloat(stream, mStart.x);
    WriteFloat(stream, mStart.y);
    WriteFloat(stream, mEnd.x);
    WriteFloat(stream, mEnd.y);
    WriteFloat(stream, mExponent);
}

// Reconstructed from eboot.elf at 0x2131A0.
void ExpInterpolator::Load(BinStream& stream) {
    stream.ReadEndian(&mStart.x, sizeof(float));
    stream.ReadEndian(&mStart.y, sizeof(float));
    stream.ReadEndian(&mEnd.x, sizeof(float));
    stream.ReadEndian(&mEnd.y, sizeof(float));
    stream.ReadEndian(&mExponent, sizeof(float));
    Sync();
}

// Reconstructed from eboot.elf at 0x213240.
InvExpInterpolator::InvExpInterpolator()
    : mStart{0.0F, 0.0F}, mEnd{1.0F, 1.0F}, mExponent(2.0F) {
    Sync();
}

// Reconstructed from eboot.elf at 0x213270.
void InvExpInterpolator::Sync() {
    const float span = mEnd.x - mStart.x;
    mInvRange = fabsf(span) >= kEmptySpan ? 1.0F / span : 1.0F;
}

// Reconstructed from eboot.elf at 0x2132B0.
InvExpInterpolator::InvExpInterpolator(
    const Vector2& start, const Vector2& end, float exponent)
    : mStart(), mEnd() {
    Reset(start, end, exponent);
}

// Reconstructed from eboot.elf at 0x213320.
void InvExpInterpolator::Reset(const Vector2& start, const Vector2& end, float exponent) {
    mStart = start;
    mEnd = end;
    mExponent = exponent;
    Sync();
}

// Reconstructed from eboot.elf at 0x213380. The exponent defaults to 2.
void InvExpInterpolator::Reset(const DataArray* data) {
    Reset(
        Vector2{data->Float(3), data->Float(1)},
        Vector2{data->Float(4), data->Float(2)},
        data->Size() > 5 ? data->Float(5) : 2.0F);
}

// Reconstructed from eboot.elf at 0x213480. The binary forms one minus the
// normalized x as (mStart.x - x) * mInvRange + 1.
float InvExpInterpolator::Eval(float x) const {
    const float power = powf((mStart.x - x) * mInvRange + 1.0F, mExponent);
    return (mEnd.y - mStart.y) * (1.0F - power) + mStart.y;
}

// Reconstructed from eboot.elf at 0x2134E0.
float InvExpInterpolator::ReverseEval(float y) const {
    const float remaining = (mStart.y - y) / (mEnd.y - mStart.y) + 1.0F;
    return (1.0F - powf(remaining, 1.0F / mExponent)) / mInvRange + mStart.x;
}

// Reconstructed from eboot.elf at 0x213540.
float InvExpInterpolator::ClampEval(float x) const {
    return Eval(Clamp(Min(mStart.x, mEnd.x), Max(mStart.x, mEnd.x), x));
}

// Reconstructed from eboot.elf at 0x213570.
float InvExpInterpolator::ClampReverseEval(float y) const {
    return ReverseEval(Clamp(Min(mStart.y, mEnd.y), Max(mStart.y, mEnd.y), y));
}

// Reconstructed from eboot.elf at 0x2135A0.
void InvExpInterpolator::Save(BinStream& stream) {
    WriteFloat(stream, mStart.x);
    WriteFloat(stream, mStart.y);
    WriteFloat(stream, mEnd.x);
    WriteFloat(stream, mEnd.y);
    WriteFloat(stream, mExponent);
}

// Reconstructed from eboot.elf at 0x213650.
void InvExpInterpolator::Load(BinStream& stream) {
    stream.ReadEndian(&mStart.x, sizeof(float));
    stream.ReadEndian(&mStart.y, sizeof(float));
    stream.ReadEndian(&mEnd.x, sizeof(float));
    stream.ReadEndian(&mEnd.y, sizeof(float));
    stream.ReadEndian(&mExponent, sizeof(float));
    Sync();
}

// Reconstructed from eboot.elf at 0x2136F0.
LogInterpolator::LogInterpolator() : mStart(), mEnd(), mBias(0.0F) {}

// Reconstructed from eboot.elf at 0x213710.
LogInterpolator::LogInterpolator(const Vector2& start, const Vector2& end, float bias)
    : mStart(), mEnd() {
    Reset(start, end, bias);
}

// Reconstructed from eboot.elf at 0x213780. The logarithms are single
// precision, stored as doubles.
void LogInterpolator::Reset(const Vector2& start, const Vector2& end, float bias) {
    mStart = start;
    mEnd = end;
    mBias = bias;
    mLogStart = logf(mBias);
    mLogEnd = logf(mEnd.y - mStart.y + mBias);
}

// Reconstructed from eboot.elf at 0x2137E0. The bias defaults to 1.
void LogInterpolator::Reset(const DataArray* data) {
    Reset(
        Vector2{data->Float(3), data->Float(1)},
        Vector2{data->Float(4), data->Float(2)},
        data->Size() > 5 ? data->Float(5) : 1.0F);
}

// Reconstructed from eboot.elf at 0x2138E0. The exponential and the bias
// correction are double precision.
float LogInterpolator::Eval(float x) const {
    if (x <= mStart.x) {
        return mStart.y;
    }
    if (x >= mEnd.x) {
        return mEnd.y;
    }
    const float t = mEnd.x == mStart.x ? 1.0F : (x - mStart.x) / (mEnd.x - mStart.x);
    const double logY = Clamp(0.0F, 1.0F, t) * (mLogEnd - mLogStart) + mLogStart;
    return mStart.y + exp(logY) - mBias;
}

// Reconstructed from eboot.elf at 0x213990.
float LogInterpolator::ReverseEval(float y) const {
    if (y <= mStart.y) {
        return mStart.x;
    }
    if (y >= mEnd.y) {
        return mEnd.x;
    }
    const float logY = logf(y - mStart.y + mBias);
    const float t =
        mLogEnd == mLogStart ? 1.0F : (logY - mLogStart) / (mLogEnd - mLogStart);
    return (mEnd.x - mStart.x) * t + mStart.x;
}

// Reconstructed from eboot.elf at 0x213A20. Eval already clamps.
float LogInterpolator::ClampEval(float x) const {
    return Eval(x);
}

// Reconstructed from eboot.elf at 0x213A30. ReverseEval already clamps.
float LogInterpolator::ClampReverseEval(float y) const {
    return ReverseEval(y);
}

// Reconstructed from eboot.elf at 0x213A40. The map's copy is 0x12B bytes.
float LogInterpolator::CalculateBiasForMid(float start, float mid, float end) {
    const float rise = mid - start;
    return rise * rise / ((end - start) - 2.0F * rise);
}

// Reconstructed from eboot.elf at 0x213A60. The binary stores the synced
// values as constants.
ATanInterpolator::ATanInterpolator()
    : mStart{0.0F, 0.0F}, mEnd{1.0F, 1.0F}, mSharpness(2.0F) {
    Sync();
}

// Reconstructed from eboot.elf at 0x213A90. The argument runs from
// -mSharpness at mStart.x to mSharpness at mEnd.x, and the result is scaled
// so that it runs from mStart.y to mEnd.y.
void ATanInterpolator::Sync() {
    const float span = mEnd.x - mStart.x;
    mXScale = 0.0F;
    if (fabsf(span) >= kEmptySpan) {
        mXScale = mSharpness * 2.0F / span;
    }
    mXOffset = -(mStart.x * mXScale) - mSharpness;
    const float rise = mEnd.y - mStart.y;
    mYScale = rise / (atanf(-mSharpness) * -2.0F);
    mYOffset = rise * 0.5F + mStart.y;
}

// Reconstructed from eboot.elf at 0x213B30.
ATanInterpolator::ATanInterpolator(const Vector2& start, const Vector2& end, float sharpness)
    : mStart(), mEnd() {
    Reset(start, end, sharpness);
}

// Reconstructed from eboot.elf at 0x213C10.
void ATanInterpolator::Reset(const Vector2& start, const Vector2& end, float sharpness) {
    mStart = start;
    mEnd = end;
    mSharpness = sharpness;
    Sync();
}

// Reconstructed from eboot.elf at 0x213CD0. The sharpness defaults to 10.
void ATanInterpolator::Reset(const DataArray* data) {
    Reset(
        Vector2{data->Float(3), data->Float(1)},
        Vector2{data->Float(4), data->Float(2)},
        data->Size() > 5 ? data->Float(5) : 10.0F);
}

// Reconstructed from eboot.elf at 0x213E20.
float ATanInterpolator::Eval(float x) const {
    return atanf(x * mXScale + mXOffset) * mYScale + mYOffset;
}

// Reconstructed from eboot.elf at 0x213E50.
void ATanInterpolator::Save(BinStream& stream) {
    WriteFloat(stream, mStart.x);
    WriteFloat(stream, mStart.y);
    WriteFloat(stream, mEnd.x);
    WriteFloat(stream, mEnd.y);
    WriteFloat(stream, mSharpness);
}

// Reconstructed from eboot.elf at 0x213F00.
void ATanInterpolator::Load(BinStream& stream) {
    stream.ReadEndian(&mStart.x, sizeof(float));
    stream.ReadEndian(&mStart.y, sizeof(float));
    stream.ReadEndian(&mEnd.x, sizeof(float));
    stream.ReadEndian(&mEnd.y, sizeof(float));
    stream.ReadEndian(&mSharpness, sizeof(float));
    Sync();
}

// Reconstructed from eboot.elf at 0x213FF0.
CubicInterpolator::CubicInterpolator()
    : mDeadZone(0.0F), mKnee(1.0F), mKneeSlope(0.0F), mMax(), mMaxSlope(0.0F) {
    Sync();
}

// Reconstructed from eboot.elf at 0x2140A0. Solves the row-vector system
// (a, b, c) * m = (mMax.y, 0, 0) for p(t) = a t^3 + b t^2 + c t, where the
// columns of m are p at the maximum, and the slope conditions at the maximum
// and at the knee.
void CubicInterpolator::Sync() {
    mKnee = Max(mKnee, mDeadZone);
    const float kneeSpan = mKnee - mDeadZone;
    mMax.x = Max(mMax.x, mDeadZone);
    const float maxSpan = mMax.x - mDeadZone;

    Hmx::Matrix3 m;
    m.x = {maxSpan * maxSpan * maxSpan, maxSpan * maxSpan * 3.0F, kneeSpan * kneeSpan * 3.0F};
    m.y = {maxSpan * maxSpan, maxSpan * 2.0F, kneeSpan * 2.0F};
    m.z = {maxSpan, 1.0F - mMaxSlope, 1.0F - mKneeSlope};
    Invert(m, m, nullptr);
    mCoeffs = {mMax.y * m.x.x, mMax.y * m.x.y, mMax.y * m.x.z};
}

// Reconstructed from eboot.elf at 0x2141A0.
CubicInterpolator::CubicInterpolator(
    float deadZone, float knee, float kneeSlope, const Vector2& max, float maxSlope)
    : mMax() {
    Reset(deadZone, knee, kneeSlope, max, maxSlope);
}

// Reconstructed from eboot.elf at 0x2142D0.
void CubicInterpolator::Reset(
    float deadZone, float knee, float kneeSlope, const Vector2& max, float maxSlope) {
    mDeadZone = deadZone;
    mKnee = knee;
    mKneeSlope = kneeSlope;
    mMax = max;
    mMaxSlope = maxSlope;
    Sync();
}

// Reconstructed from eboot.elf at 0x2143E0. The array is (cubic maxY maxX
// deadZone maxSlope knee kneeSlope).
void CubicInterpolator::Reset(const DataArray* data) {
    Reset(
        data->Float(3),
        data->Float(5),
        data->Float(6),
        Vector2{data->Float(2), data->Float(1)},
        data->Float(4));
}

// Reconstructed from eboot.elf at 0x214590.
float CubicInterpolator::Eval(float x) const {
    const float t = fabsf(x) - mDeadZone;
    if (t >= 0.0F) {
        const float sign = x > 0.0F ? 1.0F : -1.0F;
        return t * sign * ((t * mCoeffs.x + mCoeffs.y) * t + mCoeffs.z);
    }
    return 0.0F;
}

// Reconstructed from eboot.elf at 0x2145E0.
float CubicInterpolator::ClampEval(float x) const {
    return Eval(Clamp(-mMax.x, mMax.x, x));
}

// Reconstructed from eboot.elf at 0x214610.
void CubicInterpolator::Save(BinStream& stream) {
    WriteFloat(stream, mDeadZone);
    WriteFloat(stream, mMax.x);
    WriteFloat(stream, mMax.y);
    WriteFloat(stream, mMaxSlope);
    WriteFloat(stream, mKnee);
    WriteFloat(stream, mKneeSlope);
}

// Reconstructed from eboot.elf at 0x2146E0.
void CubicInterpolator::Load(BinStream& stream) {
    stream.ReadEndian(&mDeadZone, sizeof(float));
    stream.ReadEndian(&mMax.x, sizeof(float));
    stream.ReadEndian(&mMax.y, sizeof(float));
    stream.ReadEndian(&mMaxSlope, sizeof(float));
    stream.ReadEndian(&mKnee, sizeof(float));
    stream.ReadEndian(&mKneeSlope, sizeof(float));
    Sync();
}
