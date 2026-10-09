#include "audio/fmod/resources/FmodAudioStreamResource.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <unistd.h>

#include "audio/fmod/system/FmodPlatform.h"
#include "os/files/File.h"
#include "os/memory/MemMgr.h"
#include "utl/time/Timer.h"
#include "os/threading/CritSec.h"

namespace {

// The registry of loaded streams, keyed by resolved file, is an EASTL map
// at 0x19F2E00 guarded by the CritSec at 0x19F2DF0. std::map stands in for
// the EASTL tree. Names not in the reference map.
CritSec gStreamCritSec;
std::map<const char*, FmodAudioStreamResource*> gStreams;

Symbol EmptySymbol() {
    return Symbol("");
}

// File-system mode used to test for the stream file. Name not in the
// reference map.
constexpr int kStreamFileMode = 2;

// Inlined copy of the engine's existence test at 0x3788B0 for non-default
// modes: the file is opened, checked and deleted.
bool StreamFileExists(const char* path) {
    File* file = File::NewFile(path, kStreamFileMode);
    if (file == nullptr) {
        return false;
    }
    const bool exists = !file->Fail();
    delete file;
    return exists;
}

// Returns the part of a path after its last separator, as 0x2453D0 does.
const char* FileName(const char* path) {
    const char* name = path + std::strlen(path);
    while (name > path && name[-1] != '/' && name[-1] != '\\') {
        --name;
    }
    return name;
}

// Converts backslashes to slashes in place, as 0x2448D0 does.
char* NormalizeSlashes(char* path) {
    for (char* c = path; *c != '\0'; ++c) {
        if (*c == '\\') {
            *c = '/';
        }
    }
    return path;
}

}  // namespace

ResourceMetaData FmodAudioStreamResource::sMetaData;

// Reconstructed from eboot.elf at 0x271660.
FmodAudioStreamResource::FmodAudioStreamResource()
    : mFile(""),
      mLengthMs(0.0F),
      mStatus(kStatusOk),
      mDecodeFailed(false),
      mAsyncProcess(nullptr),
      mSound(nullptr) {}

// Reconstructed from eboot.elf at 0x271750.
FmodAudioStreamResource::~FmodAudioStreamResource() {
    if (mSound != nullptr) {
        mSound->release();
        mSound = nullptr;
    }
    if (mFile != EmptySymbol()) {
        _Unregister();
    }
}

// Reconstructed from eboot.elf at 0x271D00.
void FmodAudioStreamResource::_Init(ResourceMetaData& metaData) {
    metaData.mExtensions.push_back(Symbol("mp3"));
    metaData.mExtensions.push_back(Symbol("wav"));
    metaData.mExtensions.push_back(Symbol("aac"));
    metaData.mExtensions.push_back(Symbol("ogg"));
    metaData.mExtensions.push_back(Symbol("m4a"));
    metaData.mCategory = Symbol("Streaming Audio");
    metaData.mTypeFlags[0] = false;
    metaData.mTypeFlags[4] = true;
    metaData.mTypeFlags[6] = true;
    metaData.mTypeOption = 1;
}

// Reconstructed from eboot.elf at 0x272D10.
ResourceMetaData* FmodAudioStreamResource::GetMetaData() const {
    return &sMetaData;
}

