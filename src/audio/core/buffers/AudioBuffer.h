#pragma once

#include <cstddef>
#include <cstring>

#include "os/memory/MemMgr.h"
#include "utl/containers/LinkedList.h"

// Shape of an audio buffer: the sample rate, the channel and frame counts and
// whether the samples are interleaved (audio/AudioBuffer.o in the map). Field
// names are not in the reference map.
class AudioBufferConfig {
public:
    AudioBufferConfig();  // 0xD3D10
    AudioBufferConfig(int numChannels, int numFrames, float sampleRate, bool interleaved);  // 0xD3D20

    int GetNumTotalSamples() const;  // 0xD3D30
    float GetSampleRate() const;     // 0xD3D40
    // The map's SetSampleRate(float) also recomputes derived values; this
    // build only stores the rate.
    void SetSampleRate(float sampleRate);         // 0xD3D50
    void SetNumChannels(int numChannels);         // 0xD3D60
    void SetNumFrames(int numFrames);             // 0xD3D70
    bool operator==(const AudioBufferConfig& other) const;  // 0xD3D80
    AudioBufferConfig GetInterleavedConfig() const;         // 0xD3DB0
    AudioBufferConfig GetDeinterleavedConfig() const;       // 0xD3DD0

    float mSampleRate;
    int mNumChannels;
    int mNumFrames;
    bool mInterleaved;
};

static_assert(offsetof(AudioBufferConfig, mNumChannels) == 4);
static_assert(offsetof(AudioBufferConfig, mInterleaved) == 12);
static_assert(sizeof(AudioBufferConfig) == 16);

class AudioBufferBase {
public:
    // Trace helpers whose output the release build strips; only the level
    // conversion of each nonzero sample remains.
    static void Print(const short* samples, unsigned long count);  // 0xD3EC0
    static void Print(const float* samples, unsigned long count);  // 0xD3F20

    // Whether the buffer frees its channel data. Values not in the reference
    // map.
    enum CleanupMode : int {
        kCleanupFree = 0,
        kCleanupNone = 1,
    };
};

// Planar or interleaved sample storage. Owned data is one MemAlloc block
// with a guard word on each side; the channel pointers address its interior.
// Only the members the reconstructed audio code reaches are declared.
template <class T>
class AudioBuffer : public AudioBufferBase {
public:
    static constexpr int kMaxChannels = 8;  // Name not in the reference map.

    // Inlined into every owner's constructor, for example AudioBus's at
    // 0xBEF90 and AudioBusGenerator's at 0xE0490.
    AudioBuffer() : mCleanupMode(kCleanupNone) {
        Release();
    }
    // Inlined, for example into AudioBus's destructor at 0xBF0A0.
    ~AudioBuffer() {
        Release();
    }

    // Frees the old data and takes the new shape; a kCleanupFree buffer
    // allocates its samples. At 0x13720 for float samples and 0x135B0 for
    // 16-bit ones, which the reconstruction instantiates in AudioBuffer.cpp.
    void Configure(AudioBufferConfig config, CleanupMode cleanup);

    // Frees owned data and resets the shape, as the destructor and Configure
    // inline it. Name not in the reference map.
    void Release() {
        if (mCleanupMode == kCleanupFree) {
            MemFree(mChannelData[0] - 1);
            mChannelData[0] = nullptr;
        }
        for (auto& channel : mChannelData) {
            channel = nullptr;
        }
        mConfig = AudioBufferConfig(0, 0, 0.0F, false);
        mCleanupMode = kCleanupNone;
        mNumChannels = 0;
        mNumFrames = 0;
        mFirstValidFrame = 0;
        mNumValidFrames = 0;
        mLastFrame = 0;
        mCleared = true;
    }

    // Silences the samples. Inlined, for example into
    // FmodAudioBusGenerator::_MakeSamples at 0x267790. Name not in the
    // reference map.
    void Clear() {
        if (mCleanupMode != kCleanupFree && !mConfig.mInterleaved) {
            for (int channel = 0; channel < mNumChannels; ++channel) {
                std::memset(mChannelData[channel], 0, sizeof(T) * mConfig.mNumFrames);
            }
        } else {
            std::memset(mChannelData[0], 0, sizeof(T) * mConfig.GetNumTotalSamples());
        }
        mCleared = true;
    }
    // Points the channels `offset` frames into another buffer's data, as a
    // non-owning view. Inlined into FmodAudioBusGenerator::_MakeSamples at
    // 0x267790. Name not in the reference map.
    void SetChannelData(const AudioBuffer& source, int offset) {
        int numChannels = mConfig.mNumChannels == 0x7FFFFFFF ? source.mNumChannels : mConfig.mNumChannels;
        mNumChannels = numChannels;
        if (numChannels > 0) {
            std::memcpy(mChannelData, source.mChannelData, sizeof(T*) * numChannels);
            for (int channel = 0; channel < mNumChannels; ++channel) {
                mChannelData[channel] += offset;
            }
        }
    }

    // Adds planar samples to the channels. Inline in the map's build, which
    // emits it in audio/FusionSampler.o; FusionSampler::Process at 0x9ACC0
    // inlines it.
    void Accumulate(T** channels, int numChannels, int numFrames) {
        for (int channel = 0; channel < numChannels; ++channel) {
            T* output = mChannelData[channel];
            const T* input = channels[channel];
            for (int frame = 0; frame < numFrames; ++frame) {
                output[frame] += input[frame];
            }
        }
    }

    // Field names are not in the reference map.
    LinkedList::Node mNode;  // Unlinks itself when the buffer is destroyed.
    CleanupMode mCleanupMode;
    T* mChannelData[kMaxChannels];
    AudioBufferConfig mConfig;
    int mNumChannels;
    int mNumFrames;
    int mFirstValidFrame;
    int mNumValidFrames;
    int mLastFrame;
    bool mCleared;  // Set while the samples are known to be silent.
};

static_assert(offsetof(AudioBuffer<float>, mCleanupMode) == 16);
static_assert(offsetof(AudioBuffer<float>, mChannelData) == 24);
static_assert(offsetof(AudioBuffer<float>, mConfig) == 88);
static_assert(offsetof(AudioBuffer<float>, mNumChannels) == 104);
static_assert(offsetof(AudioBuffer<float>, mNumValidFrames) == 116);
static_assert(offsetof(AudioBuffer<float>, mCleared) == 124);
static_assert(sizeof(AudioBuffer<float>) == 128);

// Converts between 16-bit and float samples, channel by channel, at a scale
// of 32768. Each uses the shape of its 16-bit buffer.
void AudioBufferConvert(AudioBuffer<short>& dst, const AudioBuffer<float>& src);  // 0xD3F70
void AudioBufferConvert(AudioBuffer<float>& dst, const AudioBuffer<short>& src);  // 0xD40E0

// Draws a sample in [-1, 1] as an 11-character meter. At 0xD3DE0; the map
// spells the name this way.
const char* GetAcsiiArtLevel(float level);
