#include "audio/core/buffers/AudioBuffer.h"

#include <cmath>

namespace {

// Guard sample written before and after owned sample data: 0xF00D for
// 16-bit samples and the value 0xFEDCF00D rounded for float ones. Name not
// in the reference map.
constexpr unsigned int kAudioBufferGuard = 0xFEDCF00D;
constexpr float kShortScale = 32768.0F;
constexpr float kInverseShortScale = 1.0F / 32768.0F;

}  // namespace

// Reconstructed from eboot.elf at 0xD3D10.
AudioBufferConfig::AudioBufferConfig() : mSampleRate(0.0F), mNumChannels(0), mNumFrames(0), mInterleaved(false) {}

// Reconstructed from eboot.elf at 0xD3D20.
AudioBufferConfig::AudioBufferConfig(int numChannels, int numFrames, float sampleRate, bool interleaved)
    : mSampleRate(sampleRate), mNumChannels(numChannels), mNumFrames(numFrames), mInterleaved(interleaved) {}

// Reconstructed from eboot.elf at 0xD3D30.
int AudioBufferConfig::GetNumTotalSamples() const {
    return mNumFrames * mNumChannels;
}

// Reconstructed from eboot.elf at 0xD3D40.
float AudioBufferConfig::GetSampleRate() const {
    return mSampleRate;
}

// Reconstructed from eboot.elf at 0xD3D50.
void AudioBufferConfig::SetSampleRate(float sampleRate) {
    mSampleRate = sampleRate;
}

// Reconstructed from eboot.elf at 0xD3D60.
void AudioBufferConfig::SetNumChannels(int numChannels) {
    mNumChannels = numChannels;
}

// Reconstructed from eboot.elf at 0xD3D70.
void AudioBufferConfig::SetNumFrames(int numFrames) {
    mNumFrames = numFrames;
}

// Reconstructed from eboot.elf at 0xD3D80.
bool AudioBufferConfig::operator==(const AudioBufferConfig& other) const {
    return mNumChannels == other.mNumChannels && mNumFrames == other.mNumFrames &&
           mSampleRate == other.mSampleRate && mInterleaved == other.mInterleaved;
}

// Reconstructed from eboot.elf at 0xD3DB0.
AudioBufferConfig AudioBufferConfig::GetInterleavedConfig() const {
    return AudioBufferConfig(mNumChannels, mNumFrames, mSampleRate, true);
}

// Reconstructed from eboot.elf at 0xD3DD0.
AudioBufferConfig AudioBufferConfig::GetDeinterleavedConfig() const {
    return AudioBufferConfig(mNumChannels, mNumFrames, mSampleRate, false);
}

// Reconstructed from eboot.elf at 0xD3DE0.
const char* GetAcsiiArtLevel(float level) {
    if (level < -1.0F) {
        return "#----<    |";
    }
    if (level < -0.875F) {
        return "|o---<    |";
    }
    if (level < -0.625F) {
        return "| o--<    |";
    }
    if (level < -0.375F) {
        return "|  o-<    |";
    }
    if (level < -0.125F) {
        return "|   o<    |";
    }
    if (level < 0.0F) {
        return "|    <    |";
    }
    if (level == 0.0F) {
        return "|    .    |";
    }
    if (level < 0.125F) {
        return "|    >    |";
    }
    if (level < 0.375F) {
        return "|    >o   |";
    }
    if (level < 0.625F) {
        return "|    >-o  |";
    }
    if (level < 0.875F) {
        return "|    >--o |";
    }
    return level > 1.0F ? "|    >----#" : "|    >---o|";
}

// Reconstructed from eboot.elf at 0xD3EC0.
void AudioBufferBase::Print(const short* samples, unsigned long count) {
    for (unsigned long i = 0; i < count; ++i) {
        float value = samples[i] * kInverseShortScale;
        if (value != 0.0F) {
            static_cast<void>(std::log10(std::fabs(value)));
        }
    }
}

// Reconstructed from eboot.elf at 0xD3F20.
void AudioBufferBase::Print(const float* samples, unsigned long count) {
    for (unsigned long i = 0; i < count; ++i) {
        if (samples[i] != 0.0F) {
            static_cast<void>(std::log10(std::fabs(samples[i])));
        }
    }
}

// Reconstructed from eboot.elf at 0xD3F70.
void AudioBufferConvert(AudioBuffer<short>& dst, const AudioBuffer<float>& src) {
    for (int channel = 0; channel < dst.mConfig.mNumChannels; ++channel) {
        short* out = dst.mChannelData[channel];
        const float* in = src.mChannelData[channel];
        for (int frame = 0; frame < dst.mConfig.mNumFrames; ++frame) {
            out[frame] = static_cast<short>(static_cast<int>(in[frame] * kShortScale));
        }
    }
}

// Reconstructed from eboot.elf at 0xD40E0.
void AudioBufferConvert(AudioBuffer<float>& dst, const AudioBuffer<short>& src) {
    for (int channel = 0; channel < src.mConfig.mNumChannels; ++channel) {
        float* out = dst.mChannelData[channel];
        const short* in = src.mChannelData[channel];
        for (int frame = 0; frame < src.mConfig.mNumFrames; ++frame) {
            out[frame] = in[frame] * kInverseShortScale;
        }
    }
}

// Reconstructed from eboot.elf at 0x13720 for float samples and 0x135B0 for
// 16-bit ones.
template <class T>
void AudioBuffer<T>::Configure(AudioBufferConfig config, CleanupMode cleanup) {
    Release();
    mCleanupMode = cleanup;
    mConfig = config;
    if (cleanup == kCleanupFree) {
        int numSamples = config.mNumChannels * config.mNumFrames;
        T* block = static_cast<T*>(MemAlloc(static_cast<long>(numSamples + 2) * sizeof(T), "AudioBuffer data", 0));
        block[0] = static_cast<T>(kAudioBufferGuard);
        block[mConfig.mNumChannels * mConfig.mNumFrames + 1] = static_cast<T>(kAudioBufferGuard);
        T* data = block + 1;
        if (mConfig.mInterleaved) {
            mChannelData[0] = data;
        } else {
            for (int channel = 0; channel < mConfig.mNumChannels; ++channel) {
                mChannelData[channel] = data;
                data += mConfig.mNumFrames;
            }
        }
    }
    mNumFrames = config.mNumFrames;
    mNumChannels = config.mNumChannels;
    mFirstValidFrame = 0;
    mNumValidFrames = config.mNumFrames;
    mLastFrame = config.mNumFrames - 1;
    mCleared = false;
}

template void AudioBuffer<short>::Configure(AudioBufferConfig config, CleanupMode cleanup);
template void AudioBuffer<float>::Configure(AudioBufferConfig config, CleanupMode cleanup);
