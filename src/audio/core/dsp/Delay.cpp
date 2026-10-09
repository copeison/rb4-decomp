#include "audio/core/dsp/Delay.h"

#include <cmath>

namespace {

// The specs at 0x124DD60 and 0x124DD6C, the map's 24 bytes of Delay.o
// read-only data. The wet and dry gains share the second. Names not in the
// reference map.
const SPL::ParameterSpec kFeedbackGainSpec = {0.0F, 0.0F, 1.0F};
const SPL::ParameterSpec kMixGainSpec = {0.0F, 1.0F, 1.0F};

}  // namespace

// Reconstructed from eboot.elf at 0xDA9F0. The line stays empty until
// Prepare.
Delay::Delay()
    : mNumChannels(0),
      mSampleRate(44100),
      mWritePos(0),
      mFeedbackGain(kFeedbackGainSpec),
      mWetGain(kMixGainSpec),
      mDryGain(kMixGainSpec),
      mDelayTime(0.0F),
      mBeatSync(false),
      mOutputGain(1.0F),
      mTempo(120.0F),
      mSpeed(1.0F),
      mFirstBlock(true) {
    SetWetGain(0.5F);
    SetDryGain(0.5F);
    _ResetRamps();
    FinishGainRamps();
}

// Reconstructed from eboot.elf at 0xDADD0. The line holds the longest delay
// and the interpolation's extra frame, rounded up to a power of two.
void Delay::Prepare(float sampleRate, unsigned int numChannels, float maxDelayMs) {
    mSampleRate = static_cast<int>(sampleRate);
    mExtraFrames = 1;
    _ResetRamps();
    FinishGainRamps();
    mMaxDelaySamples = std::ceil(sampleRate * 0.001F * maxDelayMs);
    unsigned int minLength = static_cast<unsigned int>(mMaxDelaySamples) + mExtraFrames;
    unsigned int length = 1;
    while (length < minLength) {
        length *= 2;
    }
    mBufferLength = length;
    mBufferMask = length - 1;
    mNumChannels = numChannels;
    mBuffer.Configure(
        AudioBufferConfig(numChannels, length, 0.0F, false), AudioBufferBase::kCleanupFree);
    mBuffer.Clear();
    mSpeed = 1.0F;
    _UpdateDelaySamples();
}

// Reconstructed from eboot.elf at 0xDB220.
void Delay::SetDelaySeconds(float seconds) {
    mDelayTime = seconds;
    _UpdateDelaySamples();
}

// Reconstructed from eboot.elf at 0xDB270.
void Delay::_ApplyNewParams() {
    if (mDelayRamp.mTarget != mDelaySamples) {
        mDelayRamp.SetTarget(mDelaySamples, nullptr, nullptr);
    }
    if (mWetRamp.mTarget != mWetGain.Get()) {
        mWetRamp.SetTarget(mWetGain.Get(), nullptr, nullptr);
    }
    if (mDryRamp.mTarget != mDryGain.Get()) {
        mDryRamp.SetTarget(mDryGain.Get(), nullptr, nullptr);
    }
    if (mFeedbackRamp.mTarget != mFeedbackGain.Get()) {
        mFeedbackRamp.SetTarget(mFeedbackGain.Get(), nullptr, nullptr);
    }
    if (mFirstBlock) {
        FinishGainRamps();
    }
}

