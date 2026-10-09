// audio/SynthRackGenerator.o, newer than the reference map. The object spans
// 0x58130 to 0x5B29E: the manager, the generator (0x58370 to 0x5A58F and
// the inline members at 0x5ADB0 to 0x5B0DF), the rack vector's growth at
// 0x5B0E0 and the static initializer.
#include "audio/core/instruments/SynthRackGenerator.h"

#include <cmath>
#include <cstring>

#include "audio/core/generators/GeneratorPool.h"
#include "audio/core/output/AudioRenderTarget.h"
#include "audio/core/system/Audio.h"
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
    AudioEmitter* emitter = args.mEmitter;
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

// Reconstructed from eboot.elf at 0x58420.
void SynthRackGenerator::Init(AudioGeneratorManager* manager, int index) {
    static_cast<AudioGenerator*>(this)->_InitTypeId();
    mManager = manager;
    mIndex = index;
    mTranspose = 0;
    mTimeStretchAlgorithm = 0;
    mTimeStretchFormantMode = 0;
    mBusGenerator = nullptr;
    Prepare(static_cast<float>(Audio::GetSamplesPerSecond()), 2, 128, true);
}

// Reconstructed from eboot.elf at 0x58520.
void SynthRackGenerator::AddAudioThreadClient(AudioBusCallable* client) {
    ScopedCritSec lock(mClientListLock);
    mAudioThreadClients.PushBack(*client);
}

// Reconstructed from eboot.elf at 0x58590.
void SynthRackGenerator::RemoveAudioThreadClient(AudioBusCallable* client) {
    ScopedCritSec lock(mClientListLock);
    if (client->_mCallbackNode.mList == &mAudioThreadClients) {
        mAudioThreadClients.Remove(*client);
    }
}

// Reconstructed from eboot.elf at 0x58620.
bool SynthRackGenerator::_PrepareToMakeSamples(
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

// Reconstructed from eboot.elf at 0x587F0.
SynthRackGenerator::~SynthRackGenerator() {}

// Reconstructed from eboot.elf at 0x58AE0.
bool SynthRackGenerator::Poll() {
    for (RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr) {
            static_cast<AudioGenerator*>(slot.mInstrument)->Poll();
        }
    }
    if (mBusGenerator != nullptr) {
        if (mBusGenerator->Poll()) {
            return true;
        }
        mBusGenerator->Release();
        mBusGenerator = nullptr;
    }
    mState = kStateStopped;
    return false;
}

// Reconstructed from eboot.elf at 0x58C20.
void SynthRackGenerator::Pause() {
    mBusGenerator->Pause();
    mState = kStatePaused;
}

// Reconstructed from eboot.elf at 0x58C80.
void SynthRackGenerator::Continue() {
    mBusGenerator->Continue();
    mState = kStatePlaying;
}

// Reconstructed from eboot.elf at 0x58CE0.
float SynthRackGenerator::GetElapsedMs() {
    return mBusGenerator->GetElapsedMs();
}

// Reconstructed from eboot.elf at 0x58D00.
float SynthRackGenerator::GetTimelineMs() {
    return mBusGenerator->GetTimelineMs();
}

// Reconstructed from eboot.elf at 0x58D20.
void SynthRackGenerator::SeekToMs(float ms) {
    mBusGenerator->SeekToMs(ms);
}

// Reconstructed from eboot.elf at 0x58D40.
void SynthRackGenerator::Stop() {
    if (mBusGenerator == nullptr) {
        mState = kStateStopped;
        return;
    }
    mState = kStateStopping;
    mBusGenerator->Stop();
}

// Reconstructed from eboot.elf at 0x58DA0.
void SynthRackGenerator::Kill() {
    if (mBusGenerator != nullptr) {
        mBusGenerator->KillLocked();
        mBusGenerator->Release();
        mBusGenerator = nullptr;
    }
    mState = kStateStopped;
}

// Reconstructed from eboot.elf at 0x58E40.
void SynthRackGenerator::Release() {
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
        for (RackSlot& slot : mInstruments) {
            if (slot.mInstrument != nullptr) {
                static_cast<AudioGenerator*>(slot.mInstrument)->Release();
            }
        }
        mInstruments.clear();
    }
    GeneratorPool::Release(*this);
}

