// rb_meta/CalbertProcessor.o (0xF06030 to 0xF0727F). The burst positions
// are sorted with the shared eastl::sort<float*> instance (0xA3E750). The
// square roots are the fast approximate ones of the build's float model.
#include "rb_meta/calibration/CalbertProcessor.h"

#include <cmath>

#include "utl/containers/Sort.h"

namespace {

// The resampled points per burst period, and the room reserved for points
// and bursts. Names not in the reference map.
constexpr int kPointsPerPeriod = 50;
constexpr unsigned long kReservedPoints = 1250;
constexpr unsigned long kReservedBursts = 25;
// The bursts the delay is taken from, and the spread they may have.
constexpr unsigned long kCandidates = 10;
constexpr float kMaxCandidateStdDev = 0.75f;
// No previous point yet.
constexpr float kNoPoint = -1000000.0f;

}  // namespace

// Reconstructed from eboot.elf at 0xF06030.
CalbertProcessor::CalbertProcessor()
    : mPeakThreshold(4.0f), mResampleInterval(0.0f), mBurstLength(0), mRiseWeight(1.0f) {
    Reset();
    mProgress = 0.0f;
    mDelay = 0.0f;
}

// Reconstructed from eboot.elf at 0xF06160.
void CalbertProcessor::Reset() {
    mBursts.clear();
    mResampled.clear();
    mResampled.push_back(0.0f);
    mLastTime = 0.0f;
    mPrevTime = 0.0f;
    mPrevValue = 0.0f;
    mProcessed = 0;
    mProgress = 0.0f;
    mDelay = 0.0f;
    mPrevPoint = kNoPoint;
}

// Reconstructed from eboot.elf at 0xF06230.
void CalbertProcessor::SetBurstInfo(float period, float length) {
    mResampleInterval = period * (1.0f / kPointsPerPeriod);
    mBurstLength = static_cast<int>(length * kPointsPerPeriod / period) + 1;
    mResampled.reserve(kReservedPoints);
    mBursts.reserve(kReservedBursts);
}

// Reconstructed from eboot.elf at 0xF06340.
void CalbertProcessor::SetParams(float peakThreshold, float riseWeight) {
    mPeakThreshold = peakThreshold;
    mRiseWeight = riseWeight;
}

// Reconstructed from eboot.elf at 0xF06350.
void CalbertProcessor::AddSamples(float time, float* samples, int count) {
    if (!(time >= mLastTime)) {
        Reset();
    }
    if (!(time > 0.0f)) {
        Reset();
        return;
    }
    if (count == 0) {
        return;
    }
    float sampleTime = time;
    if (count > 0) {
        const float step = (time - mLastTime) / count;
        for (int i = 0; i < count; i++) {
            sampleTime = (i + 1) * step + mLastTime;
            _Resample(sampleTime, samples[i]);
        }
    }
    mLastTime = sampleTime;
    _UpdateEstimate();
}

// Reconstructed from eboot.elf at 0xF065D0.
void CalbertProcessor::_Resample(float time, float value) {
    for (int point = static_cast<int>(mResampled.size()); point * mResampleInterval < time; point++) {
        const float fraction = (point * mResampleInterval - mPrevTime) / (time - mPrevTime);
        mResampled.push_back(fraction * (value - mPrevValue) + mPrevValue);
    }
    mPrevTime = time;
    mPrevValue = value;
}

// Reconstructed from eboot.elf at 0xF06760. The burst's mean must stand
// out from the rest of the period's (plus 0.00001); its position in the
// period is recorded.
void CalbertProcessor::_UpdateEstimate() {
    const int start = mProcessed;
    const int end = start + kPointsPerPeriod;
    if (mResampled.size() <= static_cast<unsigned long>(end)) {
        return;
    }
    float burstMean;
    const int burst = _GetBurstIndexAndMean(start, end, burstMean);
    const float restMean = _NonBurstMean(start, end, burst, burst + mBurstLength) + 0.00001f;
    if (burstMean / restMean > mPeakThreshold) {
        mBursts.push_back(static_cast<float>(burst - start));
        eastl::sort(mBursts.begin(), mBursts.end(), [](float a, float b) { return a < b; });
        _CalcDelayAndProgress();
    }
    mProcessed = end;
}

// Reconstructed from eboot.elf at 0xF06C10.
void CalbertProcessor::PrintResampled() {}

// Reconstructed from eboot.elf at 0xF06C20.
void CalbertProcessor::PrintResampled(int start, int end) {
    static_cast<void>(start);
    static_cast<void>(end);
}

// Reconstructed from eboot.elf at 0xF06C30.
float CalbertProcessor::GetProgress(float& delay) {
    if (mProgress == 1.0f) {
        delay = mDelay;
    }
    return mProgress;
}

// Reconstructed from eboot.elf at 0xF06C50.
int CalbertProcessor::_GetBurstIndexAndMean(int start, int end, float& mean) {
    mean = 0.0f;
    if (mPrevPoint == kNoPoint) {
        mPrevPoint = mResampled[start];
    }
    int best = 0;
    float bestScore = 0.0f;
    for (int i = start; i < end - mBurstLength; i++) {
        float sum = 0.0f;
        for (int j = i; j < i + mBurstLength; j++) {
            sum += mResampled[j];
        }
        const float burstMean = sum / mBurstLength;
        const float point = mResampled[i];
        const float rise = point - mPrevPoint;
        mPrevPoint = point;
        const float score = rise * mRiseWeight + burstMean;
        if (score >= bestScore) {
            best = i;
            bestScore = score;
            mean = burstMean;
        }
    }
    return best;
}

// Reconstructed from eboot.elf at 0xF06DD0.
float CalbertProcessor::_NonBurstMean(int start, int end, int burstStart, int burstEnd) {
    float sum = 0.0f;
    for (int i = start; i < burstStart; i++) {
        sum += mResampled[i];
    }
    for (int i = burstEnd; i < end; i++) {
        sum += mResampled[i];
    }
    return sum / ((end - start) + (burstStart - burstEnd));
}

// Reconstructed from eboot.elf at 0xF06F80. Below ten bursts the progress
// is a twentieth per burst; a wide spread gives 0.5 plus 0.375 over the
// spread.
void CalbertProcessor::_CalcDelayAndProgress() {
    if (mBursts.size() < kCandidates) {
        mProgress = mBursts.size() * 0.05f;
        return;
    }
    float mean;
    float stdDev;
    _GetCandidatesMeanStdDev(mean, stdDev);
    if (stdDev > kMaxCandidateStdDev) {
        mProgress = 0.375f / stdDev + 0.5f;
        return;
    }
    mDelay = mean * mResampleInterval;
    mProgress = 1.0f;
}

// Reconstructed from eboot.elf at 0xF07120.
void CalbertProcessor::_GetCandidatesMeanStdDev(float& mean, float& stdDev) {
    const int first = static_cast<int>(mBursts.size() - kCandidates) / 2;
    const int last = first + static_cast<int>(kCandidates);
    const float scale = 1.0f / (last - first);
    float sum = 0.0f;
    for (int i = first; i < last; i++) {
        sum += mBursts[i];
    }
    mean = sum * scale;
    float squares = 0.0f;
    for (int i = first; i < last; i++) {
        const float difference = mBursts[i] - mean;
        squares += difference * difference;
    }
    stdDev = std::sqrt(squares * scale);
}
