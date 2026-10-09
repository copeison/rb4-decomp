#include "audio/core/dsp/IIRFilter.h"

#include <cstring>

// Reconstructed from eboot.elf at 0xE5060.
IIRFilter::IIRFilter(int order, float* b, float* a) {
    mOrder = order;
    mB = new float[order + 1];
    mA = new float[order + 1];
    mState = new float[order + 1];
    for (int i = 0; i <= order; ++i) {
        mA[i] = -a[i];
        mState[i] = 0.0f;
        mB[i] = b[i];
    }
}

// Reconstructed from eboot.elf at 0xE51E0.
IIRFilter::~IIRFilter() {
    delete[] mB;
    delete[] mA;
    delete[] mState;
}

// Reconstructed from eboot.elf at 0xE5220.
float IIRFilter::Filter(float sample) {
    float out = sample * mB[0] + mState[1];
    mState[0] = out;
    int i;
    for (i = 1; i < mOrder; ++i) {
        mState[i] = sample * mB[i] + out * mA[i] + mState[i + 1];
    }
    if (mOrder > 0) {
        mState[mOrder] = sample * mB[mOrder] + out * mA[mOrder];
    }
    return out;
}

// Reconstructed from eboot.elf at 0xE53C0. Clears only the first mOrder
// terms.
void IIRFilter::ResetState() {
    std::memset(mState, 0, mOrder * sizeof(float));
}

// Reconstructed from eboot.elf at 0xE53E0.
IIR4PoleFilter::IIR4PoleFilter(float* b, float* a) {
    for (int lane = 0; lane < 4; ++lane) {
        int term = lane == 0 ? 4 : lane;
        mGain[lane] = lane == 0 ? b[0] : 0.0f;
        mShift[lane] = lane == 0 ? 0.0f : 1.0f;
        mB[lane] = b[term];
        mA[lane] = -a[term];
        mFolded[lane] = -(b[0] * a[term]);
        mState[lane] = 0.0f;
    }
}

// Reconstructed from eboot.elf at 0xE54D0.
void IIR4PoleFilter::Begin() {}

// Reconstructed from eboot.elf at 0xE54E0.
void IIR4PoleFilter::End() {}

// Reconstructed from eboot.elf at 0xE54F0.
float IIR4PoleFilter::FilterSlow(float sample) {
    float out = sample * mGain[0] + mState[0];
    mState[0] = out * mA[1] + sample * mB[1] + mState[1];
    mState[1] = out * mA[2] + sample * mB[2] + mState[2];
    mState[2] = out * mA[3] + sample * mB[3] + mState[3];
    mState[3] = out * mA[0] + sample * mB[0];
    return out;
}

// Reconstructed from eboot.elf at 0xE5560.
float IIR4PoleFilter::FilterFast(float sample) {
    float first = mState[0];
    mLastState = first;
    float out = sample * mGain[0] + first;
    mState[0] = (mFolded[1] + mB[1]) * sample + first * mA[1] + mState[1];
    mState[1] = (mFolded[2] + mB[2]) * sample + first * mA[2] + mState[2];
    mState[2] = (mFolded[3] + mB[3]) * sample + first * mA[3] + mState[3];
    mState[3] = (mFolded[0] + mB[0]) * sample + first * mA[0];
    return out;
}

// Reconstructed from eboot.elf at 0xE5600.
float IIR4PoleFilter::FilterProtoFast(float sample) {
    float first = mState[0];
    mLastState = first;
    float out = sample * mGain[0] + first;
    mState[0] = (mFolded[1] + mB[1]) * sample + first * mA[1] + mState[1];
    mState[1] = (mFolded[2] + mB[2]) * sample + first * mA[2] + mState[2];
    mState[2] = (mFolded[3] + mB[3]) * sample + first * mA[3] + mState[3];
    mState[3] = (mFolded[0] + mB[0]) * sample + first * mA[0];
    return out;
}
