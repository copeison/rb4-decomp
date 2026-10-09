#include "audio/core/fusion/FusionGenerator.h"

#include "audio/core/generators/GeneratorPool.h"
#include "audio/core/output/AudioRenderTarget.h"
#include "audio/core/system/Audio.h"
#include "audio/fmod/playback/FmodAudioBusGenerator.h"

// The statics at 0x19C84B0 through 0x19C8508, which the static initializer
// at 0x44290 sets up in this order.
CritSec FusionGeneratorManager::sPatchMapCritSec;
eastl::map<Symbol, FusionPatchResource*> FusionGeneratorManager::sPatchMap;
CritSec FusionGenerator::mClientListLock;
Symbol FusionGenerator::kTypeId;
const char* FusionGeneratorManager::kIdStr = "FusionGeneratorManager";

namespace {

// Unlinks every client, as the destructor and Release inline the list's
// clear. Name not in the reference map.
void UnlinkAll(LinkedListSizeTracked::ListBase& list) {
    if (list.mSize != 0) {
        for (unsigned long count = list.mSize; count != 0; --count) {
            LinkedListSizeTracked::Node* const node = list.mNext;
            node->mList = nullptr;
            node->mNext->mPrev = node->mPrev;
            node->mPrev->mNext = node->mNext;
            node->mNext = node;
            node->mPrev = node;
        }
        list.mSize = 0;
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x41F90.
void FusionGeneratorManager::AddPatch(Symbol name, FusionPatchResource* patch) {
    ScopedCritSec lock(sPatchMapCritSec);
    sPatchMap[name] = patch;
}

// Reconstructed from eboot.elf at 0x42090.
void FusionGeneratorManager::GetPatchNames(eastl::vector<Symbol>& names) {
    ScopedCritSec lock(sPatchMapCritSec);
    for (const auto& entry : sPatchMap) {
        names.push_back(entry.first);
    }
}

// Reconstructed from eboot.elf at 0x421F0. A patch registered under several
// names loses every registration.
bool FusionGeneratorManager::RemovePatch(FusionPatchResource* patch) {
    ScopedCritSec lock(sPatchMapCritSec);
    bool removed = false;
    auto it = sPatchMap.begin();
    while (it != sPatchMap.end()) {
        auto next = it;
        ++next;
        if (it->second == patch) {
            sPatchMap.erase(it);
            removed = true;
        }
        it = next;
    }
    return removed;
}

// Reconstructed from eboot.elf at 0x422C0. The voice pool comes from the
// target passed in, which must not be null.
FusionGenerator* FusionGeneratorManager::GetFreeGenerator(
    AudioRenderTarget* target, AudioEmitter* emitter) {
    FusionGenerator* const generator =
        GeneratorPool::Allocate<FusionGenerator>(*this, target, emitter);
    if (generator != nullptr) {
        generator->SetVoicePool(target->GetVoicePool());
        generator->ResetInstrumentState();
    }
    return generator;
}

// Reconstructed from eboot.elf at 0x423A0. Only registered patches play. A
// slave request plays without a bus generator of its own.
AudioGenerator* FusionGeneratorManager::Play(const PlayArgs& args) {
    ResourcePtr<FusionPatchResource> patch;
    {
        ScopedCritSec lock(sPatchMapCritSec);
        const auto it = sPatchMap.find(args.mName);
        if (it == sPatchMap.end()) {
            return nullptr;
        }
        patch = it->second;
    }
    AudioEmitter* const emitter =
        args.mEmitter != nullptr ? args.mEmitter : theSoundManager.GetDefault2DEmitter();
    AudioRenderTarget* const target = gAudioRenderTargets.Find(args.mRenderTarget, true);
    FusionGenerator* const generator =
        GeneratorPool::Allocate<FusionGenerator>(*this, target, emitter);
    if (generator == nullptr) {
        return nullptr;
    }
    const bool createBus = args.mFormat != FusionPlayArgs::kFusionFormat ||
                           static_cast<const FusionPlayArgs&>(args).mSlaveType == 0;
    if (!generator->Setup(patch, args, createBus)) {
        generator->Release();
        return nullptr;
    }
    return generator;
}

// Reconstructed from eboot.elf at 0x42600. A slave's master must still be a
// live instrument: the binary does not check the lock before AddSlave.
bool FusionGenerator::Setup(
    const ResourcePtr<FusionPatchResource>& patch, const PlayArgs& args, bool createBus) {
    ResetInstrumentState();
    SetSampleRate(static_cast<float>(mRenderTarget->mSampleRate));
    SetVoicePool(mRenderTarget->GetVoicePool());
    LoadPatch(patch);
    if (args.mFormat == FusionPlayArgs::kFusionFormat) {
        const FusionPlayArgs& fusionArgs = static_cast<const FusionPlayArgs&>(args);
        if (fusionArgs.mSlaveType != 0) {
            InstrumentHandleLock master(fusionArgs.mMasterHandle);
            master->AddSlave(mHandle, fusionArgs.mSlaveType);
            mMasterHandle = fusionArgs.mMasterHandle;
        }
    }
    if (createBus) {
        if (gAudioBusGeneratorManager == nullptr) {
            return false;
        }
        mBusGenerator = gAudioBusGeneratorManager->_GetGenerator(mRenderTarget, args.mEmitter);
        if (mBusGenerator == nullptr) {
            return false;
        }
        mBusGenerator->Setup(this, args, nullptr);
    }
    mState = args.mStartPaused ? kStatePaused : kStatePlaying;
    return true;
}

// Reconstructed from eboot.elf at 0x42740. The bus is prepared at the
// engine's rate for stereo 128-sample blocks without allocating its buffer.
void FusionGenerator::Init(AudioGeneratorManager* manager, int index) {
    static_cast<AudioGenerator*>(this)->_InitTypeId();
    mManager = manager;
    mIndex = index;
    mMasterHandle = 0;
    mBusGenerator = nullptr;
    Prepare(static_cast<float>(Audio::GetSamplesPerSecond()), 2, 128, false);
}

// Reconstructed from eboot.elf at 0x42840.
void FusionGenerator::AddAudioThreadClient(AudioBusCallable* client) {
    ScopedCritSec lock(mClientListLock);
    mAudioThreadClients.PushBack(*client);
}

// Reconstructed from eboot.elf at 0x428B0.
void FusionGenerator::RemoveAudioThreadClient(AudioBusCallable* client) {
    ScopedCritSec lock(mClientListLock);
    if (client->_mCallbackNode.mList == &mAudioThreadClients) {
        mAudioThreadClients.Remove(*client);
    }
}

// Reconstructed from eboot.elf at 0x42930.
void FusionGenerator::LoadAndSetPatch(const char* path) {
    ResourcePtr<FusionPatchResource> patch =
        Resource::GetOrLoad<FusionPatchResource>(ResourcePath(path), false);
    if (patch != nullptr && !patch->Fail()) {
        LoadPatch(patch);
    }
}

// Reconstructed from eboot.elf at 0x429C0.
void FusionGenerator::CallPreProcessCallbacks(
    int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) {
    if (mAudioThreadClients.mSize == 0) {
        return;
    }
    ScopedCritSec lock(mClientListLock);
    LinkedListSizeTracked::Node* node = mAudioThreadClients.mNext;
    while (node != mAudioThreadClients.Sentinel()) {
        AudioBusCallable* const client = ClientList::Owner(node);
        const bool keep =
            client->_PrepareToMakeSamples(numSamples, sampleRate, mixCount, block, lastBlock);
        LinkedListSizeTracked::Node* const next = node->mNext;
        if (!keep) {
            mAudioThreadClients.ListBase::Remove(*node);
        }
        node = next;
    }
}

// Reconstructed from eboot.elf at 0x42AC0.
FusionGenerator::~FusionGenerator() {
    if (mBusGenerator != nullptr) {
        mBusGenerator->Release();
    }
}

// Reconstructed from eboot.elf at 0x42DB0.
bool FusionGenerator::Poll() {
    if (mState == kStateStopped) {
        return false;
    }
    if (mBusGenerator != nullptr && !mBusGenerator->Poll()) {
        mState = kStateStopped;
        return false;
    }
    return mState != kStateStopped;
}

// Reconstructed from eboot.elf at 0x42E50.
void FusionGenerator::Pause() {
    if (mBusGenerator != nullptr) {
        mBusGenerator->Pause();
    }
    mState = kStatePaused;
}

// Reconstructed from eboot.elf at 0x42EB0.
void FusionGenerator::Continue() {
    if (mBusGenerator != nullptr) {
        mBusGenerator->Continue();
    }
    mState = kStatePlaying;
}

// Reconstructed from eboot.elf at 0x42F10.
float FusionGenerator::GetElapsedMs() {
    return 0.0F;
}

// Reconstructed from eboot.elf at 0x42F30.
float FusionGenerator::GetTimelineMs() {
    return 0.0F;
}

// Reconstructed from eboot.elf at 0x42F50.
void FusionGenerator::SeekToMs(float) {}

// Reconstructed from eboot.elf at 0x42F70.
bool FusionGenerator::SetParameter(Symbol name, float value) {
    return mBusGenerator != nullptr ? mBusGenerator->SetParameter(name, value) : false;
}

// Reconstructed from eboot.elf at 0x42FB0.
bool FusionGenerator::GetParameter(Symbol name, float& value) {
    return mBusGenerator != nullptr ? mBusGenerator->GetParameter(name, value) : false;
}

// Reconstructed from eboot.elf at 0x42FF0. The bus generator finishes the
// stop; a slave stops at once.
void FusionGenerator::Stop() {
    KillAllVoices();
    if (mBusGenerator != nullptr) {
        mBusGenerator->Stop();
        mState = kStateStopping;
        return;
    }
    if (mMasterHandle != 0) {
        InstrumentHandleLock master(mMasterHandle);
        if (master.mInstrument != nullptr) {
            master->RemoveSlave(mHandle);
        }
        mMasterHandle = 0;
    }
    mState = kStateStopped;
}

// Reconstructed from eboot.elf at 0x43170.
void FusionGenerator::Kill() {
    KillAllVoices();
    if (mBusGenerator != nullptr) {
        mBusGenerator->KillLocked();
    } else if (mMasterHandle != 0) {
        InstrumentHandleLock master(mMasterHandle);
        if (master.mInstrument != nullptr) {
            master->RemoveSlave(mHandle);
        }
        mMasterHandle = 0;
    }
    mState = kStateStopped;
}

// Reconstructed from eboot.elf at 0x432E0.
void FusionGenerator::Release() {
    if (mBusGenerator != nullptr) {
        mBusGenerator->Release();
        mBusGenerator = nullptr;
    }
    KillAllVoices();
    {
        ScopedCritSec lock(mClientListLock);
        UnlinkAll(mAudioThreadClients);
    }
    SetPatch(ResourcePtr<FusionPatchResource>(), 0);
    _DumpAllInstrumentSlaves();
    GeneratorPool::Release(*this);
}

// Reconstructed from eboot.elf at 0x43480.
int FusionGeneratorManager::GetIndex() {
    return mManagerIndex;
}

// Reconstructed from eboot.elf at 0x43490.
Symbol FusionGeneratorManager::GetId() {
    return Id();
}

// Reconstructed from eboot.elf at 0x43530.
Symbol FusionGeneratorManager::GetResourceExt() {
    return ResExt();
}

// Reconstructed from eboot.elf at 0x435D0.
AudioGenerator* FusionGeneratorManager::LockIfOwned(unsigned int handle, int index) {
    return GeneratorPool::LockIfOwned(*this, mPool, handle, index);
}

// Reconstructed from eboot.elf at 0x43660.
void FusionGeneratorManager::SendStopToAllGenerators() {
    GeneratorPool::SendStop(*this, mPool);
}

// Reconstructed from eboot.elf at 0x436D0.
void FusionGeneratorManager::SendKillToAllGenerators() {
    GeneratorPool::SendKill(*this, mPool);
}

// Reconstructed from eboot.elf at 0x43750.
void FusionGeneratorManager::GetActiveHandles(void* handles) {
    GeneratorPool::GetActiveHandles(*this, mPool, handles);
}

// Reconstructed from eboot.elf at 0x438A0.
void FusionGeneratorManager::_SetManagerIndex(int index) {
    mManagerIndex = index;
}

// Reconstructed from eboot.elf at 0x438B0.
void FusionGeneratorManager::_InitGeneratorPool() {
    GeneratorPool::Init(*this, mPool);
}

// Reconstructed from eboot.elf at 0x43A10.
bool FusionGeneratorManager::_DeleteGeneratorPool() {
    return GeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x43B70, which jumps to the base
// destructor.
FusionGeneratorManager::~FusionGeneratorManager() {}

// Reconstructed from eboot.elf at 0x43E20.
Symbol FusionGenerator::GetTypeId() {
    return kTypeId;
}

// Reconstructed from eboot.elf at 0x43F00.
void* FusionGenerator::GetPluginData(const char* name) {
    return mBusGenerator != nullptr ? mBusGenerator->GetPluginData(name) : nullptr;
}
