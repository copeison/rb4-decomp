#pragma once

#include <cstddef>

namespace SPL {

// A span of values as a start and a length. LFO::SetRangeAndMode takes one
// by value; the modulators map a normalized value onto it as
// mStart + value * mLength. Field names are not in the reference map.
template <class T>
struct Range {
    T mStart;
    T mLength;
};

static_assert(sizeof(Range<float>) == 8);

// The limits and default of a control value. ToNormValue is emitted in
// audio/FusionSampler.o and is not reconstructed. Field names are not in the
// reference map.
struct ParameterSpec {
    float mMin;
    float mDefault;
    float mMax;
};

static_assert(offsetof(ParameterSpec, mMax) == 8);
static_assert(sizeof(ParameterSpec) == 12);

// A control value clamped to its spec, with a callback for each change.
// Inlined into every Modulator member, for example Modulator::SetDepth at
// 0xBEAD0. Name not in the reference map.
class Parameter {
public:
    using Callback = void (*)(Parameter* parameter, void* data);

    explicit Parameter(const ParameterSpec& spec)
        : mSpec(spec), mValue(spec.mDefault), mCallback(nullptr), mCallbackData(nullptr) {}

    // Clamps the value to the spec and reports a change. Name not in the
    // reference map.
    void Set(float value) {
        value = value < mSpec.mMin ? mSpec.mMin : value;
        value = value > mSpec.mMax ? mSpec.mMax : value;
        if (value != mValue) {
            mValue = value;
            if (mCallback != nullptr) {
                mCallback(this, mCallbackData);
            }
        }
    }
    float Get() const {  // Name not in the reference map.
        return mValue;
    }

    // Field names are not in the reference map.
    ParameterSpec mSpec;
    float mValue;
    Callback mCallback;
    void* mCallbackData;
};

static_assert(offsetof(Parameter, mValue) == 12);
static_assert(offsetof(Parameter, mCallback) == 16);
static_assert(sizeof(Parameter) == 32);

}  // namespace SPL
