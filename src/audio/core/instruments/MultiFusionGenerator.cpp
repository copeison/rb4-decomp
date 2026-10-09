// The map's audio/MultiInstrumentGenerator.o; this build renamed the
// manager to MultiFusionGeneratorManager. The object spans 0x4E780 to
// 0x5277F: the manager, the generator (0x4EC90 to 0x514FF and the inline
// members at 0x51D60 to 0x52190), the inline InstrumentGenerator and
// VirtualInstrument defaults it emits, the resource map's insertion
// (0x524A0) and the static initializer.
#include "audio/core/instruments/MultiFusionGenerator.h"

#include <cmath>

#include "audio/core/fusion/FusionGenerator.h"
#include "audio/core/generators/GeneratorPool.h"
#include "audio/core/system/Audio.h"
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

    AudioEmitter* emitter = args.mEmitter;
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

// Reconstructed from eboot.elf at 0x4EC90. The resource is taken even when
// it is null.
bool MultiFusionGenerator::Setup(const PlayArgs& args, MultiFusionResource* resource) {
    if (!Setup(args)) {
        return false;
    }
    SetResource(ResourcePtr<MultiFusionResource>(resource));
    return true;
}

// Reconstructed from eboot.elf at 0x4EDA0.
bool MultiFusionGenerator::Setup(const PlayArgs& args) {
    mTranspose = 0;
    mTimeStretchAlgorithm = 0;
    mTimeStretchFormantMode = 0;
    if (gAudioBusGeneratorManager == nullptr) {
        return false;
    }
    mBusGenerator = gAudioBusGeneratorManager->_GetGenerator(mRenderTarget, args.mEmitter);
    if (mBusGenerator == nullptr) {
        return false;
    }
    SetSampleRate(static_cast<float>(mRenderTarget->mSampleRate));
    mBusGenerator->Setup(this, args, this);
    mState = static_cast<State>(kStatePlaying + args.mStartPaused);
    return true;
}

// Reconstructed from eboot.elf at 0x4EE40.
void MultiFusionGenerator::Init(AudioGeneratorManager* manager, int index) {
    static_cast<AudioGenerator*>(this)->_InitTypeId();
    mManager = manager;
    mIndex = index;
    for (int channel = 0; channel < kNumChannels; ++channel) {
        mInstruments[channel] = nullptr;
        mNames[channel] = Symbol("");
        mVolumes[channel] = 0.0F;
        mMutes[channel] = false;
    }
    mTranspose = 0;
    mTimeStretchAlgorithm = 0;
    mTimeStretchFormantMode = 0;
    mBusGenerator = nullptr;
    Prepare(static_cast<float>(Audio::GetSamplesPerSecond()), 2, 128, true);
}

// Reconstructed from eboot.elf at 0x4F230. The Fusion generator manager is
// found by its id. A slave channel registers its instrument with channel
// 0's.
void MultiFusionGenerator::SetResource(const ResourcePtr<MultiFusionResource>& resource) {
    ScopedCritSecPtr lock(GetBusLock());
    mResource = resource.Get();
    for (int channel = 0; channel < kNumChannels; ++channel) {
        if (mInstruments[channel] != nullptr) {
            static_cast<AudioGenerator*>(mInstruments[channel])->Release();
            mInstruments[channel] = nullptr;
        }
    }
    if (mResource == nullptr || mResource->Fail()) {
        return;
    }
    auto* manager =
        static_cast<FusionGeneratorManager*>(theSoundManager._GetManager(FusionGeneratorManager::Id()));
    for (int channel = 0; channel < kNumChannels; ++channel) {
        const ResourcePtr<FusionPatchResource> patch(resource->mChannels[channel].mPatch.Get());
        if (patch == nullptr || patch->Fail()) {
            continue;
        }
        FusionGenerator* generator = manager->GetFreeGenerator(mRenderTarget, mEmitter);
        if (generator == nullptr) {
            return;
        }
        generator->SetPatch(patch, 0);
        if (channel != 0 && resource->mSlaveChannels) {
            const InstrumentSlaveType type = resource->mChannels[channel].mSlaveType;
            if (type != 0 && mInstruments[0] != nullptr) {
                mInstruments[0]->AddSlave(generator->mHandle, type);
                generator->AttachedToMaster(mInstruments[0]->mHandle);
            }
        }
        mInstruments[channel] = generator;
    }
}

