#include "audio/core/output/AudioBus.h"

// Reconstructed from eboot.elf at 0x476C0.
AudioBusCallable::~AudioBusCallable() {}

// Reconstructed from eboot.elf at 0x47650.
bool AudioBusCallable::_MakeSamples(int, float, int, int, bool) {
    return true;
}

// Reconstructed from eboot.elf at 0x47660.
bool AudioBusCallable::Unknown4() {
    return false;
}

// Reconstructed from eboot.elf at 0xBEF90.
AudioBus::AudioBus() : mUnknown136(0), mUnknown140(0), mSampleRate(0.0), mSecondsPerSample(0.0), mOwner(nullptr) {}

// Reconstructed from eboot.elf at 0xBF0A0. The owner is told first, then the
// bus tears itself down.
AudioBus::~AudioBus() {
    if (mOwner != nullptr) {
        mOwner->OnBusDestroyed(this);
    }
    TearDown();
}

// Reconstructed from eboot.elf at 0xBF210.
void AudioBus::Prepare(float sampleRate, unsigned int numChannels, unsigned int blockSize, bool allocate) {
    mSampleRate = sampleRate;
    mSecondsPerSample = 1.0 / mSampleRate;
    mUnknown136 = 0;
    mUnknown140 = 0;
    mBlockSize = static_cast<int>(blockSize);
    mNumChannels = static_cast<int>(numChannels);
    if (blockSize != 0 && allocate) {
        mBuffer.Configure(
            AudioBufferConfig(static_cast<int>(numChannels), static_cast<int>(blockSize), sampleRate, false),
            AudioBufferBase::kCleanupFree);
    }
}

// Reconstructed from eboot.elf at 0xBF2B0.
void AudioBus::SetSampleRate(float sampleRate) {
    mSampleRate = sampleRate;
    mSecondsPerSample = 1.0 / mSampleRate;
    mBuffer.mConfig.SetSampleRate(sampleRate);
}
