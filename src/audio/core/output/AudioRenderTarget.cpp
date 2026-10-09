#include "audio/core/output/AudioRenderTarget.h"

#include <cstring>

#include "audio/core/fusion/FusionVoicePool.h"
#include "os/threading/CritSec.h"

// Reconstructed from eboot.elf at 0x1127770.
AudioMixer::AudioMixer() : mSampleRate(0), mMixCount(0), mBlock(0), mLastBlock(false) {}

// Reconstructed from eboot.elf at 0x1127880. Callables queued since the last
// mix are promoted first. Every block is then prepared on all callables, and
// each callable makes its samples at most once per block. The original
// assumes the buffer length is a multiple of 128.
void AudioMixer::ExecutePremix(int numSamples, unsigned long mixCount) {
    ScopedCritSecPtr tracker(&mCritSec);

    {
        ScopedCritSecPtr pendingTracker(&mPendingCritSec);
        while (!mPendingCallables.Empty()) {
            auto* callable = mPendingCallables.PopFront();
            mCallables.PushBack(*callable);
        }
    }

    auto remaining = static_cast<unsigned int>(numSamples);
    auto* const sentinel = mCallables.Sentinel();
    int block = 0;
    while (remaining != 0) {
        const bool lastBlock = remaining == kBlockSamples;
        const float sampleRate = static_cast<float>(mSampleRate);
        for (auto* node = mCallables.mNext; node != sentinel; node = node->mNext) {
            CallableList::Owner(node)->_PrepareToMakeSamples(
                kBlockSamples, sampleRate, static_cast<int>(mixCount), block, lastBlock);
        }

        mMixCount = static_cast<int>(mixCount);
        mBlock = block;
        mLastBlock = lastBlock;

        for (auto* node = mCallables.mNext; node != sentinel; node = node->mNext) {
            CallableList::Owner(node)->mMakeGuard.store(0);
        }
        for (auto* node = mCallables.mNext; node != sentinel; node = node->mNext) {
            auto* callable = CallableList::Owner(node);
            if (callable->mMakeGuard.load() <= 0 &&
                callable->mMakeGuard.fetch_add(1) == 0) {
                callable->_MakeSamples(
                    kBlockSamples,
                    static_cast<float>(mSampleRate),
                    mMixCount,
                    mBlock,
                    mLastBlock);
            }
        }

        ++block;
        remaining -= kBlockSamples;
    }
}

// Reconstructed from eboot.elf at 0x1127B80.
void AudioMixer::SetOutputSampleRate(int sampleRate) {
    mSampleRate = sampleRate;
}

// Reconstructed from eboot.elf at 0x1127B90.
void AudioMixer::SetSampleRate(int sampleRate) {
    mSampleRate = sampleRate;
}

// Reconstructed from eboot.elf at 0x1127BA0. Both lists release their
// callables before their locks are destroyed.
AudioMixer::~AudioMixer() {}

// Reconstructed from eboot.elf at 0x57560. The pending list is checked
// first; each list is searched under its own lock.
bool AudioMixer::RemoveCallable(AudioBusCallable* callable) {
    {
        ScopedCritSecPtr pendingTracker(&mPendingCritSec);
        if (callable->_mCallbackNode.mList == &mPendingCallables) {
            mPendingCallables.Remove(*callable);
            return true;
        }
    }
    ScopedCritSecPtr tracker(&mCritSec);
    if (callable->_mCallbackNode.mList == &mCallables) {
        mCallables.Remove(*callable);
        return true;
    }
    return false;
}

// Reconstructed from eboot.elf at 0x1127FC0.
AudioRenderTarget::AudioRenderTarget(Symbol name, int sampleRate)
    : mName(name), mType(static_cast<Type>(0)), mVoicePool(nullptr), mSampleRate(sampleRate) {}

// Reconstructed from eboot.elf at 0x11281D0.
void AudioRenderTarget::InitBase(Symbol name) {
    mName = name;
    mMixer.mSampleRate = mSampleRate;
}

// Reconstructed from eboot.elf at 0x11281F0. The target leaves the registry
// before its voice pool and lists are destroyed.
AudioRenderTarget::~AudioRenderTarget() {
    gAudioRenderTargets.Unregister(this);
    delete mVoicePool;
    mVoicePool = nullptr;
}

// Reconstructed from eboot.elf at 0x1128350.
void AudioRenderTarget::Lock() {
    mMixer.Lock();
}

// Reconstructed from eboot.elf at 0x1128370.
void AudioRenderTarget::Unlock() {
    mMixer.Unlock();
}

// Reconstructed from eboot.elf at 0x1128380.
void AudioRenderTarget::AddPremixCallback(FmodPremixCallback* callback) {
    ScopedCritSecPtr tracker(&mPremixCritSec);
    mPremixCallbacks.PushBack(callback->mCallbackNode);
}

// Reconstructed from eboot.elf at 0x11283F0.
void AudioRenderTarget::RemovePremixCallback(FmodPremixCallback* callback) {
    ScopedCritSecPtr tracker(&mPremixCritSec);
    if (callback != nullptr && callback->mCallbackNode.mList == &mPremixCallbacks) {
        mPremixCallbacks.Remove(callback->mCallbackNode);
    }
}