// Reconstructed from eboot.elf at 0x4F4B0.
void MultiFusionGenerator::AddAudioThreadClient(AudioBusCallable* client) {
    ScopedCritSec lock(mClientListLock);
    mAudioThreadClients.PushBack(*client);
}

// Reconstructed from eboot.elf at 0x4F520.
void MultiFusionGenerator::RemoveAudioThreadClient(AudioBusCallable* client) {
    ScopedCritSec lock(mClientListLock);
    if (client->_mCallbackNode.mList == &mAudioThreadClients) {
        mAudioThreadClients.Remove(*client);
    }
}

// Reconstructed from eboot.elf at 0x4F5B0.
bool MultiFusionGenerator::_PrepareToMakeSamples(
    int numSamples, float sampleRate, int mixCount, int block, bool lastBlock) {
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
    return true;
}

// Reconstructed from eboot.elf at 0x4F790.
MultiFusionGenerator::~MultiFusionGenerator() {}

// Reconstructed from eboot.elf at 0x4FA80.
bool MultiFusionGenerator::Poll() {
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            static_cast<AudioGenerator*>(instrument)->Poll();
        }
    }
    if (mBusGenerator != nullptr && mBusGenerator->Poll()) {
        return true;
    }
    mState = kStateStopped;
    return false;
}

// Reconstructed from eboot.elf at 0x4FCD0.
void MultiFusionGenerator::Pause() {
    mBusGenerator->Pause();
    mState = kStatePaused;
}

// Reconstructed from eboot.elf at 0x4FD30.
void MultiFusionGenerator::Continue() {
    mBusGenerator->Continue();
    mState = kStatePlaying;
}

// Reconstructed from eboot.elf at 0x4FD90.
float MultiFusionGenerator::GetElapsedMs() {
    return mBusGenerator->GetElapsedMs();
}

// Reconstructed from eboot.elf at 0x4FDB0.
float MultiFusionGenerator::GetTimelineMs() {
    return mBusGenerator->GetTimelineMs();
}

// Reconstructed from eboot.elf at 0x4FDD0.
void MultiFusionGenerator::SeekToMs(float ms) {
    mBusGenerator->SeekToMs(ms);
}

// Reconstructed from eboot.elf at 0x4FDF0.
void MultiFusionGenerator::Stop() {
    if (mBusGenerator == nullptr) {
        mState = kStateStopped;
        return;
    }
    mState = kStateStopping;
    mBusGenerator->Stop();
}

// Reconstructed from eboot.elf at 0x4FE50.
void MultiFusionGenerator::Kill() {
    if (mBusGenerator != nullptr) {
        mBusGenerator->KillLocked();
    }
    mState = kStateStopped;
}

// Reconstructed from eboot.elf at 0x4FEB0. The bus voice must exist.
void MultiFusionGenerator::Release() {
    {
        ScopedCritSec lock(mClientListLock);
        if (mAudioThreadClients.mSize != 0) {
            for (unsigned long count = mAudioThreadClients.mSize; count != 0; --count) {
                LinkedListSizeTracked::Node* node = mAudioThreadClients.mNext;
                node->mList = nullptr;
                node->mNext->mPrev = node->mPrev;
                node->mPrev->mNext = node->mNext;
                node->mNext = node;
                node->mPrev = node;
            }
            mAudioThreadClients.mSize = 0;
        }
    }
    {
        ScopedCritSecPtr lock(GetBusLock());
        for (int channel = 0; channel < kNumChannels; ++channel) {
            if (mInstruments[channel] != nullptr) {
                static_cast<AudioGenerator*>(mInstruments[channel])->Release();
                mInstruments[channel] = nullptr;
            }
            mNames[channel] = Symbol("");
            mVolumes[channel] = 0.0F;
            mMutes[channel] = false;
        }
    }
    mBusGenerator->Release();
    mBusGenerator = nullptr;
    mResource = nullptr;
    GeneratorPool::Release(*this);
}

