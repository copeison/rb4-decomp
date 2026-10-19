#pragma once

#include <cstddef>
#include <stdlib.h>

#include "entity/props/PropArray.h"
#include "math/color/Color.h"
#include "math/easing/Easing.h"
#include "math/rotation/Quat.h"
#include "math/vector/Vector3.h"

// The key tracks of the property animations and waveforms (entity/Key.o).
// The tracks are templates: the binary emits each instantiation's methods
// where it is used, such as TKeyTrack<ValueTangent<float>>'s with the
// waveforms (vtable 0x18EA4A0, methods 0x1C9870-0x1CA2FD) and
// TKeyTrack<Hmx::Color>'s (vtable 0x18EA600). Key.o itself holds the
// spline sampling and the interpolation upgrade. Field names are not in the
// reference map.

// A float key value with its tangents (the map's ValueTangent<T>). With
// mAutoTangents set the spline sampling (0x1DCA50) derives the tangents from
// the neighbouring keys and ignores the stored ones. A new value is zero
// with automatic tangents, as the float keys' prototype (0x305FD0) and
// samples (0x306C30) show.
template <class T>
struct ValueTangent {
    T mValue = T();
    T mInTangent = T();
    T mOutTangent = T();
    bool mAutoTangents = true;

    // The same value and tangent mode, and with stored tangents the same
    // tangents. Inlined into WaveformResource<T>::SetCurve.
    bool operator==(const ValueTangent& other) const {
        return mValue == other.mValue && mAutoTangents == other.mAutoTangents &&
            (mAutoTangents || (mInTangent == other.mInTangent && mOutTangent == other.mOutTangent));
    }
};

// A key: its frame and value (the map's TKey<T>).
template <class T>
struct TKey {
    float mFrame;
    T mValue;

    bool operator==(const TKey& other) const {
        return mFrame == other.mFrame && mValue == other.mValue;
    }
};

static_assert(sizeof(TKey<ValueTangent<float>>) == 20);

// The base of the typed key tracks: how the keys interpolate, and the
// operations that do not depend on the value type. Slots 0 to 15 of the
// tracks' vtables; TKeyTrack adds slot 16.
class KeyTrackBase {
public:
    // The map's KeyTrackBase::Interpolation: how a sample between two keys
    // blends them. The easing table at 0x18EEC60 names the values; a track
    // samples Spline with the keys' tangents and the others through the
    // easing curve (InterpolationEase). The track keeps flags in the upper
    // 16 bits of the field (0x20000000 from the revision 0 upgrade,
    // 0x40000000 on the rotation tracks), so the samplers read the low 16.
    // Enumerator names are not in the reference map; they follow the
    // table's labels.
    enum Interpolation : int {
        kInterpolationNone = 0,
        kInterpolationLinear = 1,
        kInterpolationSpline = 2,
        kInterpolationHermite = 3,
        kInterpolationSigmoid = 4,
        kInterpolationQuadraticIn = 5,
        kInterpolationQuadraticOut = 6,
        kInterpolationQuadraticInOut = 7,
        kInterpolationQuadraticOutIn = 8,
        kInterpolationCubicIn = 9,
        kInterpolationCubicOut = 10,
        kInterpolationCubicInOut = 11,
        kInterpolationCubicOutIn = 12,
        kInterpolationQuarticIn = 13,
        kInterpolationQuarticOut = 14,
        kInterpolationQuarticInOut = 15,
        kInterpolationQuarticOutIn = 16,
        kInterpolationQuinticIn = 17,
        kInterpolationQuinticOut = 18,
        kInterpolationQuinticInOut = 19,
        kInterpolationQuinticOutIn = 20,
        kInterpolationPolynomialIn = 21,
        kInterpolationPolynomialOut = 22,
        kInterpolationPolynomialInOut = 23,
        kInterpolationPolynomialOutIn = 24,
        kInterpolationBackIn = 25,
        kInterpolationBackOut = 26,
        kInterpolationBackInOut = 27,
        kInterpolationBackOutIn = 28,
        kInterpolationBounceIn = 29,
        kInterpolationBounceOut = 30,
        kInterpolationBounceInOut = 31,
        kInterpolationBounceOutIn = 32,
        kInterpolationCircularIn = 33,
        kInterpolationCircularOut = 34,
        kInterpolationCircularInOut = 35,
        kInterpolationCircularOutIn = 36,
        kInterpolationElasticIn = 37,
        kInterpolationElasticOut = 38,
        kInterpolationElasticInOut = 39,
        kInterpolationElasticOutIn = 40,
        kInterpolationExponentialIn = 41,
        kInterpolationExponentialOut = 42,
        kInterpolationExponentialInOut = 43,
        kInterpolationExponentialOutIn = 44,
        kInterpolationSineIn = 45,
        kInterpolationSineOut = 46,
        kInterpolationSineInOut = 47,
        kInterpolationSineOutIn = 48,
        kInterpolationHalfStairStep = 49,
        kInterpolationThirdStairStep = 50,
        kInterpolationQuarterStairStep = 51,
        kInterpolationHalfQuarterStairStep = 52,
        kInterpolationQuarterHalfStairStep = 53,
    };

