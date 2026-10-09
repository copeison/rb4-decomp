#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/output/AudioBus.h"
#include "entity/resources/Resource.h"
#include "audio/core/streams/StreamReaderThread.h"
#include "audio/fmod/api/fmod_api.h"
#include "math/smoothing/DoubleExponentialSmoother.h"
#include "utl/containers/Vector.h"

class FmodAudioBusGenerator;
class FmodAudioStreamResource;

// Request for a buffered stream: format 3 with streaming set. The stream is
// decoded into a ring of buffers of mFramesPerBuffer frames, of which
// mBuffersBehind stay behind the play position. Name not in the reference
// map.
struct BufferedStreamPlayArgs : public PlayArgs {
    int mFramesPerBuffer;
    int mNumBuffers;
    int mBuffersBehind;
};

static_assert(offsetof(BufferedStreamPlayArgs, mFramesPerBuffer) == 104);
static_assert(offsetof(BufferedStreamPlayArgs, mBuffersBehind) == 112);

// Generator that decodes a stream into its own ring of 16-bit buffers through
// the stream reader thread and renders it as an AudioBus through a pooled
// FmodAudioBusGenerator. Playback can follow a moving sync target, which
// keeps the stream sample-accurate with the song clock. The primary vtable is
// at 0x18F0668 and the AudioBus vtable at 0x18F0780; the object is 568 bytes.
// The map has no object for it; the name comes from _InitTypeId at 0x26E3A0.
class FmodBufferedStreamGenerator : public AudioGenerator, public AudioBus {
public:
    FmodBufferedStreamGenerator();  // Inlined into _InitGeneratorPool at 0x26E000.

    void Pause() override;                // slot 0: 0x26BD40
    void Continue() override;             // slot 1: 0x26BD70
    void Stop() override;                 // slot 2: 0x26C150
    float GetElapsedMs() override;        // slot 4: 0x26BDA0
    float GetTimelineMs() override;       // slot 5: 0x26BDB0
    void SeekToMs(float ms) override;     // slot 7: 0x26BDC0
    void SetSpeed(float speed, bool immediate) override;  // slot 8: 0x26C040
    float GetSpeed(bool* changing) override;  // slot 9: 0x26C080
    void SetGain(float gain, float fadeSecs, PostFadeOption option) override;  // slot 12: 0x26E320
    float GetGain() const override;       // slot 13: 0x26E340
    void SetMute(bool mute, bool immediate) override;  // slot 14: 0x26E360
    bool GetMute() const override;        // slot 15: 0x26E380
    void Init(AudioGeneratorManager* manager, int index) override;  // slot 19: 0x26AFF0
    // Slots 20-21 at 0x26B6A0 and 0x26B930; the AudioBus side's thunks are
    // at 0x26B920 and 0x26B950.
    ~FmodBufferedStreamGenerator() override;
    bool Poll() override;                 // slot 22: 0x26B9A0
    void Release() override;              // slot 23: 0x26C1B0
    void _InitTypeId() override;          // slot 28: 0x26E3A0
    void Kill() override;                 // slot 29: 0x26C180
    AudioGenerator* GetGeneratorOfType(Symbol type) override;  // slot 30: 0x26E3F0
    Symbol GetTypeId() override;          // slot 31: 0x26E410
    // Slot 32 at 0x26C720 (0x26DAA0 adjusts from the AudioBus base): renders
    // the decoded buffers into the bus generator's block.
    bool Process(AudioBuffer<float>& buffer) override;

