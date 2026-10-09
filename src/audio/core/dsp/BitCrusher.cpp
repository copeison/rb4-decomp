#include "audio/core/dsp/BitCrusher.h"

#include <cmath>

namespace {

// The sample-and-hold factor's range, at 0x124DCF8. Name not in the
// reference map.
const SPL::ParameterSpec kSampleHoldFactorSpec = {1.0f, 1.0f, 16.0f};

}  // namespace

// Reconstructed from eboot.elf at 0xDA390.
BitCrusher::BitCrusher()
    : mNumChannels(0),
      mCrush(0.0f),
      mWet(0.0f),
      mJumpWet(true),
      mSampleHoldFactor(kSampleHoldFactorSpec),
      mCrushBits(0) {}

// Reconstructed from eboot.elf at 0xDA410.
void BitCrusher::SetCrush(float bits, bool force) {
    if (bits != mCrush || force) {
        mCrush = bits;
        mCrushBits = static_cast<short>(bits);
    }
}

// Reconstructed from eboot.elf at 0xDA430.
void BitCrusher::SetWet(float percent, bool jump) {
    mWet = percent;
    mJumpWet = mJumpWet || jump;
}

// Reconstructed from eboot.elf at 0xDA450.
BitCrusher::~BitCrusher() {}

// Reconstructed from eboot.elf at 0xDA460. The wet ramp takes 2 ms.
void BitCrusher::Setup(int numChannels, int sampleRate) {
    mSampleRate = sampleRate;
    float rate = static_cast<float>(sampleRate);
    mWetRamp.mRate = rate;
    mWetRamp.mIncrement = 500.0f / rate;
    mNumChannels = numChannels;
    ClearHistory();
}

// Reconstructed from eboot.elf at 0xDA4A0.
void BitCrusher::ClearHistory() {
    mHoldCounter = 0;
    for (float& sample : mHeldSamples) {
        sample = 0.0f;
    }
}

// Reconstructed from eboot.elf at 0xDA4C0.
void BitCrusher::SetSampleHoldFactor(unsigned short factor) {
    mSampleHoldFactor.Set(static_cast<float>(factor));
}

// Reconstructed from eboot.elf at 0xDA4F0. Each channel starts from the
// stored hold counter and wet ramp, and the last channel's state is kept.
void BitCrusher::Process(AudioBuffer<float>& input, AudioBuffer<float>& output) {
    int numFrames = input.mNumFrames;
    // Function-local statics under a guard; the compiler folds the first
    // initializer to 32767. Names not in the reference map.
    static const float sMaxSample = static_cast<float>(std::pow(2.0, 15.0) - 1.0);
    static const float sInvMaxSample = 1.0f / sMaxSample;

    float wet = mWet * 0.01f;
    if (wet != mWetRamp.mTarget) {
        mWetRamp.SetTarget(wet, nullptr, nullptr);
        if (mJumpWet) {
            mWetRamp.Finish();
        }
    }

    unsigned short holdCounter = mHoldCounter;
    SPL::Ramper ramp;
    unsigned short holdFactor = static_cast<unsigned short>(mSampleHoldFactor.Get());
    for (int channel = 0; channel < mNumChannels; ++channel) {
        holdCounter = mHoldCounter;
        const float* in;
        int inStride;
        if (input.mConfig.mInterleaved) {
            in = input.mChannelData[0] + channel;
            inStride = input.mConfig.mNumChannels;
        } else {
            in = input.mChannelData[channel];
            inStride = 1;
        }
        float* out;
        int outStride;
        if (output.mConfig.mInterleaved) {
            out = output.mChannelData[0] + channel;
            outStride = output.mConfig.mNumChannels;
        } else {
            out = output.mChannelData[channel];
            outStride = 1;
        }
        ramp = mWetRamp;
        for (int frame = 0; frame < numFrames; ++frame) {
            float wetGain = ramp.mValue;
            float dryGain = 1.0f - wetGain;
            float sample = *in;
            sample = sample > 1.0f ? 1.0f : (sample > -1.0f ? sample : -1.0f);
            float crushed;
            if (holdCounter == 0) {
                short bits = static_cast<short>(static_cast<int>(sample * sMaxSample));
                bits = static_cast<short>((bits >> mCrushBits) << mCrushBits);
                crushed = static_cast<float>(bits) * sInvMaxSample;
                mHeldSamples[channel] = crushed;
            } else {
                crushed = mHeldSamples[channel];
            }
            if (++holdCounter > holdFactor) {
                holdCounter = 0;
            }
            *out = crushed * wetGain + sample * dryGain;
            ramp.Advance();
            in += inStride;
            out += outStride;
        }
    }
    mHoldCounter = holdCounter;
    mWetRamp = ramp;
}