    // The interpolation's label in the easing table ("Linear", "Cubic
    // In/Out"), "Invalid" for -1 and null out of range; and the value of
    // a label, -1 when none matches. The map has them in entity/Key.o;
    // this build emits them in math/Easing.o.
    static const char* InterpolationToString(Interpolation interpolation);  // 0x212420
    static Interpolation StringToInterpolation(const char* name);           // 0x212440

    // A new track does not interpolate until it is set; the waveforms set
    // Linear (0x1CAC80), the volumetric scattering's default height
    // density Hermite (0x454A70).
    KeyTrackBase() : mInterpolation(kInterpolationNone) {}
    explicit KeyTrackBase(Interpolation interpolation) : mInterpolation(interpolation) {}
    // Slots 0-1.
    virtual ~KeyTrackBase() {}

    // Slots 2-3, folded for every type at 0x1C9870 and 0x1C9F70.
    virtual void SetInterpolation(Interpolation interpolation) {
        mInterpolation = interpolation;
    }
    virtual Interpolation GetInterpolation() const {
        return mInterpolation;
    }
    // Slot 4: removes every key.
    virtual void Clear() = 0;
    // Slot 5.
    virtual unsigned long NumKeys() const = 0;
    // Slot 6, folded at 0x1C9FA0.
    virtual bool Empty() const {
        return NumKeys() == 0;
    }
    // Slots 7-8: the first and last keys' frames, 0 without keys.
    virtual float GetFirstFrame() const = 0;
    virtual float GetLastFrame() const = 0;
    virtual void RemoveKeyAtIndex(unsigned long index) = 0;  // slot 9
    // Slot 10: moves the key to the frame and returns its new index;
    // `before` places it before the keys already at the frame.
    virtual unsigned long MoveKey(unsigned long index, float frame, bool before) = 0;
    // Slot 11: adds the offset to every key's frame. Name not in the
    // reference map.
    virtual void OffsetFrames(float offset) = 0;
    // Slot 12: removes the keys from `start` up to, but not at, `end` and
    // returns the index of the first.
    virtual unsigned long RemoveKeysInFrameRange(float start, float end) = 0;
    virtual float GetKeyFrameAtIndex(unsigned long index) const = 0;  // slot 13
    // Slot 14: makes room for the keys.
    virtual void Reserve(unsigned long count) = 0;
    // Slot 15: sorts the keys by frame (qsort).
    virtual void Sort() = 0;

    Interpolation mInterpolation;
};