    // Opens the stream, sets up the reader ring and takes a bus generator.
    // At 0x26ADA0. Name not in the reference map.
    bool Setup(ResourcePtr<FmodAudioStreamResource> resource, const PlayArgs& args);
    // Opens the stream without blocking. At 0x26B060. Name not in the
    // reference map.
    bool _OpenSound(const ResourcePtr<FmodAudioStreamResource>& resource);
    // Clears the sync flag without the bus lock. At 0x26B140. Name not in
    // the reference map.
    void _DisableSyncUnlocked();
    // Sizes the decode buffer and the readers. At 0x26B150. Name not in the
    // reference map.
    void _InitBuffers(const BufferedStreamPlayArgs& args);
    // Takes a voice from the bus generator pool and sets it up on this bus.
    // At 0x26B490. Name not in the reference map.
    bool _AcquireBusGenerator(const PlayArgs& args);
    // Points a reader at a block starting at the frame; a looping block
    // wraps from the loop end to the loop start. At 0x26B650 and inlined at
    // every refill. Name not in the reference map.
    void _SetReaderPosition(StreamReader& reader, int startFrame, bool loop);
    // Takes the stream format once FMOD has opened it and queues the
    // readers. At 0x26BAD0. Name not in the reference map.
    void _FinishOpen();
    // Measures the sync target's rate and smooths it. At 0x26BC70; Poll
    // inlines it. Name not in the reference map.
    void _UpdateSync();
    // Loop points in milliseconds; -1 for both disables the loop. Applied at
    // the next Process. At 0x26C0A0 and 0x26C0F0; the setter takes the end
    // first. Names not in the reference map.
    void ClearLoopPoints();
    void SetLoopPoints(float endMs, float startMs);
    // Requeues the readers ahead of and behind the current one that no
    // longer hold the blocks around the play position. At 0x26C480. Name not
    // in the reference map.
    void _RefillBuffers();
    // Whether the current reader's last read failed. At 0x26D880. Name not
    // in the reference map.
    bool _CurrentReaderFailed() const;
    // Playback advance per output frame while synced: the smoothed target
    // rate over the output rate, or zero once the target is reached. At
    // 0x26D8D0. Name not in the reference map.
    float _GetSyncSpeed() const;
    // The advance per output frame: the sync speed while synced, otherwise
    // the speed. Inlined into Process. Name not in the reference map.
    float _GetPlaybackSpeed() const {
        return mSyncEnabled ? _GetSyncSpeed() : mSpeed;
    }
    // Reconstructed from eboot.elf at 0x26D940. Olli Niemitalo's optimal 32x,
    // six-point, fifth-order z-form interpolator over y[-2]..y[3]. Name not in
    // the reference map.
    float _InterpolateOptimal(const float* samples, float fraction) const;
    // Starts following a sync target placed sSyncLeadMs ahead of the
    // timeline and returns that target in milliseconds. At 0x26DAB0. Names
    // not in the reference map.
    float EnableSync(float levelRate, float timeScale);
    void SetSyncTargetMs(float ms);  // 0x26DBA0
    void DisableSync();              // 0x26DBD0

    // At 0x19F2D00. Name not in the reference map.
    static Symbol sTypeId;
    // Lead of a new sync target, 30 ms, at 0x19B4930. Name not in the
    // reference map.
    static float sSyncLeadMs;

    // Field names are not in the reference map. Frames are stream frames.
    ResourcePtr<FmodAudioStreamResource> mResource;
    eastl::vector<StreamReader> mReaders;  // The ring of decode blocks.
    FmodAudioBusGenerator* mBusGenerator;
    AudioBuffer<short> mDecodeBuffer;      // Interleaved stereo storage of every block.
    int mFramesPerBuffer;
    int mBuffersBehind;
    FMOD::Sound* mSound;
    unsigned int mLengthFrames;
    float mSoundFrequency;
    float mSpeed;
    float mFrequencyRatio;  // Stream frames per output frame at unit speed.
    bool mOpening;
    float mTimelineMs;
    int mCurrentReader;
    bool mWaitingForStart;  // Silent until mStartReader has read.
    int mStartReader;
    float mReadPos;  // Fractional frame within the current block.
    int mClearedOnSetup;  // Zeroed by _InitBuffers and never read.
    int mLoopStartFrame;
    int mLoopEndFrame;
    float mLoopStartMs;
    float mLoopEndMs;
    bool mLoopDirty;
    bool mSyncEnabled;
    int mSyncTargetFrame;
    int mSyncPrevTargetFrame;
    float mSyncRate;  // Target frames per second, clamped to 20 times the stream rate.
    DoubleExponentialSmoother mSyncSmoother;
    float mSyncLastTime;
    float mSyncTimeScale;
};

