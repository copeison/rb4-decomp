#pragma once

#include <cstddef>
#include <functional>

#include "audio/core/generators/DialogGenerator.h"
#include "audio/fmod/playback/FmodStudioSoundGenerator.h"

// Dialog generator that plays a Studio event whose programmer sounds come
// from the localized dialog banks. The work is delegated to an embedded
// FmodStudioSoundGenerator. The vtable is at 0x18F08D8; the object is 416
// bytes. The map has no object for it; the name comes from _InitTypeId at
// 0x26FAC0.
class FmodDialogGenerator : public DialogGenerator {
public:
    void Pause() override;                // slot 0: 0x26F6E0
    void Continue() override;             // slot 1: 0x26F6F0
    void Stop() override;                 // slot 2: 0x26F700
    State GetState() override;            // slot 3: 0x26F710
    float GetElapsedMs() override;        // slot 4: 0x26F720
    float GetTimelineMs() override;       // slot 5: 0x26F730
    float GetLengthMs() const override;   // slot 6: 0x26F740
    void SeekToMs(float ms) override;     // slot 7: 0x26F750
    bool SetParameter(Symbol name, float value) override;   // slot 10: 0x26F760
    bool GetParameter(Symbol name, float& value) override;  // slot 11: 0x26F770
    void SetGain(float gain, float fadeSecs, PostFadeOption option) override;  // slot 12: 0x26F780
    float GetGain() const override;       // slot 13: 0x26F790
    void SetMute(bool mute, bool immediate) override;  // slot 14: 0x26F7A0
    bool GetMute() const override;        // slot 15: 0x26F7C0
    bool IsDialog() override;             // slot 18: 0x26F7D0
    ~FmodDialogGenerator() override;      // slots 20-21: 0x26F7E0, 0x26F940
    bool Poll() override;                 // slot 22: 0x26FAB0
    void Release() override;              // slot 23: 0x26EBE0
    void _InitTypeId() override;          // slot 28: 0x26FAC0
    void Kill() override;                 // slot 29: 0x26FB10
    AudioGenerator* GetGeneratorOfType(Symbol type) override;  // slot 30: 0x26FB20
    Symbol GetTypeId() override;          // slot 31: 0x26FB40
    bool Setup(const char* path, const PlayArgs& args) override;  // slot 32: 0x26E990

    // Creates and releases the dialog's programmer sounds. Reconstructed from
    // eboot.elf at 0x26EA10. Name not in the reference map.
    static FMOD_RESULT _ProgrammerSoundCallback(
        FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
        FMOD_STUDIO_EVENTINSTANCE* event,
        void* parameters);

    // At 0x19F2D38. Name not in the reference map.
    static Symbol sTypeId;

    // Field names are not in the reference map.
    FmodStudioSoundGenerator mStudio;
    // A callback and its context, like DialogGenerator's sink. Only
    // constructed and destroyed in this build (the pool at 0x26F37F and the
    // destructors); the names are weakly supported.
    std::function<void()> mCompletionCallback;
    void* mCompletionContext;
    float mLengthMs;
};

static_assert(offsetof(FmodDialogGenerator, mStudio) == 136);
static_assert(offsetof(FmodDialogGenerator, mCompletionCallback) == 352);
static_assert(offsetof(FmodDialogGenerator, mCompletionContext) == 400);
static_assert(offsetof(FmodDialogGenerator, mLengthMs) == 408);
static_assert(sizeof(FmodDialogGenerator) == 416);

// Pool of FmodDialogGenerator voices. The vtable is at 0x18F0838.
class FmodDialogGeneratorManager : public AudioGeneratorManager {
public:
    AudioGenerator* Play(const PlayArgs& args) override;  // slot 0: 0x26EC60
    int GetIndex() override;               // slot 6: 0x26EE80
    Symbol GetId() override;               // slot 7: 0x26EE90
    Symbol GetResourceExt() override;      // slot 8: 0x26EF30
    AudioGenerator* LockIfOwned(unsigned int handle, int index) override;  // slot 9: 0x26EFD0
    void SendStopToAllGenerators() override;  // slot 10: 0x26F040
    void SendKillToAllGenerators() override;  // slot 11: 0x26F0B0
    void GetActiveHandles(void* handles) override;  // slot 12: 0x26F120
    void _SetManagerIndex(int index) override;      // slot 13: 0x26F270
    void _InitGeneratorPool() override;    // slot 14: 0x26F280
    bool _DeleteGeneratorPool() override;  // slot 15: 0x26F420
    ~FmodDialogGeneratorManager() override;  // slots 16-17: 0x26F6B0, 0x26F6C0

    FmodDialogGenerator* mPool;  // Name not in the reference map.
};

static_assert(offsetof(FmodDialogGeneratorManager, mPool) == 64);