// Reconstructed from eboot.elf at 0x58FF0. The instrument must not be null.
void SynthRackGenerator::SetInstrument(int index, InstrumentGenerator* instrument) {
    ScopedCritSecPtr lock(GetBusLock());
    if (static_cast<int>(mInstruments.size()) <= index) {
        mInstruments.resize(index + 1);
    }
    RackSlot& slot = mInstruments[index];
    if (slot.mInstrument != nullptr) {
        static_cast<AudioGenerator*>(slot.mInstrument)->Release();
    }
    slot.mInstrument = instrument;
    instrument->mEmitter = mEmitter;
    instrument->SetMidiChannelVolume(slot.mVolume, 0.0F, 0);
    instrument->SetMidiChannelMute(slot.mMute, 0);
    instrument->SetTranspose(mTranspose);
    instrument->SetTimeStretchMode(mTimeStretchAlgorithm, mTimeStretchFormantMode);
    static_cast<AudioGenerator*>(instrument)->SetPlayScale(GetPlayScale());
}

// Reconstructed from eboot.elf at 0x59170.
void SynthRackGenerator::ResetInstrumentState() {
    ScopedCritSecPtr lock(GetBusLock());
    for (RackSlot& slot : mInstruments) {
        slot.mVolume = 0.0F;
        slot.mMute = false;
        if (slot.mInstrument != nullptr) {
            slot.mInstrument->ResetInstrumentState();
        }
    }
}

// Reconstructed from eboot.elf at 0x59210.
void SynthRackGenerator::ResetMidiState() {
    ScopedCritSecPtr lock(GetBusLock());
    for (RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr) {
            slot.mInstrument->ResetMidiState();
        }
    }
}

// Reconstructed from eboot.elf at 0x592A0.
void SynthRackGenerator::SetInstrumentMute(int index, bool mute) {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return;
    }
    RackSlot& slot = mInstruments[index];
    if (slot.mInstrument == nullptr) {
        return;
    }
    slot.mMute = mute;
    static_cast<AudioGenerator*>(slot.mInstrument)->SetMute(mute, false);
}

// Reconstructed from eboot.elf at 0x59300.
bool SynthRackGenerator::GetInstrumentMute(int index) const {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return false;
    }
    const RackSlot& slot = mInstruments[index];
    if (slot.mInstrument == nullptr) {
        return false;
    }
    return slot.mMute;
}

// Reconstructed from eboot.elf at 0x59350.
void SynthRackGenerator::SetInstrumentVolume(int index, float volumeDb, float fadeSecs) {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument == nullptr) {
        return;
    }
    static_cast<AudioGenerator*>(instrument)
        ->SetGain(powf(10.0F, volumeDb * 0.05F), fadeSecs, kPostFadeNone);
}

// Reconstructed from eboot.elf at 0x593E0.
float SynthRackGenerator::GetInstrumentVolume(int index) const {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return 0.0F;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument == nullptr) {
        return 0.0F;
    }
    const float gain = static_cast<AudioGenerator*>(instrument)->GetGain();
    if (gain == 0.0F) {
        return -96.0F;
    }
    return log10f(gain) * 20.0F;
}

// Reconstructed from eboot.elf at 0x59460.
void SynthRackGenerator::SetInstrumentGain(int index, float gain, float fadeSecs) {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument == nullptr) {
        return;
    }
    static_cast<AudioGenerator*>(instrument)->SetGain(gain, fadeSecs, kPostFadeNone);
}

// Reconstructed from eboot.elf at 0x594B0.
float SynthRackGenerator::GetInstrumentGain(int index) const {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return 1.0F;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument == nullptr) {
        return 1.0F;
    }
    return static_cast<AudioGenerator*>(instrument)->GetGain();
}

// Reconstructed from eboot.elf at 0x59500.
void SynthRackGenerator::NoteOn(
    int index, signed char note, signed char velocity, signed char channel, float startOffsetMs) {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument != nullptr) {
        instrument->NoteOn(note, velocity, channel, startOffsetMs);
    }
}

// Reconstructed from eboot.elf at 0x59550.
void SynthRackGenerator::NoteOff(int index, signed char note, signed char channel) {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument != nullptr) {
        instrument->NoteOff(note, channel);
    }
}

// Reconstructed from eboot.elf at 0x595A0.
void SynthRackGenerator::SetPitchBend(int index, float bend, signed char channel) {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument != nullptr) {
        instrument->SetPitchBend(bend, channel);
    }
}

// Reconstructed from eboot.elf at 0x595F0.
float SynthRackGenerator::GetPitchBend(int index, signed char channel) const {
    if (index < 0 || index >= static_cast<int>(mInstruments.size()) ||
        mInstruments[index].mInstrument == nullptr) {
        return 0.0F;
    }
    return mInstruments[channel].mInstrument->GetPitchBend(channel);
}

// Reconstructed from eboot.elf at 0x59650.
void SynthRackGenerator::SetController(
    int index, ControllerID controller, signed char msb, signed char lsb, signed char channel) {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument != nullptr) {
        instrument->SetController(controller, msb, lsb, channel);
    }
}

