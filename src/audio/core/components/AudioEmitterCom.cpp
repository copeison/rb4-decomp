// audio/AudioEmitterCom.o (0x31D70 to 0x392DF). The object also holds the
// DefaultEmitterProxyCom component (0x36B50 to 0x37710), which is not
// reconstructed, and the property accessors and attribute functions of the
// _Init registrations (0x37D70 to 0x390B0).
#include "audio/core/components/AudioEmitterCom.h"

#include <cstring>

#include "audio/core/generators/DialogGenerator.h"
#include "audio/core/music/MusicGenerator.h"
#include "audio/core/system/SoundManager.h"
#include "audio/fmod/system/FmodPlatform.h"
#include "entity/core/GameObject.h"
#include "entity/core/TransCom.h"

namespace {

// Whether the symbol's text is the empty string. The emitter compares its
// render target names by their text, not by their interned addresses.
// Name not in the reference map.
bool IsEmptyName(Symbol name) {
    const char* empty = Symbol().Str();
    if (empty == nullptr) {
        return name.Str() == nullptr;
    }
    return std::strcmp(name.Str(), empty) == 0;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x390C0).
// The int it first sets to -1 (0x19C7718) comes from a shared header and is
// not modelled.
CritSec AudioEmitterCom::sNamedEmittersLock;
eastl::map<Symbol, eastl::vector<AudioEmitterCom*>> AudioEmitterCom::sNamedEmitters;
Symbol AudioEmitterCom::sId("AudioEmitter");
Symbol AudioEmitterCom::sClassName("AudioEmitter");
PropRegistry AudioEmitterCom::sPropRegistry;
ComMetaData AudioEmitterCom::sMetaData;

// Reconstructed from eboot.elf at 0x31D80.
AudioEmitterCom::AudioEmitterCom()
    : mIsNamedEmitter(false),
      mEmitterName(),
      mIs2D(false),
      mAllowDialogOverlap(false),
      mReservedName() {
    mTempoAwareBusProps.mUsePreloadedBank = true;
}

// Reconstructed from eboot.elf at 0x31E20. The tempo-aware buses are only
// queried; the listeners are detached under both locks before the
// emitter's lock is deleted.
AudioEmitterCom::~AudioEmitterCom() {
    theSoundManager.ShutdownGenerator(mRuntime.mComposite);
    for (Symbol* bus = mRuntime.mTempoAwareBuses.begin(); bus != mRuntime.mTempoAwareBuses.end(); ++bus) {
        if (*bus != Symbol() && gFmodBusInterface != nullptr) {
            gFmodBusInterface->IsBusKnown(*bus);
        }
    }

    CritSec* const listenersLock = TempoListener::GetCritSec();
    if (listenersLock != nullptr) {
        listenersLock->Enter();
    }
    CritSec* const lock = mRuntime.mTempoListenersLock;
    if (lock != nullptr) {
        lock->Enter();
    }
    auto& listeners = mRuntime.mTempoListeners;
    if (listeners.mSize != 0) {
        for (unsigned long count = listeners.mSize; count != 0; --count) {
            LinkedListSizeTracked::Node* node = listeners.mNext;
            TempoListener* listener = listeners.Owner(node);
            node->mList = nullptr;
            listener->mOwner = nullptr;
            node->mNext->mPrev = node->mPrev;
            node->mPrev->mNext = node->mNext;
            node->mNext = node;
            node->mPrev = node;
        }
        listeners.mSize = 0;
    }
    if (lock != nullptr) {
        lock->Exit();
    }
    if (listenersLock != nullptr) {
        listenersLock->Exit();
    }

    delete mRuntime.mTempoListenersLock;
    mRuntime.mTempoListenersLock = nullptr;
}

// Reconstructed from eboot.elf at 0x33300.
bool AudioEmitterCom::_OnResourcesLoaded() {
    mRuntime.mComposite.mEmitter = &mRuntime.mEmitter;
    return true;
}

// Reconstructed from eboot.elf at 0x33320.
void AudioEmitterCom::_EditEnter() {
    _Enter();
}

// Reconstructed from eboot.elf at 0x33330.
void AudioEmitterCom::_EditPoll() {
    _Poll();
}

// Reconstructed from eboot.elf at 0x33340.
void AudioEmitterCom::_EditExit(DestroyType type) {
    _Exit(type);
}

// Reconstructed from eboot.elf at 0x33350.
void AudioEmitterCom::_Enter() {
    if (mIsNamedEmitter && mEmitterName != Symbol()) {
        _RegisterNamedEmitter();
    }
}

// Reconstructed from eboot.elf at 0x33370. Only names already in the
// registry (AddGameWideEmitterName) take emitters.
void AudioEmitterCom::_RegisterNamedEmitter() {
    if (mEmitterName == Symbol()) {
        return;
    }
    ScopedCritSec lock(sNamedEmittersLock);
    auto it = sNamedEmitters.find(mEmitterName);
    if (it == sNamedEmitters.end()) {
        return;
    }
    eastl::vector<AudioEmitterCom*>& emitters = it->second;
    for (AudioEmitterCom* emitter : emitters) {
        if (emitter == this) {
            return;
        }
    }
    emitters.push_back(this);
}

// Reconstructed from eboot.elf at 0x33540.
void AudioEmitterCom::_Poll() {
    const Transform xfm = mObject->GetCom<TransCom>()->mWorldXfm;
    _Poll(xfm);
}

// Reconstructed from eboot.elf at 0x335E0.
void AudioEmitterCom::_Exit(DestroyType type) {
    static_cast<void>(type);
    if (mIsNamedEmitter && mEmitterName != Symbol()) {
        _UnregisterNamedEmitter();
    }
    mRuntime.mComposite.Stop();
}

// Reconstructed from eboot.elf at 0x33620.
void AudioEmitterCom::_UnregisterNamedEmitter() {
    if (mEmitterName == Symbol()) {
        return;
    }
    ScopedCritSec lock(sNamedEmittersLock);
    auto it = sNamedEmitters.find(mEmitterName);
    if (it == sNamedEmitters.end()) {
        return;
    }
    eastl::vector<AudioEmitterCom*>& emitters = it->second;
    for (AudioEmitterCom** emitter = emitters.begin(); emitter != emitters.end(); ++emitter) {
        if (*emitter == this) {
            emitters.erase(emitter);
            return;
        }
    }
}

// Reconstructed from eboot.elf at 0x33750.
void AudioEmitterCom::StopAllSounds() {
    mRuntime.mComposite.Stop();
}

// Reconstructed from eboot.elf at 0x33760. A bus whose tempo listeners the
// FMOD bus interface cannot provide yet is tried again next poll. Without
// master music the positions are cleared once.
bool AudioEmitterCom::_Poll(const Transform& xfm) {
    mRuntime.mWorldXfm = xfm;
    mRuntime.mComposite.Poll();
    if (mRuntime.mTempoAwareBuses.size() != mTempoAwareBusProps.mBuses.size()) {
        _UpdateTempoAwareBuses();
    }
    for (unsigned long i = 0; i < mRuntime.mTempoAwareBusesBound.size(); ++i) {
        if (mRuntime.mTempoAwareBusesBound[i]) {
            continue;
        }
        eastl::vector<TempoListener*> listeners;
        if (gFmodBusInterface != nullptr &&
            gFmodBusInterface->GetBusTempoListeners(mRuntime.mTempoAwareBuses[i], listeners)) {
            mRuntime.mTempoAwareBusesBound[i] = true;
            for (TempoListener* listener : listeners) {
                RegisterTempoListener(listener);
                mRuntime.mBusTempoListeners.push_back(listener);
            }
        }
    }

    AudioGenerator* master = theSoundManager.LockIfOwned(mRuntime.mMasterMusic);
    if (master != nullptr) {
        if (master->IsMusic()) {
            auto* music = static_cast<MusicGenerator*>(master);
            mRuntime.mContentPos = music->GetCurrentContentPos();
            mRuntime.mSongPos = music->GetCurrentSongPos();
            mRuntime.mSectionName = music->GetCurrentSectionName();
            mRuntime.mMusicPlaying = true;
            const float tempo = music->GetCurrentBPM();
            const float speed = master->GetSpeed(nullptr);
            if (tempo != mRuntime.mTempo || speed != mRuntime.mSpeed) {
                mRuntime.mTempo = tempo;
                mRuntime.mSpeed = speed;
                CritSec* const lock = mRuntime.mTempoListenersLock;
                if (lock != nullptr) {
                    lock->Enter();
                }
                auto& tempoListeners = mRuntime.mTempoListeners;
                for (LinkedListSizeTracked::Node* node = tempoListeners.mNext;
                     node != tempoListeners.Sentinel();
                     node = node->mNext) {
                    tempoListeners.Owner(node)->OnTempoChanged(mRuntime.mTempo, mRuntime.mSpeed);
                }
                if (lock != nullptr) {
                    lock->Exit();
                }
            }
            --master->mRefCount;
            return true;
        }
        --master->mRefCount;
    }
    if (mRuntime.mMusicPlaying) {
        mRuntime.mContentPos.Set(0.0F, 0.0F, 0, 0, 0, 0, 0);
        mRuntime.mSongPos.Set(0.0F, 0.0F, 0, 0, 0, 0, 0);
        mRuntime.mSectionName = Symbol("");
        mRuntime.mMusicPlaying = false;
    }
    return true;
}

// Reconstructed from eboot.elf at 0x33C70. A bus that is not loaded keeps
// its place with an empty path and counts as bound.
void AudioEmitterCom::_UpdateTempoAwareBuses() {
    for (TempoListener* listener : mRuntime.mBusTempoListeners) {
        UnregisterTempoListener(listener);
    }
    for (Symbol* bus = mRuntime.mTempoAwareBuses.begin(); bus != mRuntime.mTempoAwareBuses.end(); ++bus) {
        if (*bus != Symbol() && gFmodBusInterface != nullptr) {
            gFmodBusInterface->IsBusKnown(*bus);
        }
    }
    mRuntime.mTempoAwareBuses.clear();
    mRuntime.mTempoAwareBusesBound.clear();
    mRuntime.mBusTempoListeners.clear();
    if (gFmodBusInterface == nullptr) {
        return;
    }
    for (const TempoAwareBusProps::BusName& entry : mTempoAwareBusProps.mBuses) {
        const Symbol path = mTempoAwareBusProps.mUsePreloadedBank ? entry.mPreloadedBus : entry.mBus;
        if (gFmodBusInterface->IsBusLoaded(path)) {
            mRuntime.mTempoAwareBuses.push_back(path);
            mRuntime.mTempoAwareBusesBound.push_back(false);
        } else {
            mRuntime.mTempoAwareBuses.push_back(Symbol());
            mRuntime.mTempoAwareBusesBound.push_back(true);
        }
    }
}

// Reconstructed from eboot.elf at 0x34120.
void AudioEmitterCom::RegisterTempoListener(TempoListener* listener) {
    CritSec* const listenersLock = TempoListener::GetCritSec();
    if (listenersLock != nullptr) {
        listenersLock->Enter();
    }
    if (listener->mOwner != nullptr) {
        listener->mOwner->UnregisterTempoListener(listener);
    }
    listener->mOwner = &mRuntime.mEmitter;
    listener->OnTempoChanged(mRuntime.mTempo, mRuntime.mSpeed);
    CritSec* const lock = mRuntime.mTempoListenersLock;
    if (lock != nullptr) {
        lock->Enter();
    }
    mRuntime.mTempoListeners.PushBack(*listener);
    if (lock != nullptr) {
        lock->Exit();
    }
    if (listenersLock != nullptr) {
        listenersLock->Exit();
    }
}

// Reconstructed from eboot.elf at 0x34210.
unsigned int AudioEmitterCom::PlaySound(PlayArgs& args) {
    if (args.mFormat == PlayMusicArgs::kMusicFormat) {
        return PlayMusic(static_cast<PlayMusicArgs&>(args));
    }
    return _PlayOnEmitter(args);
}

// Reconstructed from eboot.elf at 0x342C0. The binary has no caller; the
// emitter's other plays inline it.
unsigned int AudioEmitterCom::_PlayOnEmitter(PlayArgs& args) {
    bool setEmitter = false;
    if (args.mEmitter == nullptr) {
        args.mEmitter = &mRuntime.mEmitter;
        setEmitter = true;
    }
    if (IsEmptyName(args.mRenderTarget)) {
        args.mRenderTarget = mRuntime.mRenderTarget;
    }
    const unsigned int handle = theSoundManager.PlaySound(args);
    if (setEmitter) {
        args.mEmitter = nullptr;
    }
    return handle;
}

// Reconstructed from eboot.elf at 0x34360.
unsigned int AudioEmitterCom::PlaySound(Symbol name) {
    if (IsEmptyName(mRuntime.mRenderTarget)) {
        return theSoundManager.PlaySound(name, &mRuntime.mEmitter, false);
    }
    PlayArgs args;
    args.mName = name;
    args.mRenderTarget = mRuntime.mRenderTarget;
    return _PlayOnEmitter(args);
}

// Reconstructed from eboot.elf at 0x34560.
unsigned int AudioEmitterCom::PrepareSound(Symbol name) {
    if (IsEmptyName(mRuntime.mRenderTarget)) {
        return theSoundManager.PlaySound(name, &mRuntime.mEmitter, true);
    }
    PlayArgs args;
    args.mName = name;
    args.mRenderTarget = mRuntime.mRenderTarget;
    args.mStartPaused = true;
    return _PlayOnEmitter(args);
}

// Reconstructed from eboot.elf at 0x34760. A slave request follows the
// emitter's master music unless it names its own master, which must be
// music.
unsigned int AudioEmitterCom::PlayMusic(PlayMusicArgs& args) {
    if (args.mOptions.mSync == kSlave) {
        if (args.mOptions.mMasterHandle == 0) {
            args.mOptions.mMasterHandle = mRuntime.mMasterMusic;
        }
        AudioGenerator* master = theSoundManager.LockIfOwned(args.mOptions.mMasterHandle);
        if (master == nullptr) {
            return 0;
        }
        const bool isMusic = master->IsMusic();
        --master->mRefCount;
        if (!isMusic) {
            return 0;
        }
    }
    const unsigned int handle = _PlayOnEmitter(args);
    AudioGenerator* generator = theSoundManager.LockIfOwned(handle);
    if (generator != nullptr) {
        --generator->mRefCount;
        _SetupEmitterMusic(handle, args);
    }
    return handle;
}

// Reconstructed from eboot.elf at 0x34860.
void AudioEmitterCom::_SetupEmitterMusic(unsigned int handle, const PlayMusicArgs& args) {
    if (args.mOptions.mSync != kMaster) {
        return;
    }
    AudioGenerator* generator = theSoundManager.LockIfOwned(handle);
    if (generator == nullptr) {
        return;
    }
    if (generator->IsMusic()) {
        mRuntime.mMasterMusic = handle;
        mRuntime.mTempo = static_cast<MusicGenerator*>(generator)->GetCurrentBPM();
        CritSec* const lock = mRuntime.mTempoListenersLock;
        if (lock != nullptr) {
            lock->Enter();
        }
        auto& listeners = mRuntime.mTempoListeners;
        for (LinkedListSizeTracked::Node* node = listeners.mNext; node != listeners.Sentinel();
             node = node->mNext) {
            listeners.Owner(node)->OnTempoChanged(mRuntime.mTempo, mRuntime.mSpeed);
        }
        if (lock != nullptr) {
            lock->Exit();
        }
    }
    --generator->mRefCount;
}

// Reconstructed from eboot.elf at 0x34950.
bool AudioEmitterCom::HasMixGroup() const {
    return false;
}

// Reconstructed from eboot.elf at 0x34960.
unsigned int AudioEmitterCom::PlayMusic(
    Symbol name, MusicSyncOptions sync, MusicTimelineMapping mapping, MusicUnmutePoint unmute) {
    PlayMusicArgs args;
    args.mEmitter = &mRuntime.mEmitter;
    args.mName = name;
    args.mStartPaused = false;
    args.mOptions.mMasterHandle = mRuntime.mMasterMusic;
    args.mOptions.mSync = sync;
    args.mOptions.mMapping = mapping;
    args.mOptions.mUnmutePoint = unmute;
    return PlayMusic(args);
}

// Reconstructed from eboot.elf at 0x34B40.
unsigned int AudioEmitterCom::PlayMusic(Symbol name, Symbol sync) {
    return PlayMusic(
        name,
        MusicGenerator::SymbolToMusicSyncOptions(sync),
        static_cast<MusicTimelineMapping>(1),
        static_cast<MusicUnmutePoint>(0));
}

// Reconstructed from eboot.elf at 0x34B90.
unsigned int AudioEmitterCom::PrepareMusic(Symbol name, MusicSyncOptions sync) {
    PlayMusicArgs args;
    args.mEmitter = &mRuntime.mEmitter;
    args.mName = name;
    args.mStartPaused = true;
    args.mOptions.mMasterHandle = mRuntime.mMasterMusic;
    args.mOptions.mSync = sync;
    args.mOptions.mMapping = static_cast<MusicTimelineMapping>(1);
    args.mOptions.mUnmutePoint = static_cast<MusicUnmutePoint>(0);
    return PlayMusic(args);
}

// Reconstructed from eboot.elf at 0x34D60.
unsigned int AudioEmitterCom::PrepareMusic(Symbol name, Symbol sync) {
    return PrepareMusic(name, MusicGenerator::SymbolToMusicSyncOptions(sync));
}

// Reconstructed from eboot.elf at 0x35110.
unsigned int AudioEmitterCom::PlayDialog(DialogPlayArgs& args) {
    if (!mRuntime.mDialogInterruptible && !mAllowDialogOverlap && IsDialogPlaying()) {
        return 0;
    }
    bool setEmitter = false;
    if (args.mEmitter == nullptr) {
        args.mEmitter = &mRuntime.mEmitter;
        setEmitter = true;
    }
    if (!mAllowDialogOverlap) {
        AudioGenerator* line = theSoundManager.LockIfOwned(mRuntime.mDialogHandle);
        if (line != nullptr) {
            line->Stop();
            --line->mRefCount;
        }
    }
    mRuntime.mDialogHandle = _PlayOnEmitter(args);
    mRuntime.mDialogInterruptible = args.mInterruptible;
    if (setEmitter) {
        args.mEmitter = nullptr;
    }
    return mRuntime.mDialogHandle;
}

// Reconstructed from eboot.elf at 0x35230.
bool AudioEmitterCom::SetDialogInterruptible(bool interruptible) {
    AudioGenerator* line = theSoundManager.LockIfOwned(mRuntime.mDialogHandle);
    if (line == nullptr) {
        return false;
    }
    if (line->IsDialog()) {
        mRuntime.mDialogInterruptible = interruptible;
        --line->mRefCount;
        return true;
    }
    --line->mRefCount;
    return false;
}

// Reconstructed from eboot.elf at 0x352E0.
bool AudioEmitterCom::Is2D() const {
    return mIs2D;
}

// Reconstructed from eboot.elf at 0x352F0.
bool AudioEmitterCom::Is3D() const {
    return !mIs2D;
}

// Reconstructed from eboot.elf at 0x35300.
void AudioEmitterCom::Set2D(bool is2D) {
    mIs2D = is2D;
}

// Reconstructed from eboot.elf at 0x35310.
void AudioEmitterCom::Set3D(bool is3D) {
    mIs2D = !is3D;
}

// Reconstructed from eboot.elf at 0x35320.
void AudioEmitterCom::KillAllSounds() {
    mRuntime.mComposite.KillLocked();
}

// Reconstructed from eboot.elf at 0x35330.
void AudioEmitterCom::PauseAllSounds() {
    mRuntime.mComposite.Pause();
}

// Reconstructed from eboot.elf at 0x35340.
void AudioEmitterCom::ContinueAllSounds() {
    mRuntime.mComposite.Continue();
}

// Reconstructed from eboot.elf at 0x35350.
bool AudioEmitterCom::SetParameter(Symbol name, float value) {
    return mRuntime.mComposite.SetParameter(name, value);
}

// Reconstructed from eboot.elf at 0x35360.
bool AudioEmitterCom::GetParameter(Symbol name, float& value) {
    return mRuntime.mComposite.GetParameter(name, value);
}

// Reconstructed from eboot.elf at 0x35370.
bool AudioEmitterCom::UnregisterTempoListener(TempoListener* listener) {
    CritSec* const listenersLock = TempoListener::GetCritSec();
    if (listenersLock != nullptr) {
        listenersLock->Enter();
    }
    if (listener->mOwner == &mRuntime.mEmitter) {
        listener->mOwner = nullptr;
    }
    CritSec* const lock = mRuntime.mTempoListenersLock;
    if (lock != nullptr) {
        lock->Enter();
    }
    bool removed = false;
    if (listener->mNode.mList == &mRuntime.mTempoListeners) {
        mRuntime.mTempoListeners.Remove(*listener);
        removed = true;
    }
    if (lock != nullptr) {
        lock->Exit();
    }
    if (listenersLock != nullptr) {
        listenersLock->Exit();
    }
    return removed;
}

// Reconstructed from eboot.elf at 0x35450. The new entry holds a null
// emitter before any named emitter joins it.
void AddGameWideEmitterName(Symbol name) {
    ScopedCritSec lock(AudioEmitterCom::sNamedEmittersLock);
    if (AudioEmitterCom::sNamedEmitters.find(name) != AudioEmitterCom::sNamedEmitters.end()) {
        return;
    }
    eastl::vector<AudioEmitterCom*> emitters;
    emitters.reserve(8);
    emitters.push_back(nullptr);
    AudioEmitterCom::sNamedEmitters[name] = emitters;
}

// Reconstructed from eboot.elf at 0x35820.
AudioEmitterCom* GetGameWideEmitter(Symbol name) {
    ScopedCritSec lock(AudioEmitterCom::sNamedEmittersLock);
    auto it = AudioEmitterCom::sNamedEmitters.find(name);
    if (it == AudioEmitterCom::sNamedEmitters.end()) {
        return nullptr;
    }
    return it->second.back();
}

// Reconstructed from eboot.elf at 0x35B30.
bool AudioEmitterCom::IsPlaying() const {
    return mRuntime.mComposite.IsPlaying();
}

// Reconstructed from eboot.elf at 0x37030.
Symbol AudioEmitterCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x37040.
Symbol AudioEmitterCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x37050.
int AudioEmitterCom::CurrentRev() const {
    return const_cast<AudioEmitterCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x37070.
bool AudioEmitterCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x370A0.
Component* AudioEmitterCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x37240.
PropRegistry& AudioEmitterCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x37250.
ComMetaData& AudioEmitterCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x37260.
unsigned int AudioEmitterCom::GetMasterMusic() {
    return mRuntime.mMasterMusic;
}

// Reconstructed from eboot.elf at 0x37270.
SongPos AudioEmitterCom::GetCurrentMusicSongPos() {
    return mRuntime.mSongPos;
}

// Reconstructed from eboot.elf at 0x37290.
SongPos AudioEmitterCom::GetCurrentMusicContentPos() {
    return mRuntime.mContentPos;
}

// Reconstructed from eboot.elf at 0x372B0.
Symbol AudioEmitterCom::GetCurrentMusicSectionName() {
    return mRuntime.mSectionName;
}

// Reconstructed from eboot.elf at 0x372C0.
bool AudioEmitterCom::IsDialogPlaying() {
    AudioGenerator* line = theSoundManager.LockIfOwned(mRuntime.mDialogHandle);
    if (line == nullptr) {
        return false;
    }
    --line->mRefCount;
    return true;
}

// Reconstructed from eboot.elf at 0x372F0.
bool AudioEmitterCom::IsDialogUninterruptible() {
    if (mRuntime.mDialogHandle == 0) {
        return false;
    }
    return !mRuntime.mDialogInterruptible;
}

// Reconstructed from eboot.elf at 0x37310.
void AudioEmitterCom::SetAllowDialogOverlap(bool allow) {
    mAllowDialogOverlap = allow;
}

// Reconstructed from eboot.elf at 0x37320.
void AudioEmitterCom::AddGenerator(AudioGenerator* generator) {
    mRuntime.mComposite.AddGenerator(generator);
}

// Reconstructed from eboot.elf at 0x37710. The interface's back pointer is
// the enclosing component.
AudioEmitterCom::RuntimeData::RuntimeData()
    : mSongPos(),
      mContentPos(),
      mSectionName(""),
      mTempo(120.0F),
      mSpeed(120.0F),
      mMasterMusic(0),
      mWorldXfm{{{1.0F, 0.0F, 0.0F}, {0.0F, 1.0F, 0.0F}, {0.0F, 0.0F, 1.0F}}, {0.0F, 0.0F, 0.0F}},
      mTempoListenersLock(nullptr),
      mListener(nullptr),
      mMusicPlaying(false),
      mDialogHandle(0),
      mDialogInterruptible(true),
      mRenderTarget("") {
    mTempoListenersLock = new CritSec();
    mEmitter.mComponent = reinterpret_cast<AudioEmitterCom*>(
        reinterpret_cast<char*>(this) - offsetof(AudioEmitterCom, mRuntime));
}

// Reconstructed from eboot.elf at 0x379D0.
AudioGenerator* AudioEmitter::GetCompositeGenerator() {
    return &mComponent->mRuntime.mComposite;
}

// Reconstructed from eboot.elf at 0x379E0.
void AudioEmitter::RegisterTempoListener(TempoListener* listener) {
    mComponent->RegisterTempoListener(listener);
}

// Reconstructed from eboot.elf at 0x379F0.
bool AudioEmitter::UnregisterTempoListener(TempoListener* listener) {
    return mComponent->UnregisterTempoListener(listener);
}

// Reconstructed from eboot.elf at 0x37A00.
unsigned int AudioEmitter::PlaySound(PlayArgs& args) {
    return mComponent->PlaySound(args);
}

// Reconstructed from eboot.elf at 0x37AB0.
unsigned int AudioEmitter::PlaySound(Symbol name) {
    return mComponent->PlaySound(name);
}

// Reconstructed from eboot.elf at 0x37AC0.
unsigned int AudioEmitter::PrepareSound(Symbol name) {
    return mComponent->PrepareSound(name);
}

// Reconstructed from eboot.elf at 0x37AD0.
unsigned int AudioEmitter::PlayMusic(PlayMusicArgs& args) {
    return mComponent->PlayMusic(args);
}

// Reconstructed from eboot.elf at 0x37AE0.
unsigned int AudioEmitter::PlayMusic(
    Symbol name, MusicSyncOptions sync, MusicTimelineMapping mapping, MusicUnmutePoint unmute) {
    return mComponent->PlayMusic(name, sync, mapping, unmute);
}

// Reconstructed from eboot.elf at 0x37AF0.
unsigned int AudioEmitter::PlayMusic(Symbol name, Symbol sync) {
    return mComponent->PlayMusic(name, sync);
}

// Reconstructed from eboot.elf at 0x37B00.
unsigned int AudioEmitter::PrepareMusic(Symbol name, MusicSyncOptions sync) {
    return mComponent->PrepareMusic(name, sync);
}

// Reconstructed from eboot.elf at 0x37B10.
unsigned int AudioEmitter::PrepareMusic(Symbol name, Symbol sync) {
    return mComponent->PrepareMusic(name, sync);
}

// Reconstructed from eboot.elf at 0x37B20.
void AudioEmitter::StopAllSounds() {
    mComponent->mRuntime.mComposite.Stop();
}

// Reconstructed from eboot.elf at 0x37B40.
void AudioEmitter::KillAllSounds() {
    mComponent->mRuntime.mComposite.KillLocked();
}

// Reconstructed from eboot.elf at 0x37B60.
void AudioEmitter::PauseAllSounds() {
    mComponent->mRuntime.mComposite.Pause();
}

// Reconstructed from eboot.elf at 0x37B80.
void AudioEmitter::ContinueAllSounds() {
    mComponent->mRuntime.mComposite.Continue();
}

// Reconstructed from eboot.elf at 0x37BA0.
unsigned int AudioEmitter::GetMasterMusic() {
    return mComponent->GetMasterMusic();
}

// Reconstructed from eboot.elf at 0x37BB0.
void* AudioEmitter::GetMixGroup() {
    return nullptr;
}

// Reconstructed from eboot.elf at 0x37BC0.
const Transform& AudioEmitter::GetWorldXfm() {
    if (mComponent->mIs2D) {
        return theSoundManager.mListenerXfm;
    }
    return mComponent->mRuntime.mWorldXfm;
}

// Reconstructed from eboot.elf at 0x37CB0.
unsigned int AudioEmitter::PlayDialog(DialogPlayArgs& args) {
    return mComponent->PlayDialog(args);
}

// Reconstructed from eboot.elf at 0x37CC0.
bool AudioEmitter::IsDialogPlaying() {
    return mComponent->IsDialogPlaying();
}

// Reconstructed from eboot.elf at 0x37CD0.
bool AudioEmitter::IsDialogUninterruptible() {
    return mComponent->IsDialogUninterruptible();
}

// Reconstructed from eboot.elf at 0x37CE0.
bool AudioEmitter::SetDialogInterruptible(bool interruptible) {
    return mComponent->SetDialogInterruptible(interruptible);
}

// Reconstructed from eboot.elf at 0x37D00.
void AudioEmitter::SetAllowDialogOverlap(bool allow) {
    mComponent->SetAllowDialogOverlap(allow);
}

// Reconstructed from eboot.elf at 0x37D20.
bool AudioEmitter::Is2D() {
    return mComponent->mIs2D;
}

// Reconstructed from eboot.elf at 0x37D30.
bool AudioEmitter::Is3D() {
    return !mComponent->mIs2D;
}

// Reconstructed from eboot.elf at 0x37D40.
void AudioEmitter::Set2D(bool is2D) {
    mComponent->mIs2D = is2D;
}

// Reconstructed from eboot.elf at 0x37D50.
void AudioEmitter::Set3D(bool is3D) {
    mComponent->mIs2D = !is3D;
}

// Reconstructed from eboot.elf at 0x37D60.
Component* AudioEmitter::GetComponent() {
    return mComponent;
}

// Reconstructed from eboot.elf at 0xCF80. The binary emits the factory in
// audio/SoundManager.o with the class's Init.
Component* AudioEmitterCom::_Create() {
    return new AudioEmitterCom();
}
