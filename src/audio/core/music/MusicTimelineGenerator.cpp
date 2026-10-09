#include "audio/core/music/MusicTimelineGenerator.h"

#include "audio/core/generators/GeneratorPool.h"

Symbol MusicTimelineGenerator::kTypeId;
const char* MusicTimelineGeneratorManager::kMusicTimelineSoundId = ".musictimeline";
const char* MusicTimelineGeneratorManager::kIdStr = "MusicTimelineGeneratorManager";

// Reconstructed from eboot.elf at 0x566F0. Only a request for the sound
// named like the extension plays; unlike the other music managers, a
// request without an emitter keeps none.
AudioGenerator* MusicTimelineGeneratorManager::Play(const PlayArgs& args) {
    if (args.mName != ResExt()) {
        return nullptr;
    }
    AudioRenderTarget* target = gAudioRenderTargets.Find(args.mRenderTarget, true);
    auto* generator =
        GeneratorPool::Allocate<MusicTimelineGenerator>(*this, target, args.mEmitter);
    if (generator == nullptr) {
        return nullptr;
    }
    generator->Setup(args);
    return generator;
}

// Reconstructed from eboot.elf at 0x57740.
int MusicTimelineGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0x57750.
Symbol MusicTimelineGeneratorManager::GetId() {
    return Id();
}

// Reconstructed from eboot.elf at 0x577F0.
Symbol MusicTimelineGeneratorManager::GetResourceExt() {
    return ResExt();
}

// Reconstructed from eboot.elf at 0x57890.
AudioGenerator* MusicTimelineGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    return GeneratorPool::LockIfOwned(*this, mPool, handle, index);
}

// Reconstructed from eboot.elf at 0x57910.
void MusicTimelineGeneratorManager::SendStopToAllGenerators() {
    GeneratorPool::SendStop(*this, mPool);
}

// Reconstructed from eboot.elf at 0x57980.
void MusicTimelineGeneratorManager::SendKillToAllGenerators() {
    GeneratorPool::SendKill(*this, mPool);
}

// Reconstructed from eboot.elf at 0x579F0.
void MusicTimelineGeneratorManager::GetActiveHandles(void* handles) {
    GeneratorPool::GetActiveHandles(*this, mPool, handles);
}

// Reconstructed from eboot.elf at 0x57B40.
void MusicTimelineGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0x57B50.
void MusicTimelineGeneratorManager::_InitGeneratorPool() {
    GeneratorPool::Init(*this, mPool);
}

// Reconstructed from eboot.elf at 0x57E60.
bool MusicTimelineGeneratorManager::_DeleteGeneratorPool() {
    return GeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x57F10, which jumps to the base
// destructor.
MusicTimelineGeneratorManager::~MusicTimelineGeneratorManager() {}
