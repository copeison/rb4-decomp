// The map's audio/MultiInstrumentGenerator.o; this build renamed the
// manager to MultiFusionGeneratorManager. The object spans 0x4E780 to
// 0x5277F. The generator class is not reconstructed; its members span
// 0x4EC90 to 0x514FF and 0x51D60 to 0x5264F.
#include "audio/core/instruments/MultiFusionGenerator.h"

#include "audio/core/generators/GeneratorPool.h"
#include "audio/core/system/SoundManager.h"

// The object's statics, in the order of its static initializer at 0x52650.
const char* MultiFusionGeneratorManager::kIdStr = "MultiFusionGeneratorManager";
CritSec MultiFusionGeneratorManager::mResourceMapLock;
eastl::map<Symbol, MultiFusionResource*> MultiFusionGeneratorManager::mResourceMap;
Symbol MultiFusionGenerator::kTypeId;

// Reconstructed from eboot.elf at 0x4E790.
void MultiFusionGeneratorManager::Init() {
    AudioGeneratorManager::Init();
}

// Reconstructed from eboot.elf at 0x4E7A0.
void MultiFusionGeneratorManager::RegisterMultiFusionResource(
    Symbol name, MultiFusionResource* resource) {
    ScopedCritSec lock(mResourceMapLock);
    mResourceMap[name] = resource;
}

// Reconstructed from eboot.elf at 0x4E8A0.
bool MultiFusionGeneratorManager::UnregisterMultiFusionResource(MultiFusionResource* resource) {
    ScopedCritSec lock(mResourceMapLock);
    bool removed = false;
    for (auto it = mResourceMap.begin(); it != mResourceMap.end();) {
        const auto current = it;
        ++it;
        if (current->second == resource) {
            mResourceMap.erase(current);
            removed = true;
        }
    }
    return removed;
}

// Reconstructed from eboot.elf at 0x4E970. A failed setup returns the voice
// to the pool.
AudioGenerator* MultiFusionGeneratorManager::Play(const PlayArgs& args) {
    MultiFusionResource* resource = nullptr;
    bool known;
    {
        ScopedCritSec lock(mResourceMapLock);
        static Symbol sBareExt(".multifusion");  // 0x19C8718. Name not in the reference map.
        const auto it = mResourceMap.find(args.mName);
        known = args.mName == sBareExt;
        if (it != mResourceMap.end()) {
            resource = it->second;
            known = true;
        }
    }
    if (!known) {
        return nullptr;
    }

    AudioEmitterCom* emitter = args.mEmitter;
    if (emitter == nullptr) {
        emitter = theSoundManager.GetDefault2DEmitter();
    }
    AudioRenderTarget* target = gAudioRenderTargets.Find(args.mRenderTarget, true);
    MultiFusionGenerator* generator =
        GeneratorPool::Allocate<MultiFusionGenerator>(*this, target, emitter);
    if (generator == nullptr) {
        return nullptr;
    }
    const bool ready =
        resource != nullptr ? generator->Setup(args, resource) : generator->Setup(args);
    if (ready) {
        return generator;
    }
    GeneratorPool::Release(*generator);
    return nullptr;
}

// Reconstructed from eboot.elf at 0x51500.
int MultiFusionGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0x51510.
Symbol MultiFusionGeneratorManager::GetId() {
    return Id();
}

// Reconstructed from eboot.elf at 0x515B0.
Symbol MultiFusionGeneratorManager::GetResourceExt() {
    return ResExt();
}

// Reconstructed from eboot.elf at 0x51650.
AudioGenerator* MultiFusionGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    return GeneratorPool::LockIfOwned(*this, mPool, handle, index);
}

// Reconstructed from eboot.elf at 0x516E0. The binary calls the generator's
// own Stop entry (primary slot 57); the declaration does not reproduce that
// slot, so the call goes through AudioGenerator.
void MultiFusionGeneratorManager::SendStopToAllGenerators() {
    ScopedCritSecPtr tracker(&mCritSec);
    for (int index = 0; index < mPoolSize; ++index) {
        static_cast<AudioGenerator&>(mPool[index]).Stop();
    }
}

// Reconstructed from eboot.elf at 0x51750.
void MultiFusionGeneratorManager::SendKillToAllGenerators() {
    GeneratorPool::SendKill(*this, mPool);
}

// Reconstructed from eboot.elf at 0x517D0.
void MultiFusionGeneratorManager::GetActiveHandles(void* handles) {
    GeneratorPool::GetActiveHandles(*this, mPool, handles);
}

// Reconstructed from eboot.elf at 0x51920.
void MultiFusionGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0x51930, which inlines the generator's
// constructor. The binary calls the generator's own Init entry (primary
// slot 54); the declaration does not reproduce that slot, so the call goes
// through AudioGenerator.
void MultiFusionGeneratorManager::_InitGeneratorPool() {
    mPool = new MultiFusionGenerator[mPoolSize];
    for (int index = 0; index < mPoolSize; ++index) {
        static_cast<AudioGenerator&>(mPool[index]).Init(this, index);
        mFreeList.PushBack(mPool[index]);
    }
}

// Reconstructed from eboot.elf at 0x51C80.
bool MultiFusionGeneratorManager::_DeleteGeneratorPool() {
    return GeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x51D30, which jumps to the base
// destructor.
MultiFusionGeneratorManager::~MultiFusionGeneratorManager() {}