// Eases the position between two keys, from 0 to 1, by the interpolation's
// curve; None gives 0 and Linear, Spline and unknown values give the
// position. Emitted at 0x1CE460 with the waveforms' sampling, the elastic
// in/out curves after it. Name not in the reference map.
inline float InterpolationEase(KeyTrackBase::Interpolation interpolation, float t) {
    switch (interpolation) {
    case KeyTrackBase::kInterpolationNone:
        return 0.0f;
    case KeyTrackBase::kInterpolationHermite:
    case KeyTrackBase::kInterpolationSigmoid:
        return t * t * (t * -2.0f + 3.0f);
    case KeyTrackBase::kInterpolationQuadraticIn:
        return EaseQuadraticIn(t);
    case KeyTrackBase::kInterpolationQuadraticOut:
        return EaseQuadraticOut(t);
    case KeyTrackBase::kInterpolationQuadraticInOut:
        return EaseInOut(t, EaseQuadraticIn, EaseQuadraticOut);
    case KeyTrackBase::kInterpolationQuadraticOutIn:
        return EaseOutIn(t, EaseQuadraticIn, EaseQuadraticOut);
    case KeyTrackBase::kInterpolationCubicIn:
        return EaseCubicIn(t);
    case KeyTrackBase::kInterpolationCubicOut:
        return EaseCubicOut(t);
    case KeyTrackBase::kInterpolationCubicInOut:
        return EaseInOut(t, EaseCubicIn, EaseCubicOut);
    case KeyTrackBase::kInterpolationCubicOutIn:
        return EaseOutIn(t, EaseCubicIn, EaseCubicOut);
    case KeyTrackBase::kInterpolationQuarticIn:
        return EaseQuarticIn(t);
    case KeyTrackBase::kInterpolationQuarticOut:
        return EaseQuarticOut(t);
    case KeyTrackBase::kInterpolationQuarticInOut:
        return EaseInOut(t, EaseQuarticIn, EaseQuarticOut);
    case KeyTrackBase::kInterpolationQuarticOutIn:
        return EaseOutIn(t, EaseQuarticIn, EaseQuarticOut);
    case KeyTrackBase::kInterpolationQuinticIn:
        return EaseQuinticIn(t);
    case KeyTrackBase::kInterpolationQuinticOut:
        return EaseQuinticOut(t);
    case KeyTrackBase::kInterpolationQuinticInOut:
        return EaseInOut(t, EaseQuinticIn, EaseQuinticOut);
    case KeyTrackBase::kInterpolationQuinticOutIn:
        return EaseOutIn(t, EaseQuinticIn, EaseQuinticOut);
    case KeyTrackBase::kInterpolationPolynomialIn:
        return EasePowerIn(t, gEasePolynomialPower);
    case KeyTrackBase::kInterpolationPolynomialOut:
        return EasePowerOut(t, gEasePolynomialPower);
    case KeyTrackBase::kInterpolationPolynomialInOut:
        return EaseInOut(
            t,
            [](float x) { return EasePowerIn(x, gEasePolynomialPower); },
            [](float x) { return EasePowerOut(x, gEasePolynomialPower); });
    case KeyTrackBase::kInterpolationPolynomialOutIn:
        return EaseOutIn(
            t,
            [](float x) { return EasePowerIn(x, gEasePolynomialPower); },
            [](float x) { return EasePowerOut(x, gEasePolynomialPower); });
    case KeyTrackBase::kInterpolationBackIn:
        return EaseBackIn(t, gEaseBackOvershoot);
    case KeyTrackBase::kInterpolationBackOut:
        return EaseBackOut(t, gEaseBackOvershoot);
    case KeyTrackBase::kInterpolationBackInOut:
        return EaseInOut(
            t,
            [](float x) { return EaseBackIn(x, gEaseBackOvershoot); },
            [](float x) { return EaseBackOut(x, gEaseBackOvershoot); });
    case KeyTrackBase::kInterpolationBackOutIn:
        return EaseOutIn(
            t,
            [](float x) { return EaseBackIn(x, gEaseBackOvershoot); },
            [](float x) { return EaseBackOut(x, gEaseBackOvershoot); });
    case KeyTrackBase::kInterpolationBounceIn:
        return EaseBounceIn(t);
    case KeyTrackBase::kInterpolationBounceOut:
        return EaseBounceOut(t);
    case KeyTrackBase::kInterpolationBounceInOut:
        return EaseInOut(t, EaseBounceIn, EaseBounceOut);
    case KeyTrackBase::kInterpolationBounceOutIn:
        return EaseOutIn(t, EaseBounceIn, EaseBounceOut);
    case KeyTrackBase::kInterpolationCircularIn:
        return EaseCircularIn(t);
    case KeyTrackBase::kInterpolationCircularOut:
        return EaseCircularOut(t);
    case KeyTrackBase::kInterpolationCircularInOut:
        return EaseInOut(t, EaseCircularIn, EaseCircularOut);
    case KeyTrackBase::kInterpolationCircularOutIn:
        return EaseOutIn(t, EaseCircularIn, EaseCircularOut);
    case KeyTrackBase::kInterpolationElasticIn:
        return EaseElasticIn(t, gEaseElasticAmplitude, gEaseElasticPeriod);
    case KeyTrackBase::kInterpolationElasticOut:
        return EaseElasticOut(t, gEaseElasticAmplitude, gEaseElasticPeriod);
    case KeyTrackBase::kInterpolationElasticInOut:
        return EaseElasticInOut(t, gEaseElasticAmplitude, gEaseElasticPeriod);
    case KeyTrackBase::kInterpolationElasticOutIn:
        return EaseElasticOutIn(t, gEaseElasticAmplitude, gEaseElasticPeriod);
    case KeyTrackBase::kInterpolationExponentialIn:
        return EaseExponentialIn(t);
    case KeyTrackBase::kInterpolationExponentialOut:
        return EaseExponentialOut(t);
    case KeyTrackBase::kInterpolationExponentialInOut:
        return EaseInOut(t, EaseExponentialIn, EaseExponentialOut);
    case KeyTrackBase::kInterpolationExponentialOutIn:
        return EaseOutIn(t, EaseExponentialIn, EaseExponentialOut);
    case KeyTrackBase::kInterpolationSineIn:
        return EaseSineIn(t);
    case KeyTrackBase::kInterpolationSineOut:
        return EaseSineOut(t);
    case KeyTrackBase::kInterpolationSineInOut:
        return EaseInOut(t, EaseSineIn, EaseSineOut);
    case KeyTrackBase::kInterpolationSineOutIn:
        return EaseOutIn(t, EaseSineIn, EaseSineOut);
    case KeyTrackBase::kInterpolationHalfStairStep:
        return EaseStairStep(t, 2.0f, gEaseStairStepPower);
    case KeyTrackBase::kInterpolationThirdStairStep:
        return EaseStairStep(t, 3.0f, gEaseStairStepPower);
    case KeyTrackBase::kInterpolationQuarterStairStep:
        return EaseStairStep(t, 4.0f, gEaseStairStepPower);
    case KeyTrackBase::kInterpolationHalfQuarterStairStep:
        return EaseStairStep(t, t < 0.5f ? 2.0f : 4.0f, gEaseStairStepPower);
    case KeyTrackBase::kInterpolationQuarterHalfStairStep:
        return EaseStairStep(t, t < 0.5f ? 4.0f : 2.0f, gEaseStairStepPower);
    default:
        return t;
    }
}

