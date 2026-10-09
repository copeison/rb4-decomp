#pragma once

#include <cstddef>
#include <cstring>

#include "audio/core/analysis/PitchDetector.h"
#include "os/memory/MemMgr.h"
#include "utl/text/Symbol.h"

struct PlayArgs;

// Ring of 16-bit samples (mic/Mic.o in the map). Its members are inline in
// this build. Field names are not in the reference map.
class RingBuffer {
public:
    RingBuffer() : mSize(0), mBuffer(nullptr), mWritePos(0), mReadPos(0) {}
    // Inlined into Mic's destructor at 0xE1530.
    ~RingBuffer() {
        if (mBuffer != nullptr) {
            MemFree(mBuffer);
            mBuffer = nullptr;
        }
    }

    // Allocates a silent ring of the size. Inlined into Mic's constructor,
    // emitted at 0x27B500.
    void Init(int size) {
        mSize = size;
        if (mBuffer != nullptr) {
            MemFree(mBuffer);
            mBuffer = nullptr;
        }
        mBuffer = static_cast<short*>(MemAlloc(sizeof(short) * mSize, "VirtualMic RingBuffer", 128));
        std::memset(mBuffer, 0, sizeof(short) * mSize);
        mWritePos = 0;
        mReadPos = 0;
    }
    // Silences the ring and rewinds both positions. Inlined into
    // Mic::ResetBuffersAndStart at 0xE15F0.
    void Reset() {
        std::memset(mBuffer, 0, sizeof(short) * mSize);
        mWritePos = 0;
        mReadPos = 0;
    }
    // Appends every `stride`-th sample and returns the number of unread
    // samples overwritten or skipped. The read position does not move. The
    // map has Write(void*, int); this build adds the stride.
    int Write(const short* data, int count, int stride);

    int mSize;
    short* mBuffer;
    int mWritePos;
    int mReadPos;
};

static_assert(offsetof(RingBuffer, mBuffer) == 8);
static_assert(offsetof(RingBuffer, mWritePos) == 16);
static_assert(sizeof(RingBuffer) == 24);

// Reconstructed from eboot.elf at 0x27C190, the copy emitted with Mic_FMOD.
// Samples beyond the ring size are skipped from the start of the data; that
// offset ignores the stride.
inline int RingBuffer::Write(const short* data, int count, int stride) {
    int skipped = 0;
    int excess = count - mSize;
    const short* source = data + excess;
    if (excess >= 0) {
        count = mSize;
    }
    if (excess <= 0) {
        excess = 0;
        source = data;
    }
    int used = 0;
    if (mWritePos != mReadPos) {
        used = mWritePos - mReadPos;
        if (used <= 0) {
            used += mSize;
        }
    }
    int overwritten = count - (mSize - used);
    if (overwritten > 0) {
        skipped = overwritten;
    }
    int tail = mSize - mWritePos;
    int first = count < tail ? count : tail;
    if (stride == 1) {
        std::memcpy(mBuffer + mWritePos, source, sizeof(short) * first);
    } else {
        for (long i = 0; i < first; ++i) {
            mBuffer[mWritePos + i] = source[i * stride];
        }
    }
    skipped += excess;
    if (count > tail) {
        if (stride == 1) {
            std::memcpy(mBuffer, source + first, sizeof(short) * (count - first));
        } else {
            const short* rest = source + first * stride;
            for (long i = 0; i < count - first; ++i) {
                mBuffer[i] = rest[i * stride];
            }
        }
    }
    mWritePos = (mWritePos + count) % mSize;
    return skipped;
}

// Base of every microphone (mic/Mic.o). It keeps the recent samples for
// analysis and a continuous stream for readers, and tracks the level with a
// PitchDetector. The vtable is at 0x18E63E0; the object is 16,600 bytes.
class Mic {
public:
    // Recent and continuous ring sizes. Names not in the reference map.
    static constexpr int kRecentBufferSize = 0x2000;
    static constexpr int kContinuousBufferSize = 0x4000;
    static constexpr int kAnalysisSize = 0x2000;

