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

// The 32-tap filter of DistortionEffect's oversampling. The map emits its
// members in audio/DistortionEffect.o; they have not been reconstructed.
// The vtable is at 0x18E61A8.
class FIRFilter32 : public FIRFilter {
public:
    ~FIRFilter32() override {}  // Slots 0-1 at 0xDE920 and 0xDE970.
    void Init(const float* taps, int numTaps, bool copy) override;  // slot 2: 0xDE9C0
    void AddData(float* samples, int count) override;               // slot 3: 0xDEA80
    void AddData(float sample) override;                            // slot 4: 0xDEAD0
    float GetSample() override;                                     // slot 5: 0xDEAF0
};

static_assert(sizeof(FIRFilter32) == 40);
