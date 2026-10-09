#pragma once

#include <cstddef>

#include "os/threading/CritSec.h"

// Peak tracker of a statistic (audio/Meter.o). Its updates are inline; only
// the shared lock is in the map. VirtualInstrument holds two. The object is
// 32 bytes. Member names are not in the reference map.
class Meter {
public:
    Meter() : mResetPending(true), mResetValue(0.0), mPeak(0.0), mTotal(0.0) {}

    // Raises the peak to the value, first restarting it when a reset is
    // pending. Inlined, for example into FusionSampler::_ProcessNoteOn at
    // 0x97D50.
    void Update(double value) {
        ScopedCritSec lock(sAudioMeterCritSec);
        if (mResetPending) {
            mPeak = mResetValue;
            mResetPending = false;
        }
        if (mPeak < value) {
            mPeak = value;
        }
    }

    // Initialized at 0xD30F0.
    static CritSec sAudioMeterCritSec;  // 0x19C9910

    bool mResetPending;
    double mResetValue;  // The peak a reset restarts from.
    double mPeak;
    // Zeroed by VirtualInstrument's constructor and read nowhere in this
    // build. The name is weak.
    double mTotal;
};

static_assert(offsetof(Meter, mResetValue) == 8);
static_assert(offsetof(Meter, mPeak) == 16);
static_assert(sizeof(Meter) == 32);
