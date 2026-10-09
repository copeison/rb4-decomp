#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "utl/containers/LinkedList.h"

// Shape of an audio buffer: the sample rate, the channel and frame counts and
// whether the samples are interleaved. The members are out of line at
// 0xD3D10 through 0xD3D50 (audio/AudioBuffer.o in the map). Field names are
// not in the reference map.
class AudioBufferConfig {
public:
    AudioBufferConfig();  // 0xD3D10
    AudioBufferConfig(int numChannels, int numFrames, float sampleRate, bool interleaved);  // 0xD3D20

    int GetNumTotalSamples() const;  // 0xD3D30
    float GetSampleRate() const;     // 0xD3D40
    // The map's SetSampleRate(float) also recomputes derived values; this
    // build only stores the rate.
    void SetSampleRate(float sampleRate);  // 0xD3D50

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

    // At 0x13720 for float samples.
    void Configure(AudioBufferConfig config, CleanupMode cleanup);

    // Frees owned data and resets the shape, as the destructor and Configure
    // inline it. Name not in the reference map.
    void Release() {
        if (mCleanupMode == kCleanupFree) {
            MemFree(reinterpret_cast<unsigned int*>(mChannelData[0]) - 1);
            mChannelData[0] = nullptr;
        }
        for (auto& channel : mChannelData) {
            channel = nullptr;
        }
        mConfig = AudioBufferConfig(0, 0, 0.0F, false);
        mCleanupMode = kCleanupNone;
        mNumChannels = 0;
        mNumFrames = 0;
        mUnknown112 = 0;
        mNumValidFrames = 0;
        mLastFrame = 0;
        mCleared = true;
    }

    // Field names are not in the reference map.
    LinkedList::Node mNode;  // Unlinks itself when the buffer is destroyed.
    CleanupMode mCleanupMode;
    T* mChannelData[kMaxChannels];
    AudioBufferConfig mConfig;
    int mNumChannels;
    int mNumFrames;
    int mUnknown112;
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
