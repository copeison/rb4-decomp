#include "math/smoothing/DoubleExponentialSmoother.h"

namespace {

// A rate times the time step, clamped to [0, 1]. Name not in the reference
// map.
float StepWeight(float rate, float timeStep) {
    float weight = timeStep * rate;
    return 1.0F < weight ? 1.0F : (0.0F > weight ? 0.0F : weight);
}

}  // namespace

// Reconstructed from eboot.elf at 0x1172A10.
DoubleExponentialSmoother::DoubleExponentialSmoother()
    : mValue(0.0F), mLevel(0.0F), mTrend(0.0F), mLevelRate(0.0F), mTrendRate(0.0F) {}

// Reconstructed from eboot.elf at 0x1172A20.
DoubleExponentialSmoother::DoubleExponentialSmoother(float value, float levelRate, float trendRate)
    : mValue(value), mLevel(value), mTrend(0.0F), mLevelRate(levelRate), mTrendRate(trendRate) {}

// Reconstructed from eboot.elf at 0x1172A40.
void DoubleExponentialSmoother::Smooth(float sample, float timeStep) {
    float levelWeight = StepWeight(mLevelRate, timeStep);
    float trendWeight = StepWeight(mTrendRate, timeStep);
    float level = levelWeight * (sample - mValue) + mValue;
    float trend = (-mTrend - mLevel + level) * trendWeight + mTrend;
    mTrend = trend;
    mLevel = level;
    mValue = trend + level;
}

// Reconstructed from eboot.elf at 0x1172AD0.
void DoubleExponentialSmoother::ForceValue(float value, bool reset) {
    mValue = value;
    if (reset) {
        mLevel = value;
        mTrend = 0.0F;
    }
}
