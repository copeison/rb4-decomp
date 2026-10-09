#pragma once

#include <cstddef>

#include "audio/core/output/AudioRenderTarget.h"

class FModSystem;

// Render target that records its mix offline on a worker thread. The vtable
// is at 0x19A01A0, the constructor at 0x11289D0 and the destructor at
// 0x1128D30; the object is 480 bytes. Its own methods have not been
// reconstructed. Name not in the reference map.
class RecordingAudioRenderTarget : public AudioRenderTarget {
public:
    RecordingAudioRenderTarget(
        Symbol name,
        void* unknown,
        int bufferLength,
        int maxChannels,
        int sampleRate,
        float unknownRate,
        int speakerConfig);
    ~RecordingAudioRenderTarget() override;

    // Slot 17. Name not in the reference map.
    virtual FModSystem* GetFModSystem() = 0;
    // Slots 18-19. Names not in the reference map.
    virtual void StartAsyncRecording() = 0;
    virtual void WaitForRecording() = 0;

    // Runs the recording loop until stopped. At 0x1129160. Name not in the
    // reference map.
    void RecordLoop();
    // Teardown steps at 0x1129140 and 0x1128F30. Names not in the reference
    // map.
    void StopRecording();
    void ReleaseRecording();

    // Field names are not in the reference map.
    float* mMixBuffer;
    unsigned char mUnknown288[192];
};

static_assert(offsetof(RecordingAudioRenderTarget, mMixBuffer) == 280);
static_assert(sizeof(RecordingAudioRenderTarget) == 480);
