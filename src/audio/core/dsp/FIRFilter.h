#pragma once

#include <cstddef>
#include <cstring>

#include "os/memory/MemMgr.h"

// Finite impulse response filter over a circular history
// (audio/FIRFilter.o, which holds no code). Every member is inline;
// audio/FusionSampler.o emits the vtable at 0x18E5040 and the members at
// 0x9D4E0 to 0x9D6EF, reached through DistortionEffect's destructor. The
// object is 40 bytes.
class FIRFilter {
public:
    FIRFilter() : mTaps(nullptr), mNumTaps(0), mOwnsTaps(false), mHistory(nullptr), mHistoryIndex(0) {}
    // Slots 0-1 at 0x9D4E0 and 0x9D530.
    virtual ~FIRFilter() {
        if (mOwnsTaps) {
            MemFree(mTaps);
            mTaps = nullptr;
        }
        MemFree(mHistory);
        mHistory = nullptr;
    }
    // Slot 2 at 0x9D580: uses or copies the taps and clears a history of
    // the same length.
    virtual void Init(const float* taps, int numTaps, bool copy) {
        if (mOwnsTaps && mTaps != nullptr) {
            MemFree(mTaps);
        }
        if (copy) {
            mTaps = static_cast<float*>(MemAlloc(sizeof(float) * numTaps, "filter_taps", 0));
            std::memcpy(mTaps, taps, sizeof(float) * numTaps);
        } else {
            mTaps = const_cast<float*>(taps);
        }
        mOwnsTaps = copy;
        mNumTaps = numTaps;
        if (mHistory != nullptr) {
            MemFree(mHistory);
        }
        mHistory = static_cast<float*>(MemAlloc(sizeof(float) * numTaps, "filter_history", 0));
        std::memset(mHistory, 0, sizeof(float) * numTaps);
        mHistoryIndex = 0;
    }
    // Slot 3 at 0x9D640.
    virtual void AddData(float* samples, int count) {
        int index = mHistoryIndex;
        for (int i = 0; i < count; ++i) {
            mHistory[index] = samples[i];
            if (++index == mNumTaps) {
                index = 0;
            }
        }
        mHistoryIndex = index;
    }
    // Slot 4 at 0x9D680.
    virtual void AddData(float sample) {
        mHistory[mHistoryIndex] = sample;
        if (++mHistoryIndex == mNumTaps) {
            mHistoryIndex = 0;
        }
    }
    // Slot 5 at 0x9D6A0: the taps applied to the history, newest first.
    virtual float GetSample() {
        float sum = 0.0f;
        int index = mHistoryIndex;
        for (int i = 0; i < mNumTaps; ++i) {
            if (index == 0) {
                index = mNumTaps;
            }
            --index;
            sum = mTaps[i] * mHistory[index] + sum;
        }
        return sum;
    }

    // Clears the history without freeing it. Inlined into
    // DistortionEffect::SetOversample at 0xDD920. Name not in the reference
    // map.
    void ClearHistory() {
        std::memset(mHistory, 0, sizeof(float) * mNumTaps);
        mHistoryIndex = 0;
    }

    // Field names are not in the reference map.
    float* mTaps;
    int mNumTaps;
    bool mOwnsTaps;
    float* mHistory;
    int mHistoryIndex;  // Where the next sample goes.
};

static_assert(offsetof(FIRFilter, mNumTaps) == 16);
static_assert(offsetof(FIRFilter, mHistory) == 24);
static_assert(sizeof(FIRFilter) == 40);

// The 32-tap filter of DistortionEffect's 4x oversampling, which indexes
// its history modulo 32. Every member is inline; the map emits them in
// audio/DistortionEffect.o, as this build does at 0xDE5B0 to 0xDEDF3 with the
// vtable at 0x18E61A8. audio/FusionSampler.o inlines the destructor into
// DistortionEffect's.
class FIRFilter32 : public FIRFilter {
public:
    // Slots 0-1 at 0xDE920 and 0xDE970.
    ~FIRFilter32() override {}
    // Slot 2 at 0xDE9C0: the base version, inlined.
    void Init(const float* taps, int numTaps, bool copy) override {
        FIRFilter::Init(taps, numTaps, copy);
    }
    // Slot 3 at 0xDEA80.
    void AddData(float* samples, int count) override {
        for (int i = 0; i < count; ++i) {
            mHistory[mHistoryIndex & 0x1F] = samples[i];
            ++mHistoryIndex;
        }
    }
    // Slot 4 at 0xDEAD0.
    void AddData(float sample) override {
        mHistory[mHistoryIndex++ & 0x1F] = sample;
    }
    // Slot 5 at 0xDEAF0, vectorized in the binary. Tap 0 meets the oldest
    // sample, the slot the next sample overwrites; tap i > 0 meets the
    // sample i steps before it.
    float GetSample() override {
        float sum = 0.0f;
        for (int i = 0; i < 32; ++i) {
            sum += mTaps[i] * mHistory[(mHistoryIndex - i) & 0x1F];
        }
        return sum;
    }

    // Adds a sample followed by three implicit zeros (the slots it skips
    // stay zero) and writes the four polyphase outputs. Not virtual. At
    // 0xDE5B0.
    void Upsample4x(float sample, float* output) {
        mHistory[mHistoryIndex & 0x1F] = sample;
        mHistoryIndex += 4;
        for (int phase = 0; phase < 4; ++phase) {
            output[phase] = 0.0f;
            for (int i = 0; i < 8; ++i) {
                output[phase] += mTaps[4 * i + phase] * mHistory[(mHistoryIndex + 28 - 4 * i) & 0x1F];
            }
        }
    }
};

static_assert(sizeof(FIRFilter32) == 40);
