#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/output/AudioBus.h"
#include "audio/core/resources/Resource.h"
#include "audio/core/streams/StreamReaderThread.h"

class FmodAudioBusGenerator;
class FmodAudioStreamResource;

// Generator that decodes a stream into its own buffers and renders them as an
// AudioBus through a pooled FmodAudioBusGenerator, which keeps the stream
// sample-accurate with the song clock. The primary vtable is at 0x18F0668
// and the AudioBus vtable at 0x18F0780; the object is 568 bytes. The map has
// no object for it; the name comes from _InitTypeId at 0x26E3A0. Only the
// voice controls are reconstructed.
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
    ~FmodBufferedStreamGenerator() override;  // slots 20-21: 0x26B6A0, 0x26B930
    bool Poll() override;                 // slot 22: 0x26B9A0
    void Release() override;              // slot 23: 0x26C1B0
    void _InitTypeId() override;          // slot 28: 0x26E3A0
    void Kill() override;                 // slot 29: 0x26C180
    AudioGenerator* GetGeneratorOfType(Symbol type) override;  // slot 30: 0x26E3F0
    Symbol GetTypeId() override;          // slot 31: 0x26E410
    // Slot 32 at 0x26C720: renders the decoded buffers into the bus
    // generator's block, resampling to the song clock.
    bool Process(AudioBuffer<float>& buffer) override;

    // Reconstructed from eboot.elf at 0x26D940. Olli Niemitalo's optimal 32x,
    // six-point, fifth-order z-form interpolator over y[-2]..y[3]. Name not in
    // the reference map.
    float _InterpolateOptimal(const float* samples, float fraction) const;
    // Opens the stream and its bus generator. At 0x26ADA0. Name not in the
    // reference map.
    bool Setup(ResourcePtr<FmodAudioStreamResource> resource, const PlayArgs& args);

    // At 0x19F2D00. Name not in the reference map.
    static Symbol sTypeId;

    // Field names are not in the reference map.
    ResourcePtr<FmodAudioStreamResource> mResource;
    unsigned char mUnknown280[32];  // EASTL vector of 192-byte channel buffers.
    FmodAudioBusGenerator* mBusGenerator;
    unsigned char mUnknown320[152];
    float mSpeed;
    unsigned char mUnknown476[8];
    float mTimelineMs;
    unsigned char mUnknown488[80];
};

static_assert(offsetof(FmodBufferedStreamGenerator, mResource) == 272);
static_assert(offsetof(FmodBufferedStreamGenerator, mBusGenerator) == 312);
static_assert(offsetof(FmodBufferedStreamGenerator, mSpeed) == 472);
static_assert(offsetof(FmodBufferedStreamGenerator, mTimelineMs) == 484);
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