// Reconstructed from eboot.elf at 0x596A0.
void SynthRackGenerator::GetController(
    int index, ControllerID controller, signed char& msb, signed char& lsb, signed char channel) const {
    if (index < 0 || index >= static_cast<int>(mInstruments.size()) ||
        mInstruments[index].mInstrument == nullptr) {
        lsb = 0;
        msb = 0;
        return;
    }
    mInstruments[channel].mInstrument->GetController(controller, msb, lsb, channel);
}

// Reconstructed from eboot.elf at 0x59710.
void SynthRackGenerator::SetInstrumentName(int index, Symbol name) {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return;
    }
    RackSlot& slot = mInstruments[index];
    if (slot.mInstrument != nullptr) {
        slot.mName = name;
    }
}

// Reconstructed from eboot.elf at 0x59750.
Symbol SynthRackGenerator::GetInstrumentName(int index) const {
    if (index < 0 || index >= static_cast<int>(mInstruments.size()) ||
        mInstruments[index].mInstrument == nullptr) {
        return Symbol("");
    }
    return mInstruments[index].mName;
}

// Reconstructed from eboot.elf at 0x597D0.
int SynthRackGenerator::FindInstrumentChannel(Symbol name) const {
    const int count = static_cast<int>(mInstruments.size());
    for (int index = 0; index < count; ++index) {
        if (mInstruments[index].mName == name) {
            return index;
        }
    }
    return -1;
}

// Reconstructed from eboot.elf at 0x59830. FindInstrumentChannel is inlined.
InstrumentGenerator* SynthRackGenerator::FindInstrument(Symbol name) const {
    const int index = FindInstrumentChannel(name);
    if (index == -1) {
        return nullptr;
    }
    return mInstruments[index].mInstrument;
}

// Reconstructed from eboot.elf at 0x598A0.
InstrumentGenerator* SynthRackGenerator::GetInstrument(int index) const {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return nullptr;
    }
    return mInstruments[index].mInstrument;
}

// Reconstructed from eboot.elf at 0x598E0.
void SynthRackGenerator::SetPatch(int index, const ResourcePtr<FusionPatchResource>& patch, int channel) {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument != nullptr) {
        instrument->SetPatch(patch, channel);
    }
}

// Reconstructed from eboot.elf at 0x59930.
void SynthRackGenerator::SetMidiChannelVolume(int index, float volume, float fadeSecs, signed char channel) {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument != nullptr) {
        instrument->SetMidiChannelVolume(volume, fadeSecs, channel);
    }
}

// Reconstructed from eboot.elf at 0x59980.
float SynthRackGenerator::GetMidiChannelVolume(int index, signed char channel) const {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return 0.0F;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument == nullptr) {
        return 0.0F;
    }
    return instrument->GetMidiChannelVolume(channel);
}

// Reconstructed from eboot.elf at 0x599D0.
void SynthRackGenerator::SetMidiChannelGain(int index, float gain, float fadeSecs) {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument != nullptr) {
        instrument->SetMidiChannelGain(gain, fadeSecs, 0);
    }
}

// Reconstructed from eboot.elf at 0x59A20.
float SynthRackGenerator::GetMidiChannelGain(int index) const {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return 1.0F;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument == nullptr) {
        return 1.0F;
    }
    return instrument->GetMidiChannelGain(0);
}

// Reconstructed from eboot.elf at 0x59A70.
void SynthRackGenerator::SetMidiChannelMute(int index, bool mute, signed char channel) {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument != nullptr) {
        instrument->SetMidiChannelMute(mute, channel);
    }
}

// Reconstructed from eboot.elf at 0x59AC0.
bool SynthRackGenerator::GetMidiChannelMute(int index, signed char channel) const {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return false;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument == nullptr) {
        return false;
    }
    return instrument->GetMidiChannelMute(channel);
}

// Reconstructed from eboot.elf at 0x59B10.
void SynthRackGenerator::HandleMidiMessage(
    int index, signed char status, signed char data1, signed char data2, float startOffsetMs) {
    if (index < 0 || index >= static_cast<int>(mInstruments.size())) {
        return;
    }
    InstrumentGenerator* instrument = mInstruments[index].mInstrument;
    if (instrument != nullptr) {
        instrument->HandleMidiMessage(status, data1, data2, startOffsetMs);
    }
}

// Reconstructed from eboot.elf at 0x59B60.
int SynthRackGenerator::GetNumVoicesInUse() const {
    int count = 0;
    for (const RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr) {
            count += slot.mInstrument->GetNumVoicesInUse();
        }
    }
    return count;
}

