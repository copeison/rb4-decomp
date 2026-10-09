#pragma once

#include <cstddef>

#include "audio/core/output/RecordingAudioRenderTarget.h"
#include "audio/fmod/system/FmodPlatform.h"
#include "utl/threading/Thread.h"

// Recording target that renders an FMOD system through HMX.BufferedOutput.
// The vtable is at 0x18F0D40. Its members end at 1448 bytes; the embedded
// std::function aligns the object to 16. No caller constructs it in this
// build. Name not in the reference map.
class FmodRecordingAudioRenderTarget : public RecordingAudioRenderTarget {
public:
    // Reconstructed from eboot.elf at 0x275E20.
    FmodRecordingAudioRenderTarget(
        Symbol name,
        const char* path,
        int bufferLength,
        int maxChannels,
        int sampleRate,
        float gain,
        int speakerConfig);
    ~FmodRecordingAudioRenderTarget() override;  // slots 0-1: 0x275FB0, 0x276010

    int SuspendMixer() override;    // slot 2: 0x276130
    int ResumeMixer() override;     // slot 3: 0x276140
    // Slot 5 at 0x276150.
    void InitVoicePool(
        int hardVoiceLimit, int softVoiceLimit, int unknownCount, bool unknownFlag) override;
    void ConfigureVoicePool(int softVoiceLimit, int hardVoiceLimit) override;  // slot 6: 0x276160
    FusionVoicePool* GetVoicePool() override;  // slot 7: 0x276170
    bool TryBeginMix() override;    // slot 8: 0x276180
    bool EndMix() override;         // slot 9: 0x276190
    void Lock() override;           // slot 10: 0x2761A0
    void Unlock() override;         // slot 11: 0x2761D0
    int Update() override;          // slot 12: 0x276200
    void AddPremixCallback(FmodPremixCallback* callback) override;     // slot 13: 0x276210
    void RemovePremixCallback(FmodPremixCallback* callback) override;  // slot 14: 0x276230
    void ExecutePremixCallbacks(unsigned long mixCount) override;      // slot 15: 0x276250
    AudioMixer* GetMixer() override;       // slot 16: 0x276290
    FModSystem* GetOutputTarget() override;  // slot 17: 0x2762B0
    void StartAsyncRecording() override;   // slot 18: 0x276080
    void WaitForRecording() override;      // slot 19: 0x276120

    // Field names are not in the reference map.
    FModSystem mFModSystem;
    NamedThread mRecordThread;
};

static_assert(offsetof(FmodRecordingAudioRenderTarget, mFModSystem) == 480);
static_assert(offsetof(FmodRecordingAudioRenderTarget, mRecordThread) == 1312);
static_assert(sizeof(FmodRecordingAudioRenderTarget) == 1456);

// Resolves the FMOD system behind a render target, as every FMOD generator
// inlines it. Name not in the reference map.
inline FModSystem* FModSystemForTarget(AudioRenderTarget* target) {
    if (target->mType == AudioRenderTarget::kTypeFmodSystem) {
        return static_cast<FModSystem*>(target);
    }
    if (target->mType == AudioRenderTarget::kTypeRecording) {
        return &static_cast<FmodRecordingAudioRenderTarget*>(target)->mFModSystem;
    }
    return nullptr;
}
