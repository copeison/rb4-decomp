#pragma once

#include <cstddef>

#include "audio/core/output/AudioRenderTarget.h"
#include "utl/threading/Thread.h"

class AudioEmitterCom;
class BinStream;
class TransEntityResource;
class WaveFile;

// Render target that records its mix offline to a 16-bit wave file on a
// worker thread. Its own entity holds an emitter that plays into it. The
// vtable is at 0x19A01A0, the constructor at 0x11289D0 and the destructor at
// 0x1128D30; the object is 480 bytes. Name not in the reference map.
class RecordingAudioRenderTarget : public AudioRenderTarget {
public:
    RecordingAudioRenderTarget(
        Symbol name,
        const char* path,
        int bufferSize,
        int maxSoftwareChannels,
        int sampleRate,
        float gain,
        int speakerConfig);
    ~RecordingAudioRenderTarget() override;  // slots 0-1: 0x1128D30, 0x1128E30

    // Slot 17 at 0x1129360: the target whose name the emitter plays into.
    // Name not in the reference map.
    virtual AudioRenderTarget* GetOutputTarget();
    // Slots 18-19. Names not in the reference map.
    virtual void StartAsyncRecording() = 0;
    virtual void WaitForRecording() = 0;

    // Names below are not in the reference map.
    // Restores the constructor's field defaults. At 0x1128BB0, with no
    // callers in this build.
    void _ResetState();
    // Creates the entity and the emitter that plays into this target. At
    // 0x1128BF0, tail-called by the constructor.
    void _CreateEmitter();
    // True when there is no wave stream or it reports a failure. At
    // 0x1128E50, with no callers in this build.
    bool StreamFailed();
    // Writes the wave header and starts the recording thread. At 0x1128E70,
    // with no callers in this build.
    void StartRecording();
    // Two identical stream checks at 0x1128ED0 and 0x1128F00, with no callers
    // in this build.
    void CheckStream();
    void CheckStreamAgain();
    // Waits for the recording thread, then closes the stream and the wave
    // file. At 0x1128F30.
    void ReleaseRecording();
    // Kills the emitter's generator. At 0x1129140.
    void StopRecording();
    // Runs the recording loop until stopped. At 0x1129160.
    void RecordLoop();

    // Field names are not in the reference map.
    float* mMixBuffer;
    WaveFile* mWaveFile;
    BinStream* mStream;
    int mFramesWritten;
    unsigned int mGeneratorHandle;  // Generator whose pause holds the loop.
    int mNumMixes;
    bool mStopRequested;
    NamedThread mRecordThread;
    bool mFinished;
    float mGain;
    TransEntityResource* mEntityResource;
    AudioEmitterCom* mEmitter;
};

static_assert(offsetof(RecordingAudioRenderTarget, mMixBuffer) == 280);
static_assert(offsetof(RecordingAudioRenderTarget, mFramesWritten) == 304);
static_assert(offsetof(RecordingAudioRenderTarget, mGeneratorHandle) == 308);
static_assert(offsetof(RecordingAudioRenderTarget, mNumMixes) == 312);
static_assert(offsetof(RecordingAudioRenderTarget, mStopRequested) == 316);
static_assert(offsetof(RecordingAudioRenderTarget, mRecordThread) == 320);
static_assert(offsetof(RecordingAudioRenderTarget, mFinished) == 456);
static_assert(offsetof(RecordingAudioRenderTarget, mGain) == 460);
static_assert(offsetof(RecordingAudioRenderTarget, mEntityResource) == 464);
static_assert(offsetof(RecordingAudioRenderTarget, mEmitter) == 472);
static_assert(sizeof(RecordingAudioRenderTarget) == 480);