    // Inline; emitted with Mic_FMOD at 0x27B500.
    Mic()
        : mSampleRate(0),
          mOverflowCount(0),
          mVolume(1.0F),
          mMuted(false),
          mInUse(false),
          mDetectorGain(1.0F),
          mDetectorSensitivity(1.0F),
          mAnalysisEnabled(false),
          mPitchDetector(48000),
          mPitch(0.0F),
          mLevel(0.0F),
          mWindowLevel(0.0F),
          mInputPeak(0.0F) {
        mRecentBuffer.Init(kRecentBufferSize);
        mContinuousBuffer.Init(kContinuousBufferSize);
    }
    virtual ~Mic();  // slots 0-1: 0xE1530, 0xE1590
    // Slot 2 at 0xE1A00: 0 while free, 2 once bound to hardware. Name not in
    // the reference map.
    virtual int GetStatus() const;
    virtual int GetType() const;          // slot 3: 0xE1A10
    virtual bool IsRunning() const = 0;   // slot 4
    // Slot 5 at 0xE1A20, empty and not overridden. Inferred from the map's
    // empty Mic::StopPlayback().
    virtual void StopPlayback();
    virtual int GetDroppedSamples();      // slot 6: 0xE1A30
    // Slot 7 at 0xE1A40: run for every mic on the mic reader thread.
    virtual void MicThreadPoll();
    virtual float GetSensitivity() const;  // slot 8: 0xE1A50. Inferred from the map.
    virtual Symbol GetName() const;       // slot 9: 0xE1A60
    virtual void Start() = 0;             // slot 10
    // Slot 11 at 0xE1B00. The map has StartPlayback().
    virtual void StartPlayback(const PlayArgs& args);
    virtual void Stop() = 0;              // slot 12
    // Slots 13-14 push the stored volume and mute to the device. Names not
    // in the reference map.
    virtual void _ApplyVolume() = 0;
    virtual void _ApplyMute() = 0;
    // Slot 15: moves newly recorded samples into the rings. Inferred from
    // the map's Mic_FMOD::_Poll().
    virtual void _Poll() = 0;

    // Restarts with silent rings. At 0xE15F0 and 0xE1650. Names not in the
    // reference map.
    void ResetBuffersAndStart();
    void ResetBuffersAndStartPlayback(const PlayArgs& args);
    // Releases a disconnected device. At 0xE16B0. The map names
    // Mic_FMOD::_HandleMicRemoval().
    void _HandleMicRemoval();
    // Copies the latest samples of the recent ring. At 0xE16C0. Name not in
    // the reference map.
    int GetRecentSamples(short* samples, int count);
    // Consumes up to `count` samples of the continuous ring. At 0xE1740.
    // Name not in the reference map.
    int ReadContinuousSamples(short* samples, int count);
    // Drops the unread continuous samples. At 0xE17F0. Name not in the
    // reference map.
    void SkipContinuousSamples();
    // Samples the last _Poll could not store. At 0xE1800. Name not in the
    // reference map.
    int GetOverflowCount() const;
    // Fetches new samples and, while bound with analysis enabled, analyses
    // them. MicHwManager::Poll runs it for every mic. At 0xE1810.
    void Poll();
    // Runs the pitch detector over the latest samples and smooths the
    // level. At 0xE1850. Name not in the reference map.
    void _AnalyzeRecentSamples();

    // Field names are not in the reference map.
    int mSampleRate;
    RingBuffer mRecentBuffer;
    RingBuffer mContinuousBuffer;
    short mAnalysisSamples[kAnalysisSize];
    int mOverflowCount;
    float mVolume;
    bool mMuted;
    bool mInUse;  // Claimed through MicHwManager::CaptureMic.
    // Inputs and outputs of PitchDetector::AnalyzeBlock; the names follow
    // its parameters and are partly guesses.
    float mDetectorGain;
    float mDetectorSensitivity;
    bool mAnalysisEnabled;
    PitchDetector mPitchDetector;
    float mPitch;
    float mLevel;  // Smoothed detector energy in [0, 1].
    // PitchDetector's window level and input-envelope peak.
    float mWindowLevel;
    float mInputPeak;
};

static_assert(offsetof(Mic, mSampleRate) == 8);
static_assert(offsetof(Mic, mRecentBuffer) == 16);
static_assert(offsetof(Mic, mContinuousBuffer) == 40);
static_assert(offsetof(Mic, mAnalysisSamples) == 64);
static_assert(offsetof(Mic, mOverflowCount) == 16448);
static_assert(offsetof(Mic, mVolume) == 16452);
static_assert(offsetof(Mic, mMuted) == 16456);
static_assert(offsetof(Mic, mInUse) == 16457);
static_assert(offsetof(Mic, mAnalysisEnabled) == 16468);
static_assert(offsetof(Mic, mPitchDetector) == 16472);
static_assert(offsetof(Mic, mLevel) == 16588);
static_assert(sizeof(Mic) == 16600);
