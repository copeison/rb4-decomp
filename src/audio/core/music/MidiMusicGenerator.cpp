#include "audio/core/music/MidiMusicGenerator.h"

#include "audio/core/generators/GeneratorPool.h"
#include "audio/core/instruments/InstrumentGenerator.h"
#include "audio/core/resources/MidiMusicResource.h"
#include "audio/core/system/SoundManager.h"

Symbol MidiMusicGenerator::kTypeId;
CritSec MidiMusicGeneratorManager::mMidiMusicMapLock;
eastl::map<Symbol, MidiMusicResource*> MidiMusicGeneratorManager::mMidiMusicMap;
const char* MidiMusicGeneratorManager::kIdStr = "MidiMusicGeneratorManager";

namespace {

// The SynthRack manager's id, interned on first use and inlined into Play;
// the local static is at 0x19C85A8. The map's build has it as the inline
// MultiInstrumentGeneratorManager::Id(), whose local static the map emits
// with this object. Name not in the reference map.
Symbol SynthRackManagerId() {
    static Symbol id;
    if (id == Symbol()) {
        id = Symbol("SynthRackGeneratorManager");
    }
    return id;
}

}  // namespace

// Reconstructed from eboot.elf at 0x44440.
void MidiMusicGeneratorManager::RegisterMidiMusicResource(MidiMusicResource* resource) {
    ScopedCritSec lock(mMidiMusicMapLock);
    mMidiMusicMap[resource->GetSoundId()] = resource;
}

// Reconstructed from eboot.elf at 0x44530.
bool MidiMusicGeneratorManager::UnregisterMidiMusicResource(MidiMusicResource* resource) {
    ScopedCritSec lock(mMidiMusicMapLock);
    bool removed = false;
    for (auto it = mMidiMusicMap.begin(); it != mMidiMusicMap.end();) {
        if (it->second == resource) {
            it = mMidiMusicMap.erase(it);
            removed = true;
        } else {
            ++it;
        }
    }
    return removed;
}

// Reconstructed from eboot.elf at 0x44600.
ResourcePtr<MidiMusicResource> MidiMusicGeneratorManager::FindMidiMusicResource(Symbol name) {
    ScopedCritSec lock(mMidiMusicMapLock);
    auto it = mMidiMusicMap.find(name);
    if (it == mMidiMusicMap.end()) {
        return ResourcePtr<MidiMusicResource>();
    }
    return ResourcePtr<MidiMusicResource>(it->second);
}

// Reconstructed from eboot.elf at 0x44700. The song plays through a voice
// of the SynthRack manager; without one the generator goes back to the
// pool.
AudioGenerator* MidiMusicGeneratorManager::Play(const PlayArgs& args) {
    ResourcePtr<MidiMusicResource> resource;
    {
        ScopedCritSec lock(mMidiMusicMapLock);
        auto it = mMidiMusicMap.find(args.mName);
        if (it == mMidiMusicMap.end()) {
            return nullptr;
        }
        resource = it->second;
    }
    AudioEmitter* emitter =
        args.mEmitter != nullptr ? args.mEmitter : theSoundManager.GetDefault2DEmitter();
    AudioRenderTarget* target = gAudioRenderTargets.Find(args.mRenderTarget, true);
    auto* generator = GeneratorPool::Allocate<MidiMusicGenerator>(*this, target, emitter);
    if (generator == nullptr) {
        return nullptr;
    }
    AudioGeneratorManager* rack = theSoundManager._GetManager(SynthRackManagerId());
    auto* instrument = static_cast<InstrumentGenerator*>(
        GeneratorPool::Allocate<AudioGenerator>(*rack, target, emitter));
    if (instrument == nullptr) {
        FreeGenerator(generator);
        return nullptr;
    }
    generator->Setup(resource, instrument, args);
    return generator;
}

// Reconstructed from eboot.elf at 0x44430.
void MidiMusicGeneratorManager::Init() {
    AudioGeneratorManager::Init();
}

// Reconstructed from eboot.elf at 0x463C0.
int MidiMusicGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0x463D0.
Symbol MidiMusicGeneratorManager::GetId() {
    return Id();
}

// Reconstructed from eboot.elf at 0x46470.
Symbol MidiMusicGeneratorManager::GetResourceExt() {
    return ResExt();
}

// Reconstructed from eboot.elf at 0x46510.
AudioGenerator* MidiMusicGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    return GeneratorPool::LockIfOwned(*this, mPool, handle, index);
}

// Reconstructed from eboot.elf at 0x46580.
void MidiMusicGeneratorManager::SendStopToAllGenerators() {
    GeneratorPool::SendStop(*this, mPool);
}

// Reconstructed from eboot.elf at 0x465F0.
void MidiMusicGeneratorManager::SendKillToAllGenerators() {
    GeneratorPool::SendKill(*this, mPool);
}

// Reconstructed from eboot.elf at 0x46660.
void MidiMusicGeneratorManager::GetActiveHandles(void* handles) {
    GeneratorPool::GetActiveHandles(*this, mPool, handles);
}

// Reconstructed from eboot.elf at 0x467B0.
void MidiMusicGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0x467C0.
void MidiMusicGeneratorManager::_InitGeneratorPool() {
    GeneratorPool::Init(*this, mPool);
}

// Reconstructed from eboot.elf at 0x46990.
bool MidiMusicGeneratorManager::_DeleteGeneratorPool() {
    return GeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x46A40, which jumps to the base
// destructor.
MidiMusicGeneratorManager::~MidiMusicGeneratorManager() {}
