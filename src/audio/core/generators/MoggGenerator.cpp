#include "audio/core/generators/MoggGenerator.h"

#include "audio/core/decoders/ByteGrinder.h"
#include "audio/core/decoders/VorbisReader.h"
#include "audio/core/generators/GeneratorPool.h"
#include "audio/core/output/AudioRenderTarget.h"
#include "audio/core/resources/MoggResource.h"
#include "audio/core/streams/StreamReaderThread.h"
#include "audio/core/system/SoundManager.h"

// The object's statics, in the order of its static initializer at 0x4BBF0.
// The generator class itself is not reconstructed; its members span
// 0x480D0 to 0x4B16F and 0x4B770 to 0x4BA3F.
const char* MoggGeneratorManager::kIdStr = "MoggGeneratorManager";
float MoggGeneratorManager::mDefaultBufferSizeMs = 3000.0F;
float MoggGeneratorManager::mDefaultBufferLagRatio = 0.85F;
Symbol MoggGenerator::kTypeId;
CritSec MoggGeneratorManager::mMoggFileMapLock;
eastl::map<Symbol, Symbol> MoggGeneratorManager::mMoggFileMap;

// Reconstructed from eboot.elf at 0x47A80.
void MoggGeneratorManager::Init() {
    VorbisReader::Init();
    ByteGrinderInit();
    StreamReaderThread::Init();
    AudioGeneratorManager::Init();
}

// Reconstructed from eboot.elf at 0x47AB0.
void MoggGeneratorManager::RegisterMoggResource(MoggResource* resource) {
    ScopedCritSec lock(mMoggFileMapLock);
    mMoggFileMap[resource->mSoundName] = resource->mFilePath;
}

// Reconstructed from eboot.elf at 0x47BB0.
bool MoggGeneratorManager::UnregisterMoggResource(MoggResource* resource) {
    ScopedCritSec lock(mMoggFileMapLock);
    const Symbol file = resource->mFilePath;
    bool removed = false;
    for (auto it = mMoggFileMap.begin(); it != mMoggFileMap.end();) {
        const auto current = it;
        ++it;
        if (current->second == file) {
            mMoggFileMap.erase(current);
            removed = true;
        }
    }
    return removed;
}

// Reconstructed from eboot.elf at 0x47C80.
void MoggGeneratorManager::AddResourceAlias(const char* file, Symbol alias) {
    ScopedCritSec lock(mMoggFileMapLock);
    const Symbol path(file);
    mMoggFileMap[alias] = path;
}

// Reconstructed from eboot.elf at 0x47D80.
Symbol MoggGeneratorManager::_FindSound(Symbol name) {
    ScopedCritSec lock(mMoggFileMapLock);
    const auto it = mMoggFileMap.find(name);
    if (it != mMoggFileMap.end()) {
        return it->second;
    }
    return Symbol("");
}

// Reconstructed from eboot.elf at 0x47E80.
AudioGenerator* MoggGeneratorManager::PlayWithCallback(
    const PlayArgs& args, AudioBusCallable* callback) {
    const Symbol file = _FindSound(args.mName);
    if (file == Symbol("")) {
        return nullptr;
    }
    return _AllocateAndSetUpGenerator(file.Str(), args, callback);
}

// Reconstructed from eboot.elf at 0x47FC0.
MoggGenerator* MoggGeneratorManager::_AllocateAndSetUpGenerator(
    const char* file, const PlayArgs& args, AudioBusCallable* callback) {
    AudioEmitterCom* emitter = args.mEmitter;
    if (emitter == nullptr) {
        emitter = theSoundManager.GetDefault2DEmitter();
    }
    AudioRenderTarget* target = gAudioRenderTargets.Find(args.mRenderTarget, true);
    MoggGenerator* generator = GeneratorPool::Allocate<MoggGenerator>(*this, target, emitter);
    if (generator == nullptr) {
        return nullptr;
    }
    return generator->Setup(file, args, callback) ? generator : nullptr;
}

// Reconstructed from eboot.elf at 0x4B170.
AudioGenerator* MoggGeneratorManager::Play(const PlayArgs& args) {
    return PlayWithCallback(args, nullptr);
}

// Reconstructed from eboot.elf at 0x4B180.
int MoggGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0x4B190.
Symbol MoggGeneratorManager::GetId() {
    return Id();
}

// Reconstructed from eboot.elf at 0x4B230.
Symbol MoggGeneratorManager::GetResourceExt() {
    return ResExt();
}

// Reconstructed from eboot.elf at 0x4B2D0.
AudioGenerator* MoggGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    return GeneratorPool::LockIfOwned(*this, mPool, handle, index);
}

// Reconstructed from eboot.elf at 0x4B340.
void MoggGeneratorManager::SendStopToAllGenerators() {
    GeneratorPool::SendStop(*this, mPool);
}

// Reconstructed from eboot.elf at 0x4B3B0.
void MoggGeneratorManager::SendKillToAllGenerators() {
    GeneratorPool::SendKill(*this, mPool);
}

// Reconstructed from eboot.elf at 0x4B420.
void MoggGeneratorManager::GetActiveHandles(void* handles) {
    GeneratorPool::GetActiveHandles(*this, mPool, handles);
}

// Reconstructed from eboot.elf at 0x4B570.
void MoggGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0x4B580.
void MoggGeneratorManager::_InitGeneratorPool() {
    GeneratorPool::Init(*this, mPool);
}

// Reconstructed from eboot.elf at 0x4B690.
bool MoggGeneratorManager::_DeleteGeneratorPool() {
    return GeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x4B740, which jumps to the base
// destructor.
MoggGeneratorManager::~MoggGeneratorManager() {}
