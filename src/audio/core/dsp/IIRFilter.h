#pragma once

#include <cstddef>

// Transposed direct-form II filter of any order (audio/IIRFilter.o). The
// coefficient arrays hold order + 1 terms; the constructor negates the
// feedback terms. Field names are not in the reference map.
class IIRFilter {
public:
    IIRFilter(int order, float* b, float* a);  // 0xE5060
    ~IIRFilter();                              // 0xE51E0

    float Filter(float sample);  // 0xE5220
    void ResetState();           // 0xE53C0

    int mOrder;
    float* mA;      // Negated feedback coefficients.
    float* mB;      // Feed-forward coefficients.
    float* mState;  // order + 1 delay terms.
};

static_assert(offsetof(IIRFilter, mA) == 8);
static_assert(offsetof(IIRFilter, mState) == 24);
static_assert(sizeof(IIRFilter) == 32);

// Fourth-order transposed direct-form II filter laid out for four-wide
// arithmetic (audio/IIRFilter.o). Lane k of each coefficient vector holds
// the term of order k, with lane 0 holding the fourth-order term, so lane k
// updates delay term k - 1. PitchDetector creates one as its decimation
// low-pass. Field names are not in the reference map.
class IIR4PoleFilter {
public:
    // Takes five feed-forward and five feedback coefficients; a[0] is
    // assumed to be 1. At 0xE53E0.
    IIR4PoleFilter(float* b, float* a);

    void Begin();  // 0xE54D0, empty in this build.
    void End();    // 0xE54E0, empty in this build.
    // Feeds one sample through the delay line in order. At 0xE54F0.
    float FilterSlow(float sample);
    // Update every delay term from the previous first term, using the
    // folded coefficients; FilterProtoFast is the same code. At 0xE5560 and
    // 0xE5600.
    float FilterFast(float sample);
    float FilterProtoFast(float sample);

    float mGain[4];   // b[0] in lane 0, zero elsewhere.
    float mShift[4];  // 0 in lane 0 and 1 elsewhere; not read in this build.
    float mB[4];      // Feed-forward terms, rotated as described above.
    float mFolded[4];  // -b[0] * a[k]: the feedback folded into the input.
    float mA[4];      // Negated feedback terms.
    float mState[4];  // Delay terms.
    float mLastState;  // First delay term before the latest fast update.
};

static_assert(offsetof(IIR4PoleFilter, mB) == 32);
static_assert(offsetof(IIR4PoleFilter, mA) == 64);
static_assert(offsetof(IIR4PoleFilter, mState) == 80);
static_assert(offsetof(IIR4PoleFilter, mLastState) == 96);
static_assert(sizeof(IIR4PoleFilter) == 100);