// Reconstructed from eboot.elf at 0x59BD0.
void SynthRackGenerator::KillAllVoices() {
    for (RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr) {
            slot.mInstrument->KillAllVoices();
        }
    }
}

// Reconstructed from eboot.elf at 0x59C20.
void SynthRackGenerator::AllNotesOff() {
    for (RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr) {
            slot.mInstrument->AllNotesOff();
        }
    }
}

// Reconstructed from eboot.elf at 0x59C70. The flag is not passed on.
void SynthRackGenerator::SetSpeed(float speed, bool immediate) {
    static_cast<void>(immediate);
    for (RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr) {
            static_cast<AudioGenerator*>(slot.mInstrument)->SetSpeed(speed, false);
        }
    }
}

// Reconstructed from eboot.elf at 0x59D50.
float SynthRackGenerator::GetSpeed(bool* changing) {
    if (changing != nullptr) {
        *changing = false;
    }
    for (RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr) {
            return static_cast<AudioGenerator*>(slot.mInstrument)->GetSpeed(nullptr);
        }
    }
    return 1.0F;
}

// Reconstructed from eboot.elf at 0x59DF0.
void SynthRackGenerator::SetPlayScale(float scale) {
    for (RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr) {
            static_cast<AudioGenerator*>(slot.mInstrument)->SetPlayScale(scale);
        }
    }
}

// Reconstructed from eboot.elf at 0x59ED0.
float SynthRackGenerator::GetPlayScale() {
    for (RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr) {
            return static_cast<AudioGenerator*>(slot.mInstrument)->GetPlayScale();
        }
    }
    return 1.0F;
}

// Reconstructed from eboot.elf at 0x59F60.
void SynthRackGenerator::SetTranspose(int semitones) {
    mTranspose = semitones;
    for (RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr) {
            slot.mInstrument->SetTranspose(semitones);
        }
    }
}

// Reconstructed from eboot.elf at 0x59FD0.
void SynthRackGenerator::SetTimeStretchMode(int algorithm, int formantMode) {
    mTimeStretchAlgorithm = algorithm;
    mTimeStretchFormantMode = formantMode;
    for (RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr) {
            slot.mInstrument->SetTimeStretchMode(algorithm, formantMode);
        }
    }
}

// Reconstructed from eboot.elf at 0x5A050.
void SynthRackGenerator::SetTempo(float tempo) {
    for (RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr) {
            slot.mInstrument->SetTempo(tempo);
        }
    }
}

// Reconstructed from eboot.elf at 0x5A0C0.
bool SynthRackGenerator::SetParameter(Symbol name, float value) {
    bool taken = mBusGenerator != nullptr && mBusGenerator->SetParameter(name, value);
    for (RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr) {
            taken = static_cast<AudioGenerator*>(slot.mInstrument)->SetParameter(name, value) | taken;
        }
    }
    return taken;
}

// Reconstructed from eboot.elf at 0x5A200.
bool SynthRackGenerator::GetParameter(Symbol name, float& value) {
    if (mBusGenerator != nullptr && mBusGenerator->GetParameter(name, value)) {
        return true;
    }
    for (RackSlot& slot : mInstruments) {
        if (slot.mInstrument != nullptr &&
            static_cast<AudioGenerator*>(slot.mInstrument)->GetParameter(name, value)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x5A340.
bool SynthRackGenerator::Process(AudioBuffer<float>& buffer) {
    buffer.Clear();
    ScopedCritSecPtr lock(GetBusLock());
    for (RackSlot& slot : mInstruments) {
        InstrumentGenerator* instrument = slot.mInstrument;
        if (instrument == nullptr) {
            continue;
        }
        instrument->SetBeat(mBeat);
        instrument->Process(mBuffer);
        if (mBuffer.mCleared) {
            continue;
        }
        buffer.Accumulate(mBuffer.mChannelData, buffer.mNumChannels, buffer.mNumFrames);
        buffer.mCleared = false;
    }
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

// Reconstructed from eboot.elf at 0x5A770. The generator's own Stop entry
// (primary slot 57) is called.
void SynthRackGeneratorManager::SendStopToAllGenerators() {
    GeneratorPool::SendStop(*this, mPool);
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
// constructor and calls its own Init entry (primary slot 54).
void SynthRackGeneratorManager::_InitGeneratorPool() {
    GeneratorPool::Init(*this, mPool);
}

// Reconstructed from eboot.elf at 0x5ACD0.
bool SynthRackGeneratorManager::_DeleteGeneratorPool() {
    return GeneratorPool::Delete(*this, mPool);
}

// Reconstructed from eboot.elf at 0x5AD80, which jumps to the base
// destructor.
SynthRackGeneratorManager::~SynthRackGeneratorManager() {}