// Reconstructed from eboot.elf at 0x1128480.
int AudioRenderTarget::GetNumSpeakers(int speakerConfig) {
    static constexpr int kNumSpeakers[] = {1, 2, 6, 8};  // At 0x136B8FC.
    if (static_cast<unsigned int>(speakerConfig) >= 4) {
        return 0;
    }
    return kNumSpeakers[speakerConfig];
}

// Reconstructed from eboot.elf at 0x11284A0.
void* AudioRenderTarget::GetTimer(unsigned long key) {
    if (mTimers.find(key) == mTimers.end()) {
        return nullptr;
    }
    return mTimers[key];
}

// Reconstructed from eboot.elf at 0x11285D0. The pool comes from the small
// block allocator; its voices are created once every limit is set.
void AudioRenderTarget::InitVoicePool(
    int hardVoiceLimit, int softVoiceLimit, int numPitchShifters, bool skipVoiceDecoders) {
    delete mVoicePool;
    mVoicePool = nullptr;
    mVoicePool = new FusionVoicePool(static_cast<float>(mSampleRate));
    mVoicePool->SetSkipVoiceDecoders(skipVoiceDecoders);
    mVoicePool->SetHardVoiceLimit(static_cast<unsigned int>(hardVoiceLimit));
    mVoicePool->SetSoftVoiceLimit(static_cast<unsigned int>(softVoiceLimit));
    mVoicePool->SetNumPitchShifters(numPitchShifters);
    mVoicePool->CreatePendingVoices();
}

// Reconstructed from eboot.elf at 0x11286A0.
void AudioRenderTarget::ConfigureVoicePool(int softVoiceLimit, int hardVoiceLimit) {
    if (mVoicePool != nullptr) {
        mVoicePool->SetSoftVoiceLimit(static_cast<unsigned int>(softVoiceLimit));
        mVoicePool->SetHardVoiceLimit(static_cast<unsigned int>(hardVoiceLimit));
    }
}

// Reconstructed from eboot.elf at 0x11286E0. The lookup's result is unused.
void AudioRenderTarget::SetTimer(unsigned long key, void* timer) {
    GetTimer(key);
    mTimers[key] = timer;
}

// Reconstructed from eboot.elf at 0x11287B0.
int AudioRenderTarget::Update() {
    return 0;
}

// Reconstructed from eboot.elf at 0x11287C0.
void AudioRenderTarget::ExecutePremixCallbacks(unsigned long) {}

// Reconstructed from the identical FModSystem::GetVoicePool at 0x278C40.
FusionVoicePool* AudioRenderTarget::GetVoicePool() {
    return mVoicePool;
}

// Reconstructed from the identical FmodRecordingAudioRenderTarget::TryBeginMix
// at 0x276180.
bool AudioRenderTarget::TryBeginMix() {
    return false;
}

// Reconstructed from the identical FmodRecordingAudioRenderTarget::EndMix at
// 0x276190.
bool AudioRenderTarget::EndMix() {
    return false;
}

// Reconstructed from the identical FModSystem::GetMixer at 0x278C50.
AudioMixer* AudioRenderTarget::GetMixer() {
    return &mMixer;
}

AudioRenderTargetRegistry gAudioRenderTargets;

// Reconstructed from eboot.elf at 0xC0F70.
AudioRenderTargetRegistry::~AudioRenderTargetRegistry() {
    DeleteAll();
}

// Reconstructed from eboot.elf at 0xC0F90. A deleted target unregisters
// itself, so the map is walked through a copy.
void AudioRenderTargetRegistry::DeleteAll() {
    eastl::map<Symbol, AudioRenderTarget*> targets(mTargets);
    for (auto it = targets.begin(); it != targets.end(); ++it) {
        if (std::strcmp(it->first.Str(), Symbol().Str()) != 0) {
            delete it->second;
        }
    }
    mTargets.clear();
}

// Reconstructed from eboot.elf at 0xC13E0.
void AudioRenderTargetRegistry::SetDefault(AudioRenderTarget* target) {
    mDefault = target;
    mTargets[Symbol("")] = target;
}

// Reconstructed from eboot.elf at 0xC14B0.
AudioRenderTarget* AudioRenderTargetRegistry::Find(Symbol name, bool useDefault) {
    if (mTargets.find(name) != mTargets.end()) {
        return mTargets[name];
    }
    return useDefault ? mDefault : nullptr;
}

// Reconstructed from eboot.elf at 0xC15E0.
bool AudioRenderTargetRegistry::Register(AudioRenderTarget* target) {
    if (mTargets.find(target->mName) != mTargets.end()) {
        return false;
    }
    mTargets[target->mName] = target;
    return true;
}

// Reconstructed from eboot.elf at 0xC1700.
bool AudioRenderTargetRegistry::Unregister(AudioRenderTarget* target) {
    if (mTargets.find(target->mName) == mTargets.end()) {
        return false;
    }
    auto it = mTargets.lower_bound(target->mName);
    if (it != mTargets.end() && !(target->mName < it->first)) {
        mTargets.erase(it);
    }
    return true;
}
