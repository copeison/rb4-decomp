// audio/SynthRackGenerator.o, newer than the reference map. The object spans
// 0x58130 to 0x5B29E. Apart from Setup, the generator class is not
// reconstructed; its members span 0x58420 to 0x5A58F and 0x5ADB0 to
// 0x5B27F.
#include "audio/core/instruments/SynthRackGenerator.h"

#include <cstring>

#include "audio/core/generators/GeneratorPool.h"
#include "audio/core/output/AudioRenderTarget.h"
#include "audio/core/system/SoundManager.h"

// The object's statics. Its static initializer is at 0x5B280.
const char* SynthRackGeneratorManager::kIdStr = "SynthRackGeneratorManager";
Symbol SynthRackGenerator::kTypeId;

// Reconstructed from eboot.elf at 0x58130.
void SynthRackGeneratorManager::Init() {
    AudioGeneratorManager::Init();
}

// Reconstructed from eboot.elf at 0x58140. The generator's Setup is inlined.
// A failed setup returns the voice to the pool.
AudioGenerator* SynthRackGeneratorManager::Play(const PlayArgs& args) {
    if (std::strcmp(args.mName.Str(), ".synth_rack") != 0) {
        return nullptr;
    }
    AudioEmitterCom* emitter = args.mEmitter;
    if (emitter == nullptr) {
        emitter = theSoundManager.GetDefault2DEmitter();
    }
    AudioRenderTarget* target = gAudioRenderTargets.Find(args.mRenderTarget, true);
    SynthRackGenerator* generator =
        GeneratorPool::Allocate<SynthRackGenerator>(*this, target, emitter);
    if (generator == nullptr) {
        return nullptr;
    }
    if (generator->Setup(args)) {
        return generator;
    }
    GeneratorPool::Release(*generator);
    return nullptr;
}

// Reconstructed from eboot.elf at 0x58370.
bool SynthRackGenerator::Setup(const PlayArgs& args) {
    mTranspose = 0;
    mBeat = 0.0F;
    mTimeStretchAlgorithm = 0;
    mTimeStretchFormantMode = 0;
    if (gAudioBusGeneratorManager == nullptr) {
        return false;
    }
    AudioRenderTarget* target = gAudioRenderTargets.Find(args.mRenderTarget, true);
    mBusGenerator = gAudioBusGeneratorManager->_GetGenerator(target, args.mEmitter);
    if (mBusGenerator == nullptr) {
        return false;
    }
    SetSampleRate(static_cast<float>(target->mSampleRate));
    mBusGenerator->Setup(this, args, this);
    mState = static_cast<State>(kStatePlaying + args.mStartPaused);
    return true;
}

// Reconstructed from eboot.elf at 0x5A590.
int SynthRackGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0x5A5A0.
Symbol SynthRackGeneratorManager::GetId() {
    return Id();
}

// Reconstructed from eboot.elf at 0x5A640.
Symbol SynthRackGeneratorManager::GetResourceExt() {
    return ResExt();
}

// Reconstructed from eboot.elf at 0x5A6E0.
AudioGenerator* SynthRackGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    return GeneratorPool::LockIfOwned(*this, mPool, handle, index);
}

// Reconstructed from eboot.elf at 0x5A770. The binary calls the generator's
// own Stop entry (primary slot 57); the declaration does not reproduce that
// slot, so the call goes through AudioGenerator.
void SynthRackGeneratorManager::SendStopToAllGenerators() {
    ScopedCritSecPtr tracker(&mCritSec);
    for (int index = 0; index < mPoolSize; ++index) {
        static_cast<AudioGenerator&>(mPool[index]).Stop();
    }
}

// Reconstructed from eboot.elf at 0x5A7E0.
void SynthRackGeneratorManager::SendKillToAllGenerators() {
    GeneratorPool::SendKill(*this, mPool);
}

// Reconstructed from eboot.elf at 0x5A860.
void SynthRackGeneratorManager::GetActiveHandles(void* handles) {
    GeneratorPool::GetActiveHandles(*this, mPool, handles);
}

// Reconstructed from eboot.elf at 0x5A9B0.
void SynthRackGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0x5A9C0, which inlines the generator's
// constructor. The binary calls the generator's own Init entry (primary
// slot 54); the declaration does not reproduce that slot, so the call goes
// through AudioGenerator.
void SynthRackGeneratorManager::_InitGeneratorPool() {
    mPool = new SynthRackGenerator[mPoolSize];
    for (int index = 0; index < mPoolSize; ++index) {
        static_cast<AudioGenerator&>(mPool[index]).Init(this, index);
        mFreeList.PushBack(mPool[index]);
    }
}

// Reconstructed from eboot.elf at 0x5ACD0.
bool SynthRackGeneratorManager::_DeleteGeneratorPool() {
    return GeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x5AD80, which jumps to the base
// destructor.
SynthRackGeneratorManager::~SynthRackGeneratorManager() {}
