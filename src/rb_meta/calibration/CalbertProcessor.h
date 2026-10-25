#pragma once

#include <cstddef>

#include "utl/containers/Vector.h"

// Finds the latency of a repeating burst in a sampled signal, the automatic
// ("Calbert") calibration's detector (rb_meta/CalbertProcessor.o,
// 0xF06030-0xF0727F; this build places the object after RBAchUtl.o). The
// samples are resampled at a fiftieth of the burst period; each period's
// strongest rise, weighted with the following burst's mean, counts as a
// burst when its mean stands out from the rest of the period. Once ten
// bursts are found, the middle ten agree closely enough to give the delay.
// Not polymorphic; the object is 112 bytes. Field names are not in the
// reference map.
class CalbertProcessor {
public:
    CalbertProcessor();  // 0xF06030

    // Forgets the samples and the bursts.
    void Reset();  // 0xF06160
    // The burst period and the burst's length, in the samples' time unit:
    // the signal is resampled 50 times per period, and the burst covers its
    // length in resampled points. Room is reserved for 1250 points and 25
    // bursts.
    void SetBurstInfo(float period, float length);  // 0xF06230
    // How far a burst's mean must stand out from the rest of the period,
    // and the weight of the rise in the burst's score.
    void SetParams(float peakThreshold, float riseWeight);  // 0xF06340
    // Adds the samples taken evenly up to `time`; an earlier time starts
    // over, and a time not after zero only resets.
    void AddSamples(float time, float* samples, int count);  // 0xF06350
    // The progress from 0 to 1; at 1, `delay` receives the delay.
    float GetProgress(float& delay);  // 0xF06C30
    // Empty in this build.
    void PrintResampled();  // 0xF06C10
    void PrintResampled(int start, int end);  // 0xF06C20

    // Appends the resampled points up to the sample at `time`, interpolated
    // from the previous sample.
    void _Resample(float time, float value);  // 0xF065D0
    // Looks for a burst in the next period of resampled points once a whole
    // one has arrived.
    void _UpdateEstimate();  // 0xF06760
    // The point in [start, end - burst length) with the strongest rise
    // plus following burst mean, and that mean. Inlined into
    // _UpdateEstimate; the binary keeps an uncalled copy.
    int _GetBurstIndexAndMean(int start, int end, float& mean);  // 0xF06C50
    // The mean of [start, end) without [burstStart, burstEnd). Inlined into
    // _UpdateEstimate; the binary keeps an uncalled copy.
    float _NonBurstMean(int start, int end, int burstStart, int burstEnd);  // 0xF06DD0
    // From the sorted burst positions: the progress while fewer than ten are
    // known or the middle ten spread too far, otherwise the delay.
    void _CalcDelayAndProgress();  // 0xF06F80
    // The mean and the standard deviation of the middle ten burst
    // positions. Inlined into _CalcDelayAndProgress; the binary keeps an
    // uncalled copy.
    void _GetCandidatesMeanStdDev(float& mean, float& stdDev);  // 0xF07120

    // 4 by default.
    float mPeakThreshold;
    // The time between resampled points.
    float mResampleInterval;
    // The burst's length in resampled points.
    int mBurstLength;
    // The time of the last sample.
    float mLastTime;
    eastl::vector<float> mResampled;
    float mPrevTime;
    float mPrevValue;
    // The resampled points already searched, a multiple of 50.
    int mProcessed;
    // The bursts' positions in their periods, sorted.
    eastl::vector<float> mBursts;
    float mProgress;
    float mDelay;
    // The last point the rise was taken from; -1000000 before the first.
    float mPrevPoint;
    // 1 by default.
    float mRiseWeight;
};

static_assert(offsetof(CalbertProcessor, mResampled) == 16);
static_assert(offsetof(CalbertProcessor, mPrevTime) == 48);
static_assert(offsetof(CalbertProcessor, mBursts) == 64);
static_assert(offsetof(CalbertProcessor, mProgress) == 96);
static_assert(offsetof(CalbertProcessor, mRiseWeight) == 108);
static_assert(sizeof(CalbertProcessor) == 112);
