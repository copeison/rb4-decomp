#include "audio/core/music/MoggMusicGenerator.h"

#include "audio/core/generators/GeneratorPool.h"
#include "audio/core/generators/MoggGenerator.h"
#include "audio/core/resources/MoggMusicResource.h"
#include "audio/core/system/SoundManager.h"

Symbol MoggMusicGenerator::kTypeId;
CritSec MoggMusicGeneratorManager::mMoggMusicMapLock;
eastl::map<Symbol, MoggMusicResource*> MoggMusicGeneratorManager::mMoggMusicMap;
const char* MoggMusicGeneratorManager::kIdStr = "MoggMusicGeneratorManager";

namespace {

// The Mogg manager's id, interned on first use and inlined into Play; the
// local static is at 0x19C8620. The map has it as the inline
// MoggGeneratorManager::Id(). Name not in the reference map.
Symbol MoggManagerId() {
    static Symbol id;
    if (id == Symbol()) {
        id = Symbol("MoggGeneratorManager");
    }
    return id;
}

}  // namespace

// Reconstructed from eboot.elf at 0x4BD40.
void MoggMusicGeneratorManager::RegisterMoggMusicResource(MoggMusicResource* resource) {
    ScopedCritSec lock(mMoggMusicMapLock);
    mMoggMusicMap[resource->GetSoundId()] = resource;
}

// Reconstructed from eboot.elf at 0x4BE30.
bool MoggMusicGeneratorManager::UnregisterMoggMusicResource(MoggMusicResource* resource) {
    ScopedCritSec lock(mMoggMusicMapLock);
    bool removed = false;
    for (auto it = mMoggMusicMap.begin(); it != mMoggMusicMap.end();) {
        if (it->second == resource) {
            it = mMoggMusicMap.erase(it);
            removed = true;
        } else {
            ++it;
        }
    }
    return removed;
}

// Reconstructed from eboot.elf at 0x4BF00. The song streams through a
// paused voice of the Mogg manager that plays the resource's mogg with this
// generator as its audio-thread client; without one the generator goes back
// to the pool. The request is copied whole, so the copy's destructor would
// also free a parameter list the caller owns.
AudioGenerator* MoggMusicGeneratorManager::Play(const PlayArgs& args) {
    ResourcePtr<MoggMusicResource> resource;
    {
        ScopedCritSec lock(mMoggMusicMapLock);
        auto it = mMoggMusicMap.find(args.mName);
        if (it == mMoggMusicMap.end()) {
            return nullptr;
        }
        resource = it->second;
    }
    AudioEmitterCom* emitter =
        args.mEmitter != nullptr ? args.mEmitter : theSoundManager.GetDefault2DEmitter();
    AudioRenderTarget* target = gAudioRenderTargets.Find(args.mRenderTarget, true);
    auto* generator = GeneratorPool::Allocate<MoggMusicGenerator>(*this, target, emitter);
    if (generator == nullptr) {
        return nullptr;
    }
    generator->mMoggGenerator = nullptr;
    generator->mResource = nullptr;
    auto* moggManager =
        static_cast<MoggGeneratorManager*>(theSoundManager._GetManager(MoggManagerId()));
    if (moggManager == nullptr) {
        FreeGenerator(generator);
        return nullptr;
    }
    PlayArgs moggArgs;
    moggArgs = args;
    moggArgs.mFormat = 0;
    moggArgs.mName = resource->mMoggName;
    moggArgs.mStartPaused = true;
    auto* mogg = static_cast<MoggGenerator*>(moggManager->PlayWithCallback(moggArgs, generator));
    if (mogg == nullptr) {
        FreeGenerator(generator);
        return nullptr;
    }
    generator->Setup(mogg, resource, args);
    return generator;
}

// Reconstructed from eboot.elf at 0x4BD30.
void MoggMusicGeneratorManager::Init() {
    AudioGeneratorManager::Init();
}

// Reconstructed from eboot.elf at 0x4D750.
int MoggMusicGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0x4D760.
Symbol MoggMusicGeneratorManager::GetId() {
    return Id();
}

// Reconstructed from eboot.elf at 0x4D800.
Symbol MoggMusicGeneratorManager::GetResourceExt() {
    return ResExt();
}

// Reconstructed from eboot.elf at 0x4D8A0.
AudioGenerator* MoggMusicGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    return GeneratorPool::LockIfOwned(*this, mPool, handle, index);
}

// Reconstructed from eboot.elf at 0x4D920.
void MoggMusicGeneratorManager::SendStopToAllGenerators() {
    GeneratorPool::SendStop(*this, mPool);
}

// Reconstructed from eboot.elf at 0x4D990.
void MoggMusicGeneratorManager::SendKillToAllGenerators() {
    GeneratorPool::SendKill(*this, mPool);
}

// Reconstructed from eboot.elf at 0x4DA00.
void MoggMusicGeneratorManager::GetActiveHandles(void* handles) {
    GeneratorPool::GetActiveHandles(*this, mPool, handles);
}

// Reconstructed from eboot.elf at 0x4DB50.
void MoggMusicGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0x4DB60.
void MoggMusicGeneratorManager::_InitGeneratorPool() {
    GeneratorPool::Init(*this, mPool);
}

// Reconstructed from eboot.elf at 0x4DD10.
bool MoggMusicGeneratorManager::_DeleteGeneratorPool() {
    return GeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x4DDC0, which jumps to the base
// destructor.
MoggMusicGeneratorManager::~MoggMusicGeneratorManager() {}
