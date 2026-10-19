#pragma once

#include <cstddef>
#include <stdlib.h>

#include "entity/props/PropArray.h"
#include "math/color/Color.h"

// The key tracks of the property animations and waveforms (entity/Key.o).
// The tracks are templates: the binary emits each instantiation's methods
// where it is used, such as TKeyTrack<ValueTangent<float>>'s with the
// waveforms (vtable 0x18EA4A0, methods 0x1C9870-0x1CA2FD) and
// TKeyTrack<Hmx::Color>'s (vtable 0x18EA600). Key.o itself holds the
// spline sampling, which is not reconstructed. Field names are not in the
// reference map.

// A float key value with its tangents (the map's ValueTangent<T>). With
// mAutoTangents set the spline sampling (0x1DCA50) derives the tangents from
// the neighbouring keys and ignores the stored ones; a new key sets it.
template <class T>
struct ValueTangent {
    T mValue;
    T mInTangent;
    T mOutTangent;
    bool mAutoTangents;
};

// A key: its frame and value (the map's TKey<T>).
template <class T>
struct TKey {
    float mFrame;
    T mValue;
};

static_assert(sizeof(TKey<ValueTangent<float>>) == 20);

// The base of the typed key tracks: how the keys interpolate, and the
// operations that do not depend on the value type. Slots 0 to 15 of the
// tracks' vtables; TKeyTrack adds slot 16.
class KeyTrackBase {
public:
    // The map's KeyTrackBase::Interpolation: how a sample between two keys
    // blends them. The easing table at 0x18EEC60 names the values ("None",
    // "Linear", "Spline", "Hermite", "Sigmoid", then the easing curves from
    // "Quadratic In" (5) to "Quarter/Half Stair Step" (53)); a track samples
    // Spline with the keys' tangents and the others through the easing
    // curve (0x1CE460). Enumerator names are not in the reference map.
    enum Interpolation : int {
        kInterpolationNone = 0,
        kInterpolationLinear = 1,
        kInterpolationSpline = 2,
        kInterpolationHermite = 3,
        kInterpolationSigmoid = 4,
    };

    // A new track does not interpolate until it is set; the waveforms set
    // Linear (0x1CAC80), the volumetric scattering's default height
    // density Hermite (0x454A70).
    KeyTrackBase() : mInterpolation(kInterpolationNone) {}
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

static_assert(offsetof(TKeyTrack<ValueTangent<float>>, mKeys) == 16);
static_assert(sizeof(TKeyTrack<ValueTangent<float>>) == 56);

// Interpolates two linear colours as their sRGB encodings do in HSV space;
// alpha interpolates directly.
void InterpLinearAsGamma_sRGBInHSV(
    const Hmx::Color& from,
    const Hmx::Color& to,
    float t,
    Hmx::Color& result);  // 0x1DEB40
