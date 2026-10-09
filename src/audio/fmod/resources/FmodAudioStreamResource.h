#pragma once

#include <cstddef>

#include "audio/core/resources/Resource.h"
#include "audio/core/system/Audio.h"
#include "audio/fmod/api/fmod_api.h"
#include "os/threading/CritSec.h"

// Consumer of decoded PCM (audio/AsyncSampleProcessor.o). Processors form
// a chain: each Chain* member runs the matching On* hook and passes the call
// on. No implementation is linked into this build, so only slots 7-11 are
// fixed by their callers; the names of slots 0-6 come from the map's
// AsyncSampleProcessor and ToMonoFloatAsyncProcessor members, ordered like
// the Chain* slots, and their order is a guess.
class AsyncSampleProcessor {
public:
    virtual ~AsyncSampleProcessor();  // slots 0-1
    virtual void OnReset();           // slot 2
    virtual int OnInit(  // slot 3
        int sampleRate, FMOD_SOUND_FORMAT format, unsigned int numChannels, unsigned int numFrames);
    virtual bool OnProcessSampleFrames(char* data, unsigned int bytes);  // slot 4
    virtual void OnDone();            // slot 5
    virtual void OnCancel();          // slot 6
    // Slot 7: FmodAudioStreamResource::StartAsyncSampleProcessor (0x272B80)
    // resets the chain before queueing the decode.
    virtual void ChainReset();
    // Slot 8: announces the stream format and returns the block size in
    // frames.
    virtual int ChainInit(
        int sampleRate, FMOD_SOUND_FORMAT format, unsigned int numChannels, unsigned int numFrames);
    // Slot 9: consumes one block; false aborts the decode.
    virtual bool ChainProcessSampleFrames(char* data, unsigned int bytes);
    virtual void ChainDone();      // slot 10
    virtual void ChainCanceled();  // slot 11
};

// Work item run on an engine worker thread. The map emits its destructor in
// the FmodAudioStreamResource object and the queue in utl/ThreadCall, which
// has not been reconstructed. The vtable is at 0x18F0C48.
class ThreadCallback {
public:
    virtual ~ThreadCallback() {}         // slots 0-1: 0x273160, 0x273170
    virtual int ThreadStart() = 0;       // slot 2
    virtual void ThreadDone(int result) = 0;  // slot 3
};

// Runs finished ThreadCallback completions. At 0x219B80.
void ThreadCallPoll();

class FmodAudioStreamResource;

// Decodes a stream to 16-bit PCM for an AsyncSampleProcessor. The decode
// buffer is tagged "FMODSoundToPCMCallback". The vtable is at 0x18F0C18;
// the object is 56 bytes.
class _FMODSoundAsyncSampleProcessor : public ThreadCallback {
public:
    _FMODSoundAsyncSampleProcessor(
        FmodAudioStreamResource* resource, AsyncSampleProcessor* processor);
    ~_FMODSoundAsyncSampleProcessor() override;  // slots 0-1: 0x272E60, 0x272EE0
    int ThreadStart() override;                 // slot 2: 0x272F60
    void ThreadDone(int result) override;       // slot 3: 0x273150

    // Field names are not in the reference map.
    FmodAudioStreamResource* mResource;
    AsyncSampleProcessor* mProcessor;
    FMOD::Sound* mSound;
    void* mBuffer;
    int mBlockFrames;
    int mBufferBytes;
    // Set by StartAsyncSampleProcessor (0x272B80) when FMOD cannot create
    // the sound; the decode is then not queued.
    bool mCreateFailed;
    bool mCancel;
};

static_assert(offsetof(_FMODSoundAsyncSampleProcessor, mSound) == 24);
static_assert(offsetof(_FMODSoundAsyncSampleProcessor, mBlockFrames) == 40);
static_assert(offsetof(_FMODSoundAsyncSampleProcessor, mCancel) == 49);
static_assert(sizeof(_FMODSoundAsyncSampleProcessor) == 56);

// Streamed audio file played through FMOD, such as an mp3 or ogg track. The
// vtable is at 0x18F0BA0; the object is 104 bytes.
class FmodAudioStreamResource : public Resource {
public:
    // Load status. Names not in the reference map.
    enum Status : int {
        kStatusOk = 0,
        kStatusFileNotFound = 1,
        kStatusNoSound = 2,
        kStatusUnsupportedFormat = 3,
    };

    FmodAudioStreamResource();              // 0x271660
    ~FmodAudioStreamResource() override;    // slots 10-11: 0x271750, 0x271900

    ResourceMetaData* GetMetaData() const override;  // slot 0: 0x272D10
    Symbol GetId() const override;          // slot 1: 0x272D20
    bool IsA(Symbol type) const override;   // slot 2: 0x272DC0
    void LoadFile() override;               // slot 3: 0x272340
    bool Load(BinStream& stream, bool unknown) override;  // slot 4: 0x272710
    void Save(BinStream& stream, bool unknown) override;  // slot 5: 0x272720
    bool Fail() const override;             // slot 6: 0x272DF0

    // Reconstructed from eboot.elf at 0x2720F0.
    static ResourcePtr<FmodAudioStreamResource> GetOrLoad(ResourcePath path);
    // Finds a loaded stream by its resolved file. At 0x271C20. Name not in
    // the reference map.
    static ResourcePtr<FmodAudioStreamResource> Find(Symbol file);
    // Fills the type metadata at 0x19F2D98. At 0x271D00.
    static void _Init(ResourceMetaData& metaData);

    // At 0x19F2D98. Name not in the reference map.
    static ResourceMetaData sMetaData;

    float GetLengthMs();                    // 0x272770
    void WaitForAsyncProcessToComplete();   // 0x272C30
    void StopAsyncProcess(bool wait);       // 0x272C60
    // Opens the stream once. At 0x272730. Name not in the reference map.
    void OpenSound();
    // Reads an FMOD tag into a caller buffer. At 0x272840. Name not in the
    // reference map.
    int GetTag(const char* name, void* buffer, int capacity, int* size);
    // Adds and removes the resource in the registry of loaded streams. At
    // 0x271B20 and 0x271830. Names not in the reference map.
    void _Register();
    bool _Unregister();

    // Field names are not in the reference map.
    Symbol mFile;             // Resolved file; the empty symbol when unavailable.
    float mLengthMs;
    Status mStatus;
    bool mDecodeFailed;
    CritSec mAsyncCritSec;
    _FMODSoundAsyncSampleProcessor* mAsyncProcess;
    FMOD::Sound* mSound;
};

static_assert(offsetof(FmodAudioStreamResource, mFile) == 48);
static_assert(offsetof(FmodAudioStreamResource, mLengthMs) == 56);
static_assert(offsetof(FmodAudioStreamResource, mStatus) == 60);
static_assert(offsetof(FmodAudioStreamResource, mDecodeFailed) == 64);
static_assert(offsetof(FmodAudioStreamResource, mAsyncCritSec) == 72);
static_assert(offsetof(FmodAudioStreamResource, mAsyncProcess) == 88);
static_assert(offsetof(FmodAudioStreamResource, mSound) == 96);
static_assert(sizeof(FmodAudioStreamResource) == 104);
