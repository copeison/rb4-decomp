#pragma once

#include <atomic>
#include <cstddef>

#include "audio/core/generators/AudioBusGenerator.h"
#include "audio/fmod/api/fmod_api.h"

// Bus generator that plays its source through a custom FMOD DSP,
// HMXRawAudioBus. The DSP runs on a low-level channel, optionally routed to a
// Studio bus, or inside a non-oneshot Studio event. The primary vtable is at
// 0x18F0208 and the AudioBusCallable vtable at 0x18F0338; the object is 448
// bytes.
class FmodAudioBusGenerator : public AudioBusGenerator {
public:
    // Reconstructed from eboot.elf at 0x266F10.
    explicit FmodAudioBusGenerator(AudioBus* source = nullptr);

    void Pause() override;                // slot 0: 0x267BB0
    void Continue() override;             // slot 1: 0x267C00
    void Stop() override;                 // slot 2: 0x267F10
    float GetElapsedMs() override;        // slot 4: 0x267C60
    float GetTimelineMs() override;       // slot 5: 0x267CC0
    void SeekToMs(float ms) override;     // slot 7: 0x267D30
    bool SetParameter(Symbol name, float value) override;   // slot 10: 0x2683F0
    bool GetParameter(Symbol name, float& value) override;  // slot 11: 0x268410
    ~FmodAudioBusGenerator() override;    // slots 20-21: 0x266F60, 0x266F80
    bool Poll() override;                 // slot 22: 0x267D70
    void Release() override;              // slot 23: 0x268380
    void* GetPluginData(const char* name) override;  // slot 24: 0x268490
    void _InitTypeId() override;          // slot 28: 0x268BF0
    void Kill() override;                 // slot 29: 0x2682A0
    AudioGenerator* GetGeneratorOfType(Symbol type) override;  // slot 30: 0x268C40
    Symbol GetTypeId() override;          // slot 31: 0x268C60
    // Slot 32 at 0x266FD0.
    bool Setup(AudioBus* source, const PlayArgs& args, AudioBusCallable* callback) override;
    bool _PrepareToMakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;  // slot 33: 0x2675A0
    bool _MakeSamples(
        int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) override;  // slot 34: 0x267790

    // Reconstructed from eboot.elf at 0x266CB0.
    static FMOD_RESULT _DspCreate(FMOD_DSP_STATE* state);
    // Reconstructed from eboot.elf at 0x266CD0.
    static FMOD_RESULT _DspProcess(
        FMOD_DSP_STATE* state,
        unsigned int length,
        const FMOD_DSP_BUFFER_ARRAY* inputs,
        FMOD_DSP_BUFFER_ARRAY* outputs,
        bool inputsIdle,
        FMOD_DSP_PROCESS_OPERATION operation);
    // Reconstructed from eboot.elf at 0x267310. It also binds the DSP into
    // the event's channel group when the programmer sound is created.
    static FMOD_RESULT _EventProgrammerCallback(
        FMOD_STUDIO_EVENT_CALLBACK_TYPE type,
        FMOD_STUDIO_EVENTINSTANCE* event,
        void* parameters);

    void _DeativateBusAndClearDSPUserData();  // 0x2681E0
    void _StopChannel();                      // 0x267FB0
    void _StopEventInstance();                // 0x268080
    void _ReleaseDSP();                       // 0x268260
    void _UpdateWorldXfm();                   // 0x267E70
    // Runs both stop paths. At 0x267E50. Name not in the reference map.
    void _StopChannelAndEventInstance();

    // The custom DSP at 0x19B4840. Name not in the reference map.
    static FMOD_DSP_DESCRIPTION sDspDescription;
    // At 0x19F2CA0. Name not in the reference map.
    static Symbol sTypeId;

    // Field names are not in the reference map.
    FMOD::DSP* mDSP;
    FMOD::Channel* mChannel;
    FMOD::ChannelGroup* mChannelGroup;
    FMOD::Studio::Bus* mStudioBus;
    FMOD::Studio::EventInstance* mEventInstance;
    bool mEventCallbackDone;
    float mFrequency;
    std::atomic<int> mResetWord;  // Zeroed by the constructor and pool setup; never read.
    bool mKilling;
};

static_assert(offsetof(FmodAudioBusGenerator, mDSP) == 392);
static_assert(offsetof(FmodAudioBusGenerator, mChannel) == 400);
static_assert(offsetof(FmodAudioBusGenerator, mChannelGroup) == 408);
static_assert(offsetof(FmodAudioBusGenerator, mStudioBus) == 416);
static_assert(offsetof(FmodAudioBusGenerator, mEventInstance) == 424);
static_assert(offsetof(FmodAudioBusGenerator, mEventCallbackDone) == 432);
static_assert(offsetof(FmodAudioBusGenerator, mFrequency) == 436);
static_assert(offsetof(FmodAudioBusGenerator, mResetWord) == 440);
static_assert(offsetof(FmodAudioBusGenerator, mKilling) == 444);
static_assert(sizeof(FmodAudioBusGenerator) == 448);

// Pool of FmodAudioBusGenerator voices. The vtable is at 0x18F0370.
class FmodAudioBusGeneratorManager : public AudioGeneratorManager {
public:
    // The manager's id, interned on first use. Inlined into GetId at
    // 0x268520 and FmodBufferedStreamGenerator::_AcquireBusGenerator at
    // 0x26B490, which share its static. Name not in the reference map.
    static Symbol Id() {
        static Symbol sId("");
        if (sId == Symbol("")) {
            sId = Symbol("FmodAudioBusGeneratorManager");
        }
        return sId;
    }

    AudioGenerator* Play(const PlayArgs& args) override;  // slot 0: 0x268500
    void Init() override;                  // slot 3: 0x268480
    int GetIndex() override;               // slot 6: 0x268510
    Symbol GetId() override;               // slot 7: 0x268520
    Symbol GetResourceExt() override;      // slot 8: 0x2685C0
    AudioGenerator* LockIfOwned(unsigned int handle, int index) override;  // slot 9: 0x268660
    void SendStopToAllGenerators() override;  // slot 10: 0x2686D0
    void SendKillToAllGenerators() override;  // slot 11: 0x268740
    void GetActiveHandles(void* handles) override;  // slot 12: 0x2687B0
    void _SetManagerIndex(int index) override;      // slot 13: 0x268900
    void _InitGeneratorPool() override;    // slot 14: 0x268910
    bool _DeleteGeneratorPool() override;  // slot 15: 0x268A60
    ~FmodAudioBusGeneratorManager() override;  // slots 16-17: 0x268B10, 0x268B20
    // Slot 18 at 0x268B40. The map has _GetGenerator(Symbol,
    // AudioEmitterCom*, bool).
    virtual FmodAudioBusGenerator* _GetGenerator(
        AudioRenderTarget* target, AudioEmitterCom* emitter);

    FmodAudioBusGenerator* mPool;  // Name not in the reference map.
};

static_assert(offsetof(FmodAudioBusGeneratorManager, mPool) == 64);

// The bus pool the FMOD platform creates (slot 20 at 0x261210 stores it).
// The Fusion, Mogg, MultiFusion and SynthRack generators take their bus
// voices from it through _GetGenerator; without it they cannot play. Name
// not in the reference map.
extern FmodAudioBusGeneratorManager* gAudioBusGeneratorManager;  // 0x19E26A8
