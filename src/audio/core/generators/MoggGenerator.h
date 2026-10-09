#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"
#include "os/threading/CritSec.h"
#include "utl/containers/Map.h"
#include "utl/text/Symbol.h"

class AudioBusCallable;
class MoggResource;

// Plays a Mogg file: a multitrack Ogg stream mixed through bus voices of
// gAudioBusGeneratorManager (audio/MoggGenerator.o). The class has not been
// reconstructed; it adds no virtual functions, so its 32 slots (vtable
// 0x18E0AD0) are declared in AudioGenerator's order. The object is 464
// bytes.
class MoggGenerator : public AudioGenerator {
public:
    // Out of line at 0x4B830; _InitGeneratorPool calls it for each voice.
    MoggGenerator();

    void Pause() override;                                    // slot 0: 0x493A0
    void Continue() override;                                 // slot 1: 0x49490
    void Stop() override;                                     // slot 2: 0x49690
    float GetElapsedMs() override;                            // slot 4: 0x49580
    float GetTimelineMs() override;                           // slot 5: 0x49640
    float GetLengthMs() const override;                       // slot 6: 0x4B770
    void SeekToMs(float ms) override;                         // slot 7: 0x49650
    void SetSpeed(float speed, bool immediate) override;      // slot 8: 0x49590
    float GetSpeed(bool* changing) override;                  // slot 9: 0x4B780
    bool SetParameter(Symbol name, float value) override;     // slot 10: 0x49BB0
    bool GetParameter(Symbol name, float& value) override;    // slot 11: 0x49C10
    void SetGain(float gain, float fadeSecs, PostFadeOption option) override;  // slot 12: 0x49AD0
    float GetGain() const override;                           // slot 13: 0x49B30
    // Slot 14 at 0x49B40. The map has SetMute(bool).
    void SetMute(bool mute, bool immediate) override;
    bool GetMute() const override;                            // slot 15: 0x49BA0
    void Init(AudioGeneratorManager* manager, int index) override;  // slot 19: 0x48B30
    ~MoggGenerator() override;                                // slots 20-21: 0x48E60, 0x49060
    bool Poll() override;                                     // slot 22: 0x49080
    void Release() override;                                  // slot 23: 0x498E0
    float GetPrimaryStreamValue() override;                   // slot 27: 0x49660
    void _InitTypeId() override;                              // slot 28: 0x4B7B0
    void Kill() override;                                     // slot 29: 0x49720
    AudioGenerator* GetGeneratorOfType(Symbol type) override; // slot 30: 0x4B800
    Symbol GetTypeId() override;                              // slot 31: 0x4B820

    // Opens the file, builds the stream and its bus voices and applies the
    // request. At 0x480D0.
    bool Setup(const char* file, const PlayArgs& args, AudioBusCallable* callback);

    static Symbol kTypeId;  // 0x19C85C0, "MoggGenerator"

    // Not reconstructed. +80 holds an embedded stream whose constructor is
    // inlined: it installs the vtable 0x18E5B58 that StandardStream's
    // constructor at 0xC9C40 installs (the map emits StandardStream() in
    // this object). +376 and +408 hold the vectors of audio buses and bus
    // generators, and +440 a list. Name not in the reference map.
    unsigned char mStreamState[384];
};

static_assert(sizeof(MoggGenerator) == 464);

// The pool of Mogg generators, and the registry that maps sound names to
// Mogg files. The vtable is at 0x18E0BE0. The object is 72 bytes.
class MoggGeneratorManager : public AudioGeneratorManager {
public:
    // Inlined into InitGeneratorManager<MoggGeneratorManager> (0x61C0).
    explicit MoggGeneratorManager(int poolSize) {
        mPoolSize = poolSize;
    }

    // The manager's id, interned on first use. Inlined into GetId (0x4B190)
    // and the MoggMusic generator's object (0x4C0D8), which share its static
    // at 0x19C8620.
    static Symbol Id() {
        static Symbol id("");
        if (id == Symbol("")) {
            id = Symbol("MoggGeneratorManager");
        }
        return id;
    }
    // The resource extension, interned on first use. Inlined into
    // GetResourceExt (0x4B230); the static is at 0x19C8630.
    static Symbol ResExt() {
        static Symbol resExt("");
        if (resExt == Symbol("")) {
            resExt = Symbol(".mogg");
        }
        return resExt;
    }

    // Slot 0 at 0x4B170: PlayWithCallback without a callback.
    AudioGenerator* Play(const PlayArgs& args) override;
    // Slot 3 at 0x47A80: initializes the Vorbis reader, the Mogg key
    // derivation and the stream reader thread, then registers the manager.
    void Init() override;
    int GetIndex() override;               // slot 6: 0x4B180
    Symbol GetId() override;               // slot 7: 0x4B190
    Symbol GetResourceExt() override;      // slot 8: 0x4B230
    AudioGenerator* LockIfOwned(unsigned int handle, int index) override;  // slot 9: 0x4B2D0
    void SendStopToAllGenerators() override;  // slot 10: 0x4B340
    void SendKillToAllGenerators() override;  // slot 11: 0x4B3B0
    void GetActiveHandles(void* handles) override;  // slot 12: 0x4B420
    void _SetManagerIndex(int index) override;      // slot 13: 0x4B570
    void _InitGeneratorPool() override;    // slot 14: 0x4B580
    bool _DeleteGeneratorPool() override;  // slot 15: 0x4B690
    ~MoggGeneratorManager() override;      // slots 16-17: 0x4B740, 0x4B750

    // Maps the resource's sound name to its file. At 0x47AB0.
    static void RegisterMoggResource(MoggResource* resource);
    // Removes every name that maps to the resource's file; false when none
    // did. At 0x47BB0.
    static bool UnregisterMoggResource(MoggResource* resource);
    // Maps another sound name to the file. At 0x47C80; MoggResource's
    // AddAlias forwards here.
    static void AddResourceAlias(const char* file, Symbol alias);

    // The file registered for the sound name, or the empty symbol. At
    // 0x47D80; PlayWithCallback inlines it.
    Symbol _FindSound(Symbol name);
    // Plays the request's sound when a file is registered for it; the
    // callback, when given, is passed to the generator's Setup. At 0x47E80.
    AudioGenerator* PlayWithCallback(const PlayArgs& args, AudioBusCallable* callback);
    // Takes an idle voice bound to the request's render target and emitter
    // (the default 2D emitter without one) and sets it up. A voice whose
    // Setup fails is not released. At 0x47FC0.
    MoggGenerator* _AllocateAndSetUpGenerator(
        const char* file, const PlayArgs& args, AudioBusCallable* callback);

    static const char* kIdStr;  // 0x19B00B0

    // The stream buffer length each generator's Setup uses when the request
    // gives none. At 0x19B00B8; game code reads it too.
    static float mDefaultBufferSizeMs;
    // Passed with the buffer length to the stream's setup (0xC9F80), which
    // stores it at +280: the fraction of the buffer by which the channels may
    // drift apart before the stream resynchronizes them. At 0x19B00BC. Name
    // not in the reference map; the evidence is weak.
    static float mDefaultBufferLagRatio;

    // Sound names to Mogg files. At 0x19C85D8.
    static eastl::map<Symbol, Symbol> mMoggFileMap;
    // Guards mMoggFileMap. At 0x19C85C8. Name not in the reference map.
    static CritSec mMoggFileMapLock;

    MoggGenerator* mPool;  // Name not in the reference map.
};

static_assert(offsetof(MoggGeneratorManager, mPool) == 64);
static_assert(sizeof(MoggGeneratorManager) == 72);