// Reconstructed from eboot.elf at 0x500D0. The instrument must not be null.
void MultiFusionGenerator::SetInstrument(int channel, InstrumentGenerator* instrument) {
    ScopedCritSecPtr lock(GetBusLock());
    if (mInstruments[channel] != nullptr) {
        static_cast<AudioGenerator*>(mInstruments[channel])->Release();
    }
    mInstruments[channel] = instrument;
    instrument->mEmitter = mEmitter;
    instrument->SetMidiChannelVolume(mVolumes[channel], 0.0F, 0);
    instrument->SetMidiChannelMute(mMutes[channel], 0);
    instrument->SetTranspose(mTranspose);
    instrument->SetTimeStretchMode(mTimeStretchAlgorithm, mTimeStretchFormantMode);
    static_cast<AudioGenerator*>(instrument)->SetPlayScale(GetPlayScale());
}

// Reconstructed from eboot.elf at 0x501E0.
void MultiFusionGenerator::ResetInstrumentState() {
    ScopedCritSecPtr lock(GetBusLock());
    for (int channel = 0; channel < kNumChannels; ++channel) {
        mVolumes[channel] = 0.0F;
        mMutes[channel] = false;
        if (mInstruments[channel] != nullptr) {
            mInstruments[channel]->ResetInstrumentState();
        }
    }
}

// Reconstructed from eboot.elf at 0x50260.
void MultiFusionGenerator::ResetMidiState() {
    ScopedCritSecPtr lock(GetBusLock());
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            instrument->ResetMidiState();
        }
    }
}

// Reconstructed from eboot.elf at 0x503C0.
void MultiFusionGenerator::SetExtraPitchBend(float bend, signed char channel) {
    if (channel != -1) {
        InstrumentGenerator* instrument = mInstruments[channel];
        if (instrument != nullptr) {
            instrument->SetExtraPitchBend(bend, channel);
        }
        return;
    }
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            instrument->SetExtraPitchBend(bend, -1);
        }
    }
}

// Reconstructed from eboot.elf at 0x50450.
int MultiFusionGenerator::FindInstrumentChannel(Symbol name) const {
    for (int channel = 0; channel < kNumChannels; ++channel) {
        if (mNames[channel] == name) {
            return channel;
        }
    }
    return -1;
}

// Reconstructed from eboot.elf at 0x50560. The volume is kept even without
// an instrument.
void MultiFusionGenerator::SetMidiChannelVolume(float volumeDb, float fadeSecs, signed char channel) {
    InstrumentGenerator* instrument = mInstruments[channel];
    if (instrument != nullptr) {
        static_cast<AudioGenerator*>(instrument)
            ->SetGain(powf(10.0F, volumeDb * 0.05F), fadeSecs, kPostFadeNone);
    }
    mVolumes[channel] = volumeDb;
}

// Reconstructed from eboot.elf at 0x505F0. A zero gain is kept as -96 dB.
void MultiFusionGenerator::SetMidiChannelGain(float gain, float fadeSecs, signed char channel) {
    InstrumentGenerator* instrument = mInstruments[channel];
    if (instrument != nullptr) {
        static_cast<AudioGenerator*>(instrument)->SetGain(gain, fadeSecs, kPostFadeNone);
    }
    mVolumes[channel] = gain == 0.0F ? -96.0F : log10f(gain) * 20.0F;
}

// Reconstructed from eboot.elf at 0x506A0.
void MultiFusionGenerator::SetMidiChannelMute(bool mute, signed char channel) {
    InstrumentGenerator* instrument = mInstruments[channel];
    if (instrument != nullptr) {
        static_cast<AudioGenerator*>(instrument)->SetMute(mute, false);
    }
    mMutes[channel] = mute;
}

// Reconstructed from eboot.elf at 0x50700.
int MultiFusionGenerator::GetNumVoicesInUse() const {
    int count = 0;
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            count += instrument->GetNumVoicesInUse();
        }
    }
    return count;
}

// Reconstructed from eboot.elf at 0x50750.
void MultiFusionGenerator::KillAllVoices() {
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            instrument->KillAllVoices();
        }
    }
}