// Reconstructed from eboot.elf at 0xDB510. Each channel runs the ramps from
// the same state; the last channel's ramps are kept.
void Delay::Process(AudioBuffer<float>& buffer) {
    int numFrames = buffer.mConfig.mNumFrames;
    _ApplyNewParams();
    mFirstBlock = false;
    SPL::Ramper wetRamp;
    SPL::Ramper dryRamp;
    SPL::Ramper delayRamp;
    SPL::Ramper feedbackRamp;
    for (int channel = 0; channel < mNumChannels; ++channel) {
        float* samples = buffer.mChannelData[channel];
        wetRamp = mWetRamp;
        dryRamp = mDryRamp;
        delayRamp = mDelayRamp;
        feedbackRamp = mFeedbackRamp;
        if (numFrames == 0) {
            continue;
        }
        float* line = mBuffer.mChannelData[channel];
        unsigned int mask = mBufferMask;
        unsigned int pos = mWritePos & mask;
        for (int frame = 0; frame < numFrames; ++frame) {
            float delay = delayRamp.mValue;
            unsigned int delayFrames = static_cast<unsigned int>(std::ceil(delay));
            float fraction = static_cast<float>(delayFrames) - delay;
            unsigned int read = (mBufferLength + pos - delayFrames) & mask;
            float input = samples[frame];
            float delayed = (1.0F - fraction) * line[read] + fraction * line[(read + 1) & mask];
            line[pos] = delayed * feedbackRamp.mValue + input;
            samples[frame] = (input * dryRamp.mValue + delayed * wetRamp.mValue) * mOutputGain;
            if ((frame & 31) == 0) {
                wetRamp.Advance();
                dryRamp.Advance();
                feedbackRamp.Advance();
            }
            if ((frame & 1) == 0) {
                delayRamp.Advance();
            }
            pos = (pos + 1) & mask;
            mask = mBufferMask;
        }
    }
    mDelayRamp = delayRamp;
    mFeedbackRamp = feedbackRamp;
    mWetRamp = wetRamp;
    mDryRamp = dryRamp;
    mWritePos = (mWritePos + numFrames) & mBufferMask;
}

// Reconstructed from eboot.elf at 0xDBEE0. The channels of a frame share
// the ramp values.
void Delay::Process(float* input, float* output, int numFrames, int numChannels) {
    _ApplyNewParams();
    mFirstBlock = false;
    SPL::Ramper wetRamp;
    SPL::Ramper dryRamp;
    SPL::Ramper delayRamp;
    SPL::Ramper feedbackRamp;
    wetRamp = mWetRamp;
    dryRamp = mDryRamp;
    delayRamp = mDelayRamp;
    feedbackRamp = mFeedbackRamp;
    float* lines[AudioBuffer<float>::kMaxChannels];
    for (int channel = 0; channel < numChannels; ++channel) {
        lines[channel] = mBuffer.mChannelData[channel];
    }
    unsigned int pos = mWritePos & mBufferMask;
    for (int frame = 0; frame < numFrames; ++frame) {
        unsigned int mask = mBufferMask;
        for (int channel = 0; channel < numChannels; ++channel) {
            float in = *input++;
            float delay = delayRamp.mValue;
            unsigned int delayFrames = static_cast<unsigned int>(std::ceil(delay));
            float fraction = static_cast<float>(delayFrames) - delay;
            unsigned int read = (mBufferLength + pos - delayFrames) & mask;
            float* line = lines[channel];
            float delayed = (1.0F - fraction) * line[read] + fraction * line[(read + 1) & mask];
            line[pos] = delayed * feedbackRamp.mValue + in;
            *output++ = (in * dryRamp.mValue + delayed * wetRamp.mValue) * mOutputGain;
        }
        if ((frame & 31) == 0) {
            wetRamp.Advance();
            dryRamp.Advance();
            feedbackRamp.Advance();
        }
        if ((frame & 1) == 0) {
            delayRamp.Advance();
        }
        pos = (pos + 1) & mask;
    }
    mDelayRamp = delayRamp;
    mFeedbackRamp = feedbackRamp;
    mWetRamp = wetRamp;
    mDryRamp = dryRamp;
    mWritePos = (mWritePos + numFrames) & mBufferMask;
}

// Reconstructed from eboot.elf at 0xDCA30.
void Delay::OnTempoChanged(float tempo, float speed) {
    mTempo = tempo;
    mSpeed = speed;
    if (mBeatSync) {
        _UpdateDelaySamples();
    }
}

// Reconstructed from eboot.elf at 0xDCA80.
void Delay::SetTempo(float tempo) {
    mTempo = tempo;
    if (mBeatSync) {
        _UpdateDelaySamples();
    }
}

// Reconstructed from eboot.elf at 0xDCAD0.
void Delay::SetSpeed(float speed) {
    mSpeed = speed;
    if (mBeatSync) {
        _UpdateDelaySamples();
    }
}

// Reconstructed from eboot.elf at 0xDCB20.
void Delay::SetBeatSync(bool beatSync) {
    mBeatSync = beatSync;
    _UpdateDelaySamples();
}

// Reconstructed from eboot.elf at 0xDCB70.
void Delay::UnitTest() {}