// Converts an interpolation saved before revision 1 into the field's
// current form: Linear, Hermite and the cubic curves move to their new
// values, the old values from 7 on set the 0x20000000 flag, and unknown
// values become -1. The other flag bits are kept. Called by the key
// components' revision handlers. Name not in the reference map.
void UpgradeInterpolation(int oldInterpolation, KeyTrackBase::Interpolation* interpolation);  // 0x1DC930

// A track of keys sorted by frame (the map's TKeyTrack<T>; 56 bytes). The
// methods are the template's, which the compiler emits for each value type
// as the binary does; the addresses given are TKeyTrack<ValueTangent<float>>'s.
template <class T>
class TKeyTrack : public KeyTrackBase {
public:
    // Inlined into its users, for example WaveformResource<Hmx::Color>'s
    // constructor (0x1CAC80).
    TKeyTrack() : mKeys() {}
    // Slots 0-1: 0x1C9A10, 0x1C9F20.
    ~TKeyTrack() override {}

    // Slot 4: 0x1C9F80.
    void Clear() override {
        mKeys.Resize(0);
    }
    // Slot 5: 0x1C9F90.
    unsigned long NumKeys() const override {
        return mKeys.mSize;
    }
    // Slots 7-8: 0x1C9FC0, 0x1C9FE0.
    float GetFirstFrame() const override {
        return mKeys.mSize != 0 ? Keys()[0].mFrame : 0.0f;
    }
    float GetLastFrame() const override {
        return mKeys.mSize != 0 ? Keys()[mKeys.mSize - 1].mFrame : 0.0f;
    }
    // Slot 9: 0x1CA000.
    void RemoveKeyAtIndex(unsigned long index) override {
        mKeys.Destruct(1, mKeys.ElementAt(index));
        const unsigned int last = mKeys.mSize - 1;
        if (static_cast<unsigned int>(index) < last) {
            mKeys.Move(last - index, mKeys.ElementAt(index), mKeys.ElementAt(index + 1));
        }
        mKeys.mSize = last;
    }
    // Slot 10: 0x1CA080.
    unsigned long MoveKey(unsigned long index, float frame, bool before) override {
        const T value = Keys()[index].mValue;
        RemoveKeyAtIndex(index);
        return AddKey(value, frame, false, before);
    }
    // Slot 11: 0x1CA100.
    void OffsetFrames(float offset) override {
        TKey<T>* const keys = Keys();
        for (unsigned long i = 0; i < mKeys.mSize; ++i) {
            keys[i].mFrame = offset + keys[i].mFrame;
        }
    }
    // Slot 12: 0x1CA130. A key at `end` stays.
    unsigned long RemoveKeysInFrameRange(float start, float end) override {
        const unsigned long first = mKeys.mSize != 0 ? LowerBound(start) : NumKeys();
        const long last = static_cast<long>(LowerBound(end)) - 1;
        for (long count = last - static_cast<long>(first) + 1; count > 0; --count) {
            RemoveKeyAtIndex(first);
        }
        return first;
    }
    // Slot 13: 0x1CA2D0.
    float GetKeyFrameAtIndex(unsigned long index) const override {
        return Keys()[index].mFrame;
    }
    // Slot 14: 0x1C9880.
    void Reserve(unsigned long count) override {
        mKeys.Reserve(static_cast<unsigned int>(count));
    }
    // Slot 15: 0x1CA2E0, with the comparison at 0x1CA4B0.
    void Sort() override;
    // Slot 16: inserts the key and returns its index. With `replace` a key
    // already at the frame takes the value instead; otherwise the new key
    // goes before (`before`) or after the keys at its frame. 0x1C9900.
    virtual unsigned long AddKey(const T& value, float frame, bool replace, bool before) {
        const TKey<T> key = {frame, value};
        unsigned long index = mKeys.mSize != 0 ? LowerBound(frame) : 0;
        if (index != mKeys.mSize && replace && frame == Keys()[index].mFrame) {
            Keys()[index].mValue = value;
            return index;
        }
        if (!before) {
            while (index < mKeys.mSize && frame == Keys()[index].mFrame) {
                ++index;
            }
        }
        mKeys._Insert(index, &key);
        return index;
    }