// Reconstructed from eboot.elf at 0x508C0.
void MultiFusionGenerator::AllNotesOff() {
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            instrument->AllNotesOff();
        }
    }
}

// Reconstructed from eboot.elf at 0x50A30. The flag is not passed on.
void MultiFusionGenerator::SetSpeed(float speed, bool immediate) {
    static_cast<void>(immediate);
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            static_cast<AudioGenerator*>(instrument)->SetSpeed(speed, false);
        }
    }
}

// Reconstructed from eboot.elf at 0x50AF0.
float MultiFusionGenerator::GetSpeed(bool* changing) {
    if (changing != nullptr) {
        *changing = false;
    }
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            return static_cast<AudioGenerator*>(instrument)->GetSpeed(nullptr);
        }
    }
    return 1.0F;
}

// Reconstructed from eboot.elf at 0x50CF0.
void MultiFusionGenerator::SetPlayScale(float scale) {
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            static_cast<AudioGenerator*>(instrument)->SetPlayScale(scale);
        }
    }
}

// Reconstructed from eboot.elf at 0x50DB0.
float MultiFusionGenerator::GetPlayScale() {
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            return static_cast<AudioGenerator*>(instrument)->GetPlayScale();
        }
    }
    return 1.0F;
}

// Reconstructed from eboot.elf at 0x50FA0.
void MultiFusionGenerator::SetTranspose(int semitones) {
    mTranspose = semitones;
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            instrument->SetTranspose(semitones);
        }
    }
}

// Reconstructed from eboot.elf at 0x50FF0.
void MultiFusionGenerator::SetTimeStretchMode(int algorithm, int formantMode) {
    mTimeStretchAlgorithm = algorithm;
    mTimeStretchFormantMode = formantMode;
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            instrument->SetTimeStretchMode(algorithm, formantMode);
        }
    }
}

// Reconstructed from eboot.elf at 0x51050.
void MultiFusionGenerator::SetTempo(float tempo) {
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            instrument->SetTempo(tempo);
        }
    }
}

// Reconstructed from eboot.elf at 0x510A0.
bool MultiFusionGenerator::SetParameter(Symbol name, float value) {
    bool taken = mBusGenerator != nullptr && mBusGenerator->SetParameter(name, value);
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr) {
            taken = static_cast<AudioGenerator*>(instrument)->SetParameter(name, value) | taken;
        }
    }
    return taken;
}

// Reconstructed from eboot.elf at 0x511C0.
bool MultiFusionGenerator::GetParameter(Symbol name, float& value) {
    if (mBusGenerator != nullptr && mBusGenerator->GetParameter(name, value)) {
        return true;
    }
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument != nullptr && static_cast<AudioGenerator*>(instrument)->GetParameter(name, value)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x512C0. Slaves are mixed by their
// master.
bool MultiFusionGenerator::Process(AudioBuffer<float>& buffer) {
    buffer.Clear();
    ScopedCritSecPtr lock(GetBusLock());
    for (InstrumentGenerator* instrument : mInstruments) {
        if (instrument == nullptr || instrument->IsAttachedToMaster()) {
            continue;
        }
        instrument->Process(mBuffer);
        if (mBuffer.mCleared) {
            continue;
        }
        buffer.Accumulate(mBuffer.mChannelData, buffer.mNumChannels, buffer.mNumFrames);
        buffer.mCleared = false;
    }
    return true;
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

// Reconstructed from eboot.elf at 0x516E0. The generator's own Stop entry
// (primary slot 57) is called.
void MultiFusionGeneratorManager::SendStopToAllGenerators() {
    GeneratorPool::SendStop(*this, mPool);
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
// constructor and calls its own Init entry (primary slot 54).
void MultiFusionGeneratorManager::_InitGeneratorPool() {
    GeneratorPool::Init(*this, mPool);
}

// Reconstructed from eboot.elf at 0x51C80.
bool MultiFusionGeneratorManager::_DeleteGeneratorPool() {
    return GeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x51D30, which jumps to the base
// destructor.
MultiFusionGeneratorManager::~MultiFusionGeneratorManager() {}