static_assert(offsetof(FmodBufferedStreamGenerator, mResource) == 272);
static_assert(offsetof(FmodBufferedStreamGenerator, mReaders) == 280);
static_assert(offsetof(FmodBufferedStreamGenerator, mBusGenerator) == 312);
static_assert(offsetof(FmodBufferedStreamGenerator, mDecodeBuffer) == 320);
static_assert(offsetof(FmodBufferedStreamGenerator, mFramesPerBuffer) == 448);
static_assert(offsetof(FmodBufferedStreamGenerator, mSound) == 456);
static_assert(offsetof(FmodBufferedStreamGenerator, mSpeed) == 472);
static_assert(offsetof(FmodBufferedStreamGenerator, mOpening) == 480);
static_assert(offsetof(FmodBufferedStreamGenerator, mTimelineMs) == 484);
static_assert(offsetof(FmodBufferedStreamGenerator, mReadPos) == 500);
static_assert(offsetof(FmodBufferedStreamGenerator, mLoopStartFrame) == 508);
static_assert(offsetof(FmodBufferedStreamGenerator, mLoopDirty) == 524);
static_assert(offsetof(FmodBufferedStreamGenerator, mSyncTargetFrame) == 528);
static_assert(offsetof(FmodBufferedStreamGenerator, mSyncSmoother) == 540);
static_assert(offsetof(FmodBufferedStreamGenerator, mSyncTimeScale) == 564);
static_assert(sizeof(FmodBufferedStreamGenerator) == 568);

// Pool of FmodBufferedStreamGenerator voices with their stream reader thread.
// The vtable is at 0x18F05C8.
class FmodBufferedStreamGeneratorManager : public AudioGeneratorManager {
public:
    AudioGenerator* Play(const PlayArgs& args) override;  // slot 0: 0x26AB60
    void Init() override;                  // slot 3: 0x26AAC0
    int GetIndex() override;               // slot 6: 0x26DC00
    Symbol GetId() override;               // slot 7: 0x26DC10
    Symbol GetResourceExt() override;      // slot 8: 0x26DCB0
    AudioGenerator* LockIfOwned(unsigned int handle, int index) override;  // slot 9: 0x26DD50
    void SendStopToAllGenerators() override;  // slot 10: 0x26DDC0
    void SendKillToAllGenerators() override;  // slot 11: 0x26DE30
    void GetActiveHandles(void* handles) override;  // slot 12: 0x26DEA0
    void _SetManagerIndex(int index) override;      // slot 13: 0x26DFF0
    void _InitGeneratorPool() override;    // slot 14: 0x26E000
    bool _DeleteGeneratorPool() override;  // slot 15: 0x26E270
    ~FmodBufferedStreamGeneratorManager() override;  // slots 16-17: 0x26AAE0, 0x26AB20

    // Reconstructed from eboot.elf at 0x26AC30. Name not in the reference
    // map.
    FmodBufferedStreamGenerator* _AllocateAndSetUpGenerator(
        ResourcePtr<FmodAudioStreamResource> resource, const PlayArgs& args);

    // Field names are not in the reference map.
    FmodBufferedStreamGenerator* mPool;
    StreamReaderThread mReaderThread;
};

static_assert(offsetof(FmodBufferedStreamGeneratorManager, mPool) == 64);
static_assert(offsetof(FmodBufferedStreamGeneratorManager, mReaderThread) == 72);
