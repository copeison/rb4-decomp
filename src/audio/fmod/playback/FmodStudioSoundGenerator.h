#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/fmod/api/fmod_api.h"

// Copies a Studio path into a 256-character buffer, prefixing "event:/" when
// the name has no scheme. The studio manager also accepts "snapshot:/"
// paths; the dialog manager only recognizes "event:". Both inline this into
// their Play. Name not in the reference map.
const char* MakeStudioEventPath(char (&buffer)[256], const char* name, bool allowSnapshot);

// Generator that plays one FMOD Studio event. The vtable is at 0x18F09F0;
// the object is 208 bytes.
class FmodStudioSoundGenerator : public AudioGenerator {
public:
    FmodStudioSoundGenerator();  // 0x26FB70

    void Pause() override;                // slot 0: 0x2701D0
    void Continue() override;             // slot 1: 0x2701F0
    void Stop() override;                 // slot 2: 0x270620
    float GetElapsedMs() override;        // slot 4: 0x270210
    float GetTimelineMs() override;       // slot 5: 0x270260
    float GetLengthMs() const override;   // slot 6: 0x2714C0
    void SeekToMs(float ms) override;     // slot 7: 0x2702B0
    bool SetParameter(Symbol name, float value) override;   // slot 10: 0x2707D0
    bool GetParameter(Symbol name, float& value) override;  // slot 11: 0x270800
    void SetGain(float gain, float fadeSecs, PostFadeOption option) override;  // slot 12: 0x270880
    float GetGain() const override;       // slot 13: 0x2709A0
    void SetMute(bool mute, bool immediate) override;  // slot 14: 0x2709B0
    bool GetMute() const override;        // slot 15: 0x270B10
    ~FmodStudioSoundGenerator() override;  // slots 20-21: 0x2714D0, 0x271540
    bool Poll() override;                 // slot 22: 0x2702C0
    void Release() override;              // slot 23: 0x270750
    void _InitTypeId() override;          // slot 28: 0x2715B0
    void Kill() override;                 // slot 29: 0x2706A0
    AudioGenerator* GetGeneratorOfType(Symbol type) override;  // slot 30: 0x271600
    Symbol GetTypeId() override;          // slot 31: 0x271620

    // Reconstructed from eboot.elf at 0x26FC30. The map has
    // _Setup(char const*, bool); this build takes the request, an optional
    // event callback and the callback's user data.
    bool _Setup(
        const char* path,
        const PlayArgs& args,
        FMOD_STUDIO_EVENT_CALLBACK callback,
        void* userData);
    // Reconstructed from eboot.elf at 0x26FF50.
    static FMOD_RESULT _EventCallback(
        FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
        FMOD_STUDIO_EVENTINSTANCE* event,
        void* parameters);
    // Stops and releases the event at once. At 0x270660. Name not in the
    // reference map.
    void _ReleaseEvent();

    // At 0x19F2D68. Name not in the reference map.
    static Symbol sTypeId;

    // Field names are not in the reference map.
    GainRamp mGain;
    bool mStopAfterGain;
    GainRamp mMuteGain;
    bool mMuted;
    float mLastPollMs;
    FMOD::Studio::EventInstance* mEventInstance;
    float mLengthMs;
    bool mEventDone;  // Set once Studio may release the event.
};

static_assert(offsetof(FmodStudioSoundGenerator, mGain) == 80);
static_assert(offsetof(FmodStudioSoundGenerator, mStopAfterGain) == 128);
static_assert(offsetof(FmodStudioSoundGenerator, mMuteGain) == 136);
static_assert(offsetof(FmodStudioSoundGenerator, mMuted) == 184);
static_assert(offsetof(FmodStudioSoundGenerator, mLastPollMs) == 188);
static_assert(offsetof(FmodStudioSoundGenerator, mEventInstance) == 192);
static_assert(offsetof(FmodStudioSoundGenerator, mLengthMs) == 200);
static_assert(offsetof(FmodStudioSoundGenerator, mEventDone) == 204);
static_assert(sizeof(FmodStudioSoundGenerator) == 208);

// Pool of FmodStudioSoundGenerator voices. The vtable is at 0x18F0B00.
class FmodStudioSoundGeneratorManager : public AudioGeneratorManager {
public:
    AudioGenerator* Play(const PlayArgs& args) override;  // slot 0: 0x270B20
    int GetIndex() override;               // slot 6: 0x270D90
    Symbol GetId() override;               // slot 7: 0x270DA0
    Symbol GetResourceExt() override;      // slot 8: 0x270E40
    AudioGenerator* LockIfOwned(unsigned int handle, int index) override;  // slot 9: 0x270EE0
    void SendStopToAllGenerators() override;  // slot 10: 0x270F50
    void SendKillToAllGenerators() override;  // slot 11: 0x270FC0
    void GetActiveHandles(void* handles) override;  // slot 12: 0x271030
    void _SetManagerIndex(int index) override;      // slot 13: 0x271180
    void _InitGeneratorPool() override;    // slot 14: 0x271190
    bool _DeleteGeneratorPool() override;  // slot 15: 0x271350
    ~FmodStudioSoundGeneratorManager() override;  // slots 16-17: 0x271490, 0x2714A0

    FmodStudioSoundGenerator* mPool;  // Name not in the reference map.
};

static_assert(offsetof(FmodStudioSoundGeneratorManager, mPool) == 64);