// Reconstructed from eboot.elf at 0x272DC0.
bool FmodAudioStreamResource::IsA(Symbol type) const {
    for (const ResourceMetaData* metaData = &sMetaData; metaData != nullptr;
         metaData = metaData->mParent) {
        if (metaData->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x272D20.
Symbol FmodAudioStreamResource::GetId() const {
    static Symbol sId("");
    if (sId == EmptySymbol()) {
        sId = Symbol("FmodAudioStreamResource");
    }
    return sId;
}

// Reconstructed from eboot.elf at 0x272340. The file is resolved, opened once
// to validate its format, and registered under its resolved path.
void FmodAudioStreamResource::LoadFile() {
    mLengthMs = -1.0F;
    mStatus = kStatusOk;
    // When the platform redirects files (the flag at 0x19FEE10), the path is
    // first mapped through 0x244960; that branch is not reconstructed.
    char path[512];
    std::snprintf(path, sizeof(path), "%s", mPath.mPath.Str());
    mFile = Symbol(NormalizeSlashes(path));
    if (!StreamFileExists(mFile.Str())) {
        mFile = EmptySymbol();
        mStatus = kStatusFileNotFound;
        return;
    }

    FMOD::Sound* sound = nullptr;
    FMOD::System* lowLevel = FModSystem::Get()->mLowLevelSystem;
    if (lowLevel == nullptr ||
        (lowLevel->createSound(mFile.Str(), FMOD_CREATESTREAM, nullptr, &sound), sound == nullptr)) {
        mFile = EmptySymbol();
        mStatus = kStatusNoSound;
        return;
    }

    FMOD_SOUND_TYPE type;
    FMOD_SOUND_FORMAT format;
    int channels;
    int bits;
    sound->getFormat(&type, &format, &channels, &bits);
    if (format != FMOD_SOUND_FORMAT_PCM16 || channels < 1 || channels > 2 || bits != 16) {
        mStatus = kStatusUnsupportedFormat;
    }
    sound->release();
    _Register();
}

// Reconstructed from eboot.elf at 0x272710.
bool FmodAudioStreamResource::Load(BinStream&, bool) {
    return false;
}

// Reconstructed from eboot.elf at 0x272720.
void FmodAudioStreamResource::Save(BinStream&, bool) {}

// Reconstructed from eboot.elf at 0x272DF0.
bool FmodAudioStreamResource::Fail() const {
    return mFile == EmptySymbol();
}

// Reconstructed from eboot.elf at 0x272730.
void FmodAudioStreamResource::OpenSound() {
    if (mSound == nullptr) {
        FModSystem::Get()->mLowLevelSystem->createSound(
            mFile.Str(), FMOD_CREATESTREAM, nullptr, &mSound);
    }
}

// Reconstructed from eboot.elf at 0x272770. The length is cached; a sound
// opened only to measure it is released again.
float FmodAudioStreamResource::GetLengthMs() {
    if (mLengthMs >= 0.0F) {
        return mLengthMs;
    }
    FMOD::Sound* const wasOpen = mSound;
    if (mSound == nullptr) {
        FModSystem::Get()->mLowLevelSystem->createSound(
            mFile.Str(), FMOD_CREATESTREAM, nullptr, &mSound);
        if (mSound == nullptr) {
            return 0.0F;
        }
    }
    unsigned int length;
    mSound->getLength(&length, FMOD_TIMEUNIT_MS);
    mLengthMs = static_cast<float>(length);
    if (wasOpen == nullptr && mSound != nullptr) {
        mSound->release();
        mSound = nullptr;
    }
    return mLengthMs;
}

// Reconstructed from eboot.elf at 0x272C30.
void FmodAudioStreamResource::WaitForAsyncProcessToComplete() {
    while (mAsyncProcess != nullptr) {
        core_poll_and_update_time();
        usleep(1000);
    }
}

// Reconstructed from eboot.elf at 0x272C60. Cancels the running decode and,
// when asked, waits for its worker to finish.
void FmodAudioStreamResource::StopAsyncProcess(bool wait) {
    mAsyncCritSec.Enter();
    if (mAsyncProcess == nullptr) {
        mAsyncCritSec.Exit();
        return;
    }
    mAsyncProcess->mCancel = true;
    mAsyncCritSec.Exit();
    if (!wait) {
        return;
    }
    _FMODSoundAsyncSampleProcessor* process;
    do {
        core_poll_and_update_time();
        usleep(1000);
        mAsyncCritSec.Enter();
        process = mAsyncProcess;
        mAsyncCritSec.Exit();
    } while (process != nullptr);
}

// Reconstructed from eboot.elf at 0x271B20. Streams are registered by file
// name, which is what playback requests name.
void FmodAudioStreamResource::_Register() {
    ScopedCritSecPtr tracker(&gStreamCritSec);
    gStreams[Symbol(FileName(mFile.Str())).Str()] = this;
}

// Reconstructed from eboot.elf at 0x271830. Every entry for this resource is
// removed.
bool FmodAudioStreamResource::_Unregister() {
    ScopedCritSecPtr tracker(&gStreamCritSec);
    bool removed = false;
    for (auto entry = gStreams.begin(); entry != gStreams.end();) {
        if (entry->second == this) {
            entry = gStreams.erase(entry);
            removed = true;
        } else {
            ++entry;
        }
    }
    return removed;
}

// Reconstructed from eboot.elf at 0x271C20.
ResourcePtr<FmodAudioStreamResource> FmodAudioStreamResource::Find(Symbol file) {
    ScopedCritSecPtr tracker(&gStreamCritSec);
    const auto entry = gStreams.find(file.Str());
    if (entry == gStreams.end()) {
        return ResourcePtr<FmodAudioStreamResource>();
    }
    return ResourcePtr<FmodAudioStreamResource>(entry->second);
}

// Reconstructed from eboot.elf at 0x2720F0. A path that names no loaded
// resource creates and loads a new stream.
ResourcePtr<FmodAudioStreamResource> FmodAudioStreamResource::GetOrLoad(ResourcePath path) {
    if (path.mPath == EmptySymbol()) {
        return ResourcePtr<FmodAudioStreamResource>();
    }
    if (Resource* existing = Resource::Get(path)) {
        return ResourcePtr<FmodAudioStreamResource>(
            static_cast<FmodAudioStreamResource*>(existing));
    }
    auto* resource = new FmodAudioStreamResource();
    resource->SetFile(path, true);
    resource->LoadFile();
    return ResourcePtr<FmodAudioStreamResource>(resource);
}

// Reconstructed from eboot.elf at 0x272F60. Waits for the stream to open,
// then feeds it block by block to the processor. A rejected block marks the
// resource as failed to decode.
int _FMODSoundAsyncSampleProcessor::ThreadStart() {
    FMOD_OPENSTATE openState;
    unsigned int percentBuffered = 0;
    bool starving = false;
    bool diskBusy = false;
    do {
        mSound->getOpenState(&openState, &percentBuffered, &starving, &diskBusy);
        if (openState == FMOD_OPENSTATE_READY) {
            break;
        }
    } while (!mCancel);
    if (mCancel) {
        return 0;
    }

    FMOD_SOUND_TYPE type;
    FMOD_SOUND_FORMAT format;
    int channels;
    int bits;
    float frequency;
    int priority;
    unsigned int numFrames = 0;
    mSound->getFormat(&type, &format, &channels, &bits);
    mSound->getDefaults(&frequency, &priority);
    mSound->getLength(&numFrames, FMOD_TIMEUNIT_PCM);
    mBlockFrames = mProcessor->ChainInit(
        static_cast<int>(frequency), FMOD_SOUND_FORMAT_PCM16, channels, numFrames);
    const unsigned int frameBytes = channels * 2;
    mBufferBytes = mBlockFrames * channels * 2;
    mBuffer = MemAlloc(mBufferBytes, "FMODSoundToPCMCallback", 8);

    unsigned int framesRead = 0;
    bool accepted;
    mSound->seekData(0);
    do {
        unsigned int bytesRead = 0;
        mSound->readData(mBuffer, mBufferBytes, &bytesRead);
        const unsigned int frames = bytesRead / frameBytes;
        accepted = mProcessor->ChainProcessSampleFrames(static_cast<char*>(mBuffer), bytesRead);
        if (bytesRead == 0 || !(accepted && !mCancel)) {
            break;
        }
        framesRead += frames;
    } while (framesRead < numFrames);
    if (!accepted) {
        mResource->mDecodeFailed = true;
        mCancel = true;
    }
    mSound->seekData(0);
    if (mCancel) {
        mProcessor->ChainCanceled();
    } else {
        mProcessor->ChainDone();
    }
    mProcessor = nullptr;
    return 0;
}

// Reconstructed from eboot.elf at 0x273150.
void _FMODSoundAsyncSampleProcessor::ThreadDone(int) {
    delete this;
}

// Reconstructed from eboot.elf at 0x272E60.
_FMODSoundAsyncSampleProcessor::~_FMODSoundAsyncSampleProcessor() {
    if (mSound != nullptr) {
        mSound->release();
        mSound = nullptr;
    }
    if (mBuffer != nullptr) {
        MemFree(mBuffer);
        mBuffer = nullptr;
    }
    mResource->mAsyncCritSec.Enter();
    mResource->mAsyncProcess = nullptr;
    mResource->mAsyncCritSec.Exit();
}
