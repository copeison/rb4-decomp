#pragma once

#include <cstddef>

// Holt's double exponential smoothing of a scalar: a level and a trend, each
// blended toward new samples by a rate scaled with the elapsed time
// (math/DoubleExponentialSmoother.o). The object's Vector2DESmoother and
// Vector3DESmoother variants are not reconstructed. Field names are not in
// the reference map.
class DoubleExponentialSmoother {
public:
    DoubleExponentialSmoother();  // 0x1172A10
    DoubleExponentialSmoother(float value, float levelRate, float trendRate);  // 0x1172A20

    // Blends the level toward the sample and the trend toward the level's
    // change; each rate is multiplied by the time step and clamped to 1.
    void Smooth(float sample, float timeStep);  // 0x1172A40
    // Sets the output; a reset also restarts the level and the trend.
    void ForceValue(float value, bool reset);  // 0x1172AD0

    float mValue;  // Level plus trend.
    float mLevel;
    float mTrend;
    float mLevelRate;
    float mTrendRate;
};

static_assert(sizeof(DoubleExponentialSmoother) == 20);