    // The keys around the frame and how far the frame lies between them,
    // with the index of the first: without keys both are null and the index
    // is -1; before the first key both are the first key and the index is
    // -1; at or after the last both are the last. Inlined into
    // SampleValueAtFrame.
    unsigned long FindKeysAtFrame(float frame, const TKey<T>*& prev, const TKey<T>*& next, float& t) const {
        const TKey<T>* const keys = Keys();
        const unsigned long count = mKeys.mSize;
        t = 0.0f;
        if (count == 0) {
            prev = next = nullptr;
            return static_cast<unsigned long>(-1);
        }
        if (frame < keys[0].mFrame) {
            prev = next = &keys[0];
            return static_cast<unsigned long>(-1);
        }
        const unsigned long last = count - 1;
        if (frame >= keys[last].mFrame) {
            prev = next = &keys[last];
            return last;
        }
        unsigned long first = 0;
        unsigned long remaining = last + 1;
        while (remaining != 0) {
            const unsigned long half = remaining >> 1;
            if (frame < keys[first + half].mFrame) {
                remaining = half;
            } else {
                first += half + 1;
                remaining -= half + 1;
            }
        }
        unsigned long index = first - 1;
        if (index >= count) {
            index = static_cast<unsigned long>(-1);
        }
        prev = &keys[index];
        next = &keys[index + 1];
        t = (frame - prev->mFrame) / (next->mFrame - prev->mFrame);
        return index;
    }
    // Samples the keys at the frame into `value` and returns the index of
    // the key at or before it; without keys the value is left and the
    // index is -1. 0x1CE320 for ValueTangent<float>, 0x31AED0 for String;
    // inlined for the discrete types.
    unsigned long SampleValueAtFrame(float frame, T& value) const {
        const TKey<T>* prev;
        const TKey<T>* next;
        float t;
        const unsigned long index = FindKeysAtFrame(frame, prev, next, t);
        _DoSampleValueAtFrame(prev, next, t, value);
        return index;
    }
    // Blends two keys by the interpolation: nothing without keys, the
    // first key's value when they are the same key, the spline through
    // their tangents, or the eased blend. 0x31B030 for String.
    void _DoSampleValueAtFrame(const TKey<T>* prev, const TKey<T>* next, float t, T& value) const {
        if (prev == nullptr) {
            return;
        }
        if (next == prev) {
            value = prev->mValue;
            return;
        }
        const Interpolation interpolation = static_cast<Interpolation>(mInterpolation & 0xFFFF);
        if (interpolation == kInterpolationSpline) {
            _SampleSplineInterpValue(prev, next, t, value);
        } else {
            _SampleLinearInterpValue(prev, next, InterpolationEase(interpolation, t), value);
        }
    }
    // The blend at the eased position. Values that do not blend keep the
    // first key's until the position reaches the second; the types that
    // do specialize it.
    void _SampleLinearInterpValue(const TKey<T>* prev, const TKey<T>* next, float t, T& value) const {
        value = t < 1.0f ? prev->mValue : next->mValue;
    }
    // The spline through the keys and their tangents. Values that do not
    // blend have no spline and keep the value; the types that do
    // specialize it.
    void _SampleSplineInterpValue(const TKey<T>*, const TKey<T>*, float, T&) const {}

