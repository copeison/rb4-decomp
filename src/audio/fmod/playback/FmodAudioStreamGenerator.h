#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/resources/Resource.h"
#include "audio/fmod/api/fmod_api.h"

class AudioBusCallable;
class FmodAudioStreamResource;

// Generator that streams an FmodAudioStreamResource file on a low-level FMOD
// channel. The vtable is at 0x18F0418; the object is 288 bytes.
class FmodAudioStreamGenerator : public AudioGenerator {
public:
    // Inlined into _InitGeneratorPool at 0x26A690.
    FmodAudioStreamGenerator();

    void Pause() override;                // slot 0: 0x269BF0
    void Continue() override;             // slot 1: 0x269C00
    void Stop() override;                 // slot 2: 0x269CF0
    float GetElapsedMs() override;        // slot 4: 0x269C10
    float GetTimelineMs() override;       // slot 5: 0x269C50
    float GetLengthMs() const override;   // slot 6: 0x26AA10
    void SeekToMs(float ms) override;     // slot 7: 0x269C60
    void SetSpeed(float speed, bool immediate) override;  // slot 8: 0x269C70
    float GetSpeed(bool* changing) override;  // slot 9: 0x269C90
    void SetGain(float gain, float fadeSecs, PostFadeOption option) override;  // slot 12: 0x269D40
    float GetGain() const override;       // slot 13: 0x269E70
    void SetMute(bool mute, bool immediate) override;  // slot 14: 0x269E80
    bool GetMute() const override;        // slot 15: 0x269FA0
    void Init(AudioGeneratorManager* manager, int index) override;  // slot 19: 0x2692B0
    ~FmodAudioStreamGenerator() override;  // slots 20-21: 0x2693B0, 0x269440
    bool Poll() override;                 // slot 22: 0x2694D0
    void Release() override;              // slot 23: 0x269FB0
    void _InitTypeId() override;          // slot 28: 0x26AA20
    void Kill() override;                 // slot 29: 0x269D00
    AudioGenerator* GetGeneratorOfType(Symbol type) override;  // slot 30: 0x26AA70
    Symbol GetTypeId() override;          // slot 31: 0x26AA90

    // Reconstructed from eboot.elf at 0x268EF0. The map has
    // Setup(char const*, PlayArgs const&, AudioBusCallable*); this build
    // passes the resource.
    bool Setup(ResourcePtr<FmodAudioStreamResource> resource, const PlayArgs& args);
    // Reconstructed from eboot.elf at 0x269CD0. The map has SetLoop(float,
    // float); the first argument is the loop end.
    void SetLoop(float endMs, float startMs);
    void ClearLoop();                     // 0x269CB0
    void _UpdateWorldXfm();               // 0x269B70
    // Starts the channel once the sound and its bus are ready. At 0x269980.
    // Name not in the reference map.
    void _TryStartChannel();
    // Applies gain and mute to the channel. At 0x269B40. Name not in the
    // reference map.
    void _UpdateVolume();
    // Validate a bus path with the engine. At 0x2692E0, 0x26A0F0 and
    // 0x26A1C0. Names not in the reference map.
    static void _CheckBusLoaded(FMOD::Studio::Bus* bus);
    static void _CheckBusKnown(FMOD::Studio::Bus* bus);
    static Symbol _GetBusPath(FMOD::Studio::Bus* bus);

    // At 0x19F2CD0. Name not in the reference map.
    static Symbol sTypeId;

    // Field names are not in the reference map.
    FMOD::Sound* mSound;
    FMOD::Channel* mChannel;
    FMOD::Studio::Bus* mStudioBus;
    int mBusRetries;
    GeneratorTimer mPlayTimer;
    unsigned int mLengthMs;
    unsigned int mPositionMs;
    unsigned int mPendingSeekMs;
    GainRamp mGain;
    GainRamp mMuteGain;
    bool mMuted;
    bool mStopAfterGain;
    float mLastPollMs;
    bool mPauseRequested;
    float mLoopEndMs;
    float mLoopStartMs;
    bool mLoopDirty;
    unsigned int mLengthPcm;
    float mFrequency;
    float mSpeed;
    bool mSpeedDirty;
};

static_assert(offsetof(FmodAudioStreamGenerator, mSound) == 80);
static_assert(offsetof(FmodAudioStreamGenerator, mBusRetries) == 104);
static_assert(offsetof(FmodAudioStreamGenerator, mPlayTimer) == 112);
static_assert(offsetof(FmodAudioStreamGenerator, mLengthMs) == 136);
static_assert(offsetof(FmodAudioStreamGenerator, mPendingSeekMs) == 144);
static_assert(offsetof(FmodAudioStreamGenerator, mGain) == 152);
static_assert(offsetof(FmodAudioStreamGenerator, mMuteGain) == 200);
static_assert(offsetof(FmodAudioStreamGenerator, mMuted) == 248);
static_assert(offsetof(FmodAudioStreamGenerator, mLastPollMs) == 252);
static_assert(offsetof(FmodAudioStreamGenerator, mPauseRequested) == 256);
static_assert(offsetof(FmodAudioStreamGenerator, mLoopEndMs) == 260);
static_assert(offsetof(FmodAudioStreamGenerator, mLoopDirty) == 268);
static_assert(offsetof(FmodAudioStreamGenerator, mLengthPcm) == 272);
static_assert(offsetof(FmodAudioStreamGenerator, mSpeed) == 280);
static_assert(offsetof(FmodAudioStreamGenerator, mSpeedDirty) == 284);
static_assert(sizeof(FmodAudioStreamGenerator) == 288);

// Pool of FmodAudioStreamGenerator voices. The vtable is at 0x18F0528.
class FmodAudioStreamGeneratorManager : public AudioGeneratorManager {
public:
    AudioGenerator* Play(const PlayArgs& args) override;  // slot 0: 0x26A270
    void Init() override;                  // slot 3: 0x268C90
    int GetIndex() override;               // slot 6: 0x26A280
    Symbol GetId() override;               // slot 7: 0x26A290
    Symbol GetResourceExt() override;      // slot 8: 0x26A330
    AudioGenerator* LockIfOwned(unsigned int handle, int index) override;  // slot 9: 0x26A3D0
    void SendStopToAllGenerators() override;  // slot 10: 0x26A450
    void SendKillToAllGenerators() override;  // slot 11: 0x26A4C0
    void GetActiveHandles(void* handles) override;  // slot 12: 0x26A530
    void _SetManagerIndex(int index) override;      // slot 13: 0x26A680
    void _InitGeneratorPool() override;    // slot 14: 0x26A690
    bool _DeleteGeneratorPool() override;  // slot 15: 0x26A850
    ~FmodAudioStreamGeneratorManager() override;  // slots 16-17: 0x26A9E0, 0x26A9F0

    // Reconstructed from eboot.elf at 0x268CA0. Streams the format-3
    // streaming requests leave to FmodBufferedStreamGeneratorManager.
    FmodAudioStreamGenerator* PlayWithCallback(const PlayArgs& args, AudioBusCallable* callback);
    // Reconstructed from eboot.elf at 0x268D80. The map has
    // _AllocateAndSetUpGenerator(char const*, PlayArgs const&,
    // AudioBusCallable*); this build passes the resource.
    FmodAudioStreamGenerator* _AllocateAndSetUpGenerator(
        ResourcePtr<FmodAudioStreamResource> resource, const PlayArgs& args);

    FmodAudioStreamGenerator* mPool;  // Name not in the reference map.
};

static_assert(offsetof(FmodAudioStreamGeneratorManager, mPool) == 64);
