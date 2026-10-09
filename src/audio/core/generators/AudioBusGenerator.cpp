#include "audio/core/generators/AudioBusGenerator.h"

#include <cmath>
#include <cstring>

Symbol AudioBusGenerator::sTypeId;

// Reconstructed from eboot.elf at 0xE0490. The block buffer holds one stereo
// engine buffer. With a source the generator is set up at once with default
// arguments.
AudioBusGenerator::AudioBusGenerator(AudioBus* source)
    : mSource(nullptr) {
    mBuffer.mHasSamples = false;
    mBuffer.Configure(
        AudioBufferConfig(2, Audio::sBufferSize, 0.0F, false), AudioBufferBase::kCleanupFree);
    if (source != nullptr) {
        Setup(source, PlayArgs(), nullptr);
    }
    _ResetGainAndMute(1.0F, false);
}

// Reconstructed from eboot.elf at 0xE0A60.
AudioBusGenerator::~AudioBusGenerator() {}

// Reconstructed from eboot.elf at 0xE0900.
void AudioBusGenerator::_ResetGainAndMute(float gain, bool mute) {
    mMuted = mute;
    mMuteRamp.SetDurationMs(kMinGainRampMs, BlocksPerSecond());
    mMuteRamp.SetTarget(mute ? 0.0F : 1.0F);
    mMuteRamp.Snap();
    mGainRamp.SetTarget(gain);
    mGainRamp.Snap();
    mGainFadeMode = kPostFadeNone;
}

// Reconstructed from eboot.elf at 0xE0C70. The binary returns no defined
// value; every caller ignores it.
bool AudioBusGenerator::Setup(AudioBus* source, const PlayArgs& args, AudioBusCallable* callback) {
    mMixer = mRenderTarget->GetMixer();
    mState = args.mStartPaused ? kStatePaused : kStatePlaying;
    mSource = source;
    mCallback = callback;
    mIsSetUp = true;

    if (mBuffer.mCleanupMode != AudioBufferBase::kCleanupFree && !mBuffer.mConfig.mInterleaved) {
        for (int channel = 0; channel < mBuffer.mNumChannels; ++channel) {
            std::memset(
                mBuffer.mChannelData[channel], 0, sizeof(float) * mBuffer.mConfig.mNumFrames);
        }
    } else {
        std::memset(mBuffer.mChannelData[0], 0, sizeof(float) * mBuffer.mConfig.GetNumTotalSamples());
    }
    mBuffer.mCleared = true;
    mBuffer.mHasSamples = false;

    _ResetGainAndMute(1.0F, args.mStartMuted);
    if (args.mHasInitialGain) {
        SetGain(
            std::pow(10.0F, args.mInitialGainDb * 0.05F),
            args.mInitialGainFadeSecs,
            static_cast<PostFadeOption>(args.mInitialGainPostFade));
    }
    return true;
}

// Reconstructed from eboot.elf at 0xE1260. A fade takes at least the
// minimum ramp; a paused voice takes an immediate change at once.
void AudioBusGenerator::SetGain(float gain, float fadeSecs, PostFadeOption option) {
    const float fadeMs = fadeSecs * 1000.0F;
    mGainRamp.SetDurationMs(fadeMs > kMinGainRampMs ? fadeMs : kMinGainRampMs, BlocksPerSecond());
    mGainRamp.SetTarget(gain);
    if (fadeSecs == 0.0F && mState == kStatePaused) {
        mGainRamp.Snap();
    }
    mGainFadeMode = option;
}

// Reconstructed from eboot.elf at 0xE1360.
void AudioBusGenerator::SetMute(bool mute, bool immediate) {
    mMuted = mute;
    mMuteRamp.SetTarget(mute ? 0.0F : 1.0F);
    if (immediate) {
        mMuteRamp.Snap();
    }
}

// Reconstructed from eboot.elf at 0xE13F0.
bool AudioBusGenerator::IsVirtualInstrument() {
    if (mSource == nullptr) {
        return false;
    }
    return mSource->IsVirtualInstrument();
}

// Reconstructed from eboot.elf at 0xE1430.
float AudioBusGenerator::GetGain() const {
    return mGainRamp.mValue;
}

// Reconstructed from eboot.elf at 0xE1440.
bool AudioBusGenerator::GetMute() const {
    return mMuted;
}

// Reconstructed from eboot.elf at 0xE1450.
void AudioBusGenerator::_InitTypeId() {
    sTypeId = Symbol("AudioBusGenerator");
}

// Reconstructed from eboot.elf at 0xE14A0.
AudioGenerator* AudioBusGenerator::GetGeneratorOfType(Symbol type) {
    return type == sTypeId ? this : nullptr;
}

// Reconstructed from eboot.elf at 0xE14C0.
Symbol AudioBusGenerator::GetTypeId() {
    return sTypeId;
}

// Reconstructed from eboot.elf at 0xE14D0.
bool AudioBusGenerator::_PrepareToMakeSamples(int, float, int, int, bool) {
    return false;
}

// Reconstructed from eboot.elf at 0xE14E0.
bool AudioBusGenerator::_MakeSamples(int, float, int, int, bool) {
    return false;
}