    PropArray<TKey<T>> mKeys;

private:
    // The keys. Name not in the reference map.
    TKey<T>* Keys() const {
        return static_cast<TKey<T>*>(mKeys.mData);
    }
    // The first key at or after the frame. Inlined into the searches. Name
    // not in the reference map.
    unsigned long LowerBound(float frame) const {
        const TKey<T>* const keys = Keys();
        unsigned long first = 0;
        unsigned long count = mKeys.mSize;
        while (count != 0) {
            const unsigned long half = count >> 1;
            if (frame > keys[first + half].mFrame) {
                first += half + 1;
                count -= half + 1;
            } else {
                count = half;
            }
        }
        return first;
    }
    // Orders keys by frame for qsort. Name not in the reference map.
    static int CompareFrames(const void* a, const void* b) {
        const float frameA = static_cast<const TKey<T>*>(a)->mFrame;
        const float frameB = static_cast<const TKey<T>*>(b)->mFrame;
        if (frameA < frameB) {
            return -1;
        }
        return frameA > frameB ? 1 : 0;
    }
};

template <class T>
void TKeyTrack<T>::Sort() {
    if (mKeys.mSize != 0) {
        qsort(mKeys.mData, mKeys.mSize, sizeof(TKey<T>), CompareFrames);
    }
}

