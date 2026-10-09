#pragma once

#include <cstddef>

namespace SPL {

// Linear ramp of a control value, advanced once per control block, with a
// callback for when it reaches its target. The map emits SetTarget and the
// ramp shapes in audio/SoundManager.o; this build inlines a linear SetTarget
// into its users, such as FusionVoice and FusionSampler. The object is 48
// bytes.
class Ramper {
public:
    // Inlined, for example into FusionVoice's constructor at 0x9D920. The
    // value, the target and the step are left unset.
    Ramper() : mIncrement(1.0f), mProgress(1.0f), mRate(1.0f), mUpdating(false), mCallback(nullptr) {}

    // Sets the control rate and the ramp length. Inlined into
    // FusionVoice::SetSampleRate at 0x9DC50. Name not in the reference map.
    void SetRate(float rate, float rampSeconds) {
        mRate = rate;
        mIncrement = 1.0f / rate * (1.0f / rampSeconds);
    }
    // Starts a ramp from the current value. The map's out-of-line version
    // also selects a ramp shape. Inlined, for example into
    // FusionVoice::_RestoreFilterGain at 0x9F250.
    void SetTarget(float target, void (*callback)(void*), void* data) {
        mStep = (target - mValue) * mIncrement;
        mUpdating = true;
        mTarget = target;
        mProgress = 0.0f;
        mUpdating = false;
        mCallback = callback;
        mCallbackData = data;
    }
    // Jumps to the target and fires the callback once, unless a SetTarget is
    // in progress. Inlined into FusionVoice::Process at 0x9F2A0. Name not in
    // the reference map.
    void Finish() {
        if (!mUpdating) {
            mValue = mTarget;
            mProgress = 1.0f;
            if (mCallback != nullptr) {
                mCallback(mCallbackData);
            }
            mCallback = nullptr;
            mCallbackData = nullptr;
        }
    }
    // Moves one control block along the ramp. Inlined into
    // FusionVoice::Process at 0x9F2A0. Name not in the reference map.
    void Advance() {
        if (mProgress != 1.0f) {
            mProgress += mIncrement;
            mValue += mStep;
            if (mProgress >= 1.0f) {
                Finish();
            }
        }
    }
    // Whether no ramp is running. Name not in the reference map.
    bool IsIdle() const {
        return mProgress == 1.0f;
    }

    // Field names are not in the reference map.
    float mValue;
    float mTarget;
    float mStep;       // Value change per control block.
    float mIncrement;  // Progress per control block.
    float mProgress;   // 0 to 1; 1 when idle.
    float mRate;       // Control blocks per second.
    // Set while SetTarget runs, so that the audio thread does not finish a
    // half-written ramp.
    volatile bool mUpdating;
    void (*mCallback)(void*);
    void* mCallbackData;
};

static_assert(offsetof(Ramper, mIncrement) == 12);
static_assert(offsetof(Ramper, mUpdating) == 24);
static_assert(offsetof(Ramper, mCallback) == 32);
static_assert(sizeof(Ramper) == 48);

}  // namespace SPL