// The float keys blend their values; the blended key keeps the first
// key's tangents. Inlined into 0x1CE320.
template <>
inline void TKeyTrack<ValueTangent<float>>::_SampleLinearInterpValue(
    const TKey<ValueTangent<float>>* prev,
    const TKey<ValueTangent<float>>* next,
    float t,
    ValueTangent<float>& value) const {
    value.mValue = (next->mValue.mValue - prev->mValue.mValue) * t + prev->mValue.mValue;
}

// The cubic Hermite curve between the keys, whose tangents are scaled by
// the span between them; an automatic tangent is the slope through the
// neighbouring keys (SplineTangent). A stored tangent steeper than 500
// holds the first key's value instead.
template <>
void TKeyTrack<ValueTangent<float>>::_SampleSplineInterpValue(
    const TKey<ValueTangent<float>>* prev,
    const TKey<ValueTangent<float>>* next,
    float t,
    ValueTangent<float>& value) const;  // 0x1DCA50

// The vector keys blend each component; the ends give the keys' values
// exactly. Inlined into 0x324D40.
template <>
inline void TKeyTrack<Vector3>::_SampleLinearInterpValue(
    const TKey<Vector3>* prev,
    const TKey<Vector3>* next,
    float t,
    Vector3& value) const {
    if (t == 0.0f) {
        value = prev->mValue;
    } else if (t == 1.0f) {
        value = next->mValue;
    } else {
        Interp(prev->mValue, next->mValue, t, value);
    }
}

// The cubic Hermite curve between the keys, with the slopes through the
// neighbouring keys (SplineTangent, inlined twice): k1-k0 for two keys,
// (k1-k0)*1.5+(k0-k2)*0.25 at the first key and its mirror at the last,
// (k[i+1]-k[i-1])*0.5 otherwise. Not reconstructed.
template <>
void TKeyTrack<Vector3>::_SampleSplineInterpValue(
    const TKey<Vector3>* prev,
    const TKey<Vector3>* next,
    float t,
    Vector3& value) const;  // 0x1DCCF0

// The quaternion keys blend along the shorter arc. Inlined into 0x3125F0.
template <>
inline void TKeyTrack<Hmx::Quat>::_SampleLinearInterpValue(
    const TKey<Hmx::Quat>* prev,
    const TKey<Hmx::Quat>* next,
    float t,
    Hmx::Quat& value) const {
    FastInterp(prev->mValue, next->mValue, t, value);
}

// Catmull-Rom through the keys before and after the pair (the ends
// repeated), each neighbour flipped onto the first key's hemisphere, then
// normalized. Not reconstructed.
template <>
void TKeyTrack<Hmx::Quat>::_SampleSplineInterpValue(
    const TKey<Hmx::Quat>* prev,
    const TKey<Hmx::Quat>* next,
    float t,
    Hmx::Quat& value) const;  // 0x1DD190

// The colour keys blend in gamma space: with the interpolation's
// 0x20000000 flag through InterpLinearAsGamma_sRGBInHSV, otherwise both
// keys are converted to sRGB, blended and converted back. They have no
// spline. Not reconstructed; the track's SampleValueAtFrame is emitted at
// 0x2424A0.
template <>
void TKeyTrack<Hmx::Color>::_SampleLinearInterpValue(
    const TKey<Hmx::Color>* prev,
    const TKey<Hmx::Color>* next,
    float t,
    Hmx::Color& value) const;  // 0x1DEA70

static_assert(sizeof(TKey<Vector3>) == 16);
static_assert(sizeof(TKey<Hmx::Quat>) == 20);
static_assert(sizeof(TKey<Hmx::Color>) == 20);
static_assert(offsetof(TKeyTrack<ValueTangent<float>>, mKeys) == 16);
static_assert(sizeof(TKeyTrack<ValueTangent<float>>) == 56);

// Interpolates two linear colours as their sRGB encodings do in HSV space;
// alpha interpolates directly.
void InterpLinearAsGamma_sRGBInHSV(
    const Hmx::Color& from,
    const Hmx::Color& to,
    float t,
    Hmx::Color& result);  // 0x1DEB40
