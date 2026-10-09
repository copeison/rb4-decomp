#include "audio/core/fusion/FusionVoicePool.h"

#include "audio/core/dsp/SmbPitchShift.h"
#include "audio/core/fusion/FusionSampler.h"
#include "audio/core/fusion/FusionVoice.h"
#include "audio/core/system/Audio.h"

namespace {

constexpr unsigned int kDefaultVoiceLimit = 16;
constexpr unsigned int kMaxVoiceLimit = 256;
// Shifter frame size and oversampling. Names not in the reference map.
constexpr int kPitchShiftFrameSize = 1024;
constexpr int kPitchShiftOversampling = 4;
// A keyzone without a priority ranks below every other.
constexpr unsigned char kLowestPriority = 0xFF;

unsigned char VoicePriority(const FusionVoice* voice) {
    return voice->mKeyzone != nullptr ? voice->mKeyzone->mPriority : kLowestPriority;
}

}  // namespace

// Reconstructed from eboot.elf at 0xA0400.
FusionVoicePool::PitchShiftSlot::~PitchShiftSlot() {
    delete mPitchShift;
    mPitchShift = nullptr;
}

// Reconstructed from eboot.elf at 0xA0430.
FusionVoicePool::FusionVoicePool()
    : mVoices(nullptr),
      mNumVoices(0),
      mHardVoiceLimit(kDefaultVoiceLimit),
      mNumPitchShifts(0),
      mNumFreePitchShifts(0),
      mPitchShiftCount(0),
      mSoftVoiceLimit(kDefaultVoiceLimit),
      mPeakVoicesInUse(0),
      mSkipVoiceDecoders(false),
      mSampleRate(static_cast<float>(Audio::GetSamplesPerSecond())),
      mVoicesPending(true) {}

// Reconstructed from eboot.elf at 0xA0520.
void FusionVoicePool::SetSampleRate(float sampleRate) {
    if (sampleRate == mSampleRate) {
        return;
    }
    mCritSec.Enter();
    KillVoices();
    _DestroyVoices();
    mSampleRate = sampleRate;
    if (!mVoicesPending || mClients.size() != 0) {
        _CreateVoices(static_cast<unsigned short>(mHardVoiceLimit));
        _CreatePitchShifters(static_cast<unsigned short>(mPitchShiftCount));
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA05F0.
void FusionVoicePool::KillVoices() {
    mCritSec.Enter();
    for (unsigned int i = 0; i < mNumVoices; ++i) {
        mVoices[i].Kill();
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA0660. The scan for a voice still in use
// fed a warning the release build strips.
void FusionVoicePool::_DestroyVoices() {
    for (unsigned int i = 0; i < mNumVoices; ++i) {
        if (mVoices[i].IsInUse()) {
            break;
        }
    }
    mNumVoices = 0;
    delete[] mVoices;
    mVoices = nullptr;
    mNumPitchShifts = 0;
    mPitchShifts.clear();
}

// Reconstructed from eboot.elf at 0xA0750.
void FusionVoicePool::_CreateAllVoices() {
    _CreateVoices(static_cast<unsigned short>(mHardVoiceLimit));
    _CreatePitchShifters(static_cast<unsigned short>(mPitchShiftCount));
}

// Reconstructed from eboot.elf at 0xA0780.
void FusionVoicePool::Lock() {
    mCritSec.Enter();
}

// Reconstructed from eboot.elf at 0xA07A0.
void FusionVoicePool::Unlock() {
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA07B0.
FusionVoicePool::FusionVoicePool(float sampleRate)
    : mVoices(nullptr),
      mNumVoices(0),
      mHardVoiceLimit(kDefaultVoiceLimit),
      mNumPitchShifts(0),
      mNumFreePitchShifts(0),
      mPitchShiftCount(0),
      mSoftVoiceLimit(kDefaultVoiceLimit),
      mPeakVoicesInUse(0),
      mSkipVoiceDecoders(false),
      mSampleRate(sampleRate),
      mVoicesPending(true) {}

// Reconstructed from eboot.elf at 0xA08A0. The clients are told from a copy
// of the list, since each one unregisters itself.
FusionVoicePool::~FusionVoicePool() {
    mCritSec.Enter();
    eastl::list<FusionSampler*> clients(mClients);
    for (FusionSampler* client : clients) {
        client->VoicePoolWillDestruct(this);
    }
    delete[] mVoices;
    mVoices = nullptr;
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA0B00.
void FusionVoicePool::CreatePendingVoices() {
    if (mVoicesPending) {
        _CreateVoices(static_cast<unsigned short>(mHardVoiceLimit));
        _CreatePitchShifters(static_cast<unsigned short>(mPitchShiftCount));
        mVoicesPending = false;
    }
}

// Reconstructed from eboot.elf at 0xA0B40.
void FusionVoicePool::DeferVoiceCreation() {
    if (mVoicesPending) {
        return;
    }
    mVoicesPending = true;
    mCritSec.Enter();
    if (mClients.size() == 0) {
        _DestroyVoices();
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA0B90. The first client creates deferred
// voices.
void FusionVoicePool::AddClient(FusionSampler* sampler) {
    mCritSec.Enter();
    auto it = mClients.begin();
    for (; it != mClients.end(); ++it) {
        if (*it == sampler) {
            break;
        }
    }
    if (it == mClients.end()) {
        mClients.push_back(sampler);
        if (mClients.size() == 1 && mHardVoiceLimit != 0 && mVoicesPending) {
            _CreateVoices(static_cast<unsigned short>(mHardVoiceLimit));
            _CreatePitchShifters(static_cast<unsigned short>(mPitchShiftCount));
        }
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA0C50. The last client frees deferred
// voices.
void FusionVoicePool::RemoveClient(FusionSampler* sampler) {
    mCritSec.Enter();
    auto it = mClients.begin();
    for (; it != mClients.end(); ++it) {
        if (*it == sampler) {
            break;
        }
    }
    if (it != mClients.end()) {
        mClients.erase(it);
        if (mClients.size() == 0 && mVoicesPending) {
            _DestroyVoices();
        }
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA0CF0.
unsigned int FusionVoicePool::GetNumVoicesInUse() {
    mCritSec.Enter();
    unsigned int numInUse = 0;
    for (unsigned int i = 0; i < mNumVoices; ++i) {
        numInUse += mVoices[i].IsInUse();
    }
    if (numInUse > mPeakVoicesInUse) {
        mPeakVoicesInUse = numInUse;
    }
    mCritSec.Exit();
    return numInUse;
}

// Reconstructed from eboot.elf at 0xA0D70.
void FusionVoicePool::SetSoftVoiceLimit(unsigned int limit) {
    mCritSec.Enter();
    unsigned int softLimit = limit != 0 ? limit : 1;
    if (limit > mHardVoiceLimit) {
        softLimit = mHardVoiceLimit;
    }
    mSoftVoiceLimit = softLimit;
    if (mNumVoices != 0) {
        FastReleaseExcessVoices(nullptr);
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA0DE0. For a sampler, every voice it owns
// counts against its limit; otherwise every busy voice counts against the
// soft limit. The excess goes in order of the lowest priority, then the
// largest age.
unsigned int FusionVoicePool::FastReleaseExcessVoices(FusionSampler* sampler) {
    FusionVoice* candidates[kMaxVoiceLimit];
    mCritSec.Enter();
    unsigned int numCandidates = 0;
    if (sampler != nullptr) {
        for (unsigned long i = 0; i < mNumVoices; ++i) {
            if (mVoices[i].mSampler == sampler) {
                candidates[numCandidates++] = &mVoices[i];
            }
        }
    } else {
        for (unsigned int i = 0; i < mNumVoices; ++i) {
            FusionVoice* voice = &mVoices[i];
            if (voice->IsWaitingForAttack() || voice->IsInUse()) {
                candidates[numCandidates++] = voice;
            }
        }
    }

    int limit = sampler != nullptr ? sampler->GetMaxNumVoices() : static_cast<int>(mSoftVoiceLimit);
    int excess = static_cast<int>(numCandidates) - limit;
    while (excess > 0) {
        FusionVoice* victim = nullptr;
        unsigned int victimIndex = 0;
        for (unsigned int i = 0; i < numCandidates; ++i) {
            FusionVoice* voice = candidates[i];
            if (voice == nullptr) {
                continue;
            }
            if (victim == nullptr) {
                victim = voice;
                victimIndex = i;
            }
            unsigned char priority = VoicePriority(voice);
            unsigned char victimPriority = VoicePriority(victim);
            if (priority > victimPriority || (priority == victimPriority && voice->mAge > victim->mAge)) {
                victim = voice;
                victimIndex = i;
            }
        }
        if (victim != nullptr) {
            victim->FastRelease();
            candidates[victimIndex] = nullptr;
            --excess;
        }
    }
    mCritSec.Exit();
    return numCandidates;
}

// Reconstructed from eboot.elf at 0xA1010. Created voices are rebuilt at the
// new limit.
void FusionVoicePool::SetHardVoiceLimit(unsigned int limit) {
    mCritSec.Enter();
    unsigned int hardLimit = limit != 0 ? limit : 1;
    if (hardLimit > kMaxVoiceLimit) {
        hardLimit = kMaxVoiceLimit;
    }
    mHardVoiceLimit = hardLimit;
    if (mSoftVoiceLimit > hardLimit) {
        mSoftVoiceLimit = hardLimit;
    }
    if (mClients.size() != 0 || !mVoicesPending) {
        _CreateVoices(static_cast<unsigned short>(hardLimit));
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA1090. Voices are named 'A' onwards.
void FusionVoicePool::_CreateVoices(unsigned short numVoices) {
    mCritSec.Enter();
    KillVoices();
    mNumVoices = 0;
    mCritSec.Exit();
    delete[] mVoices;
    mVoices = nullptr;
    mVoices = new FusionVoice[numVoices];
    for (unsigned short i = 0; i < numVoices; ++i) {
        mVoices[i].Init(this, static_cast<char>('A' + i), mSkipVoiceDecoders);
    }
    mPeakVoicesInUse = 0;
    mNumVoices = numVoices;
}

// Reconstructed from eboot.elf at 0xA1250.
void FusionVoicePool::SetSkipVoiceDecoders(bool skip) {
    mSkipVoiceDecoders = skip;
}

// Reconstructed from eboot.elf at 0xA1260.
void FusionVoicePool::SetNumPitchShifters(int count) {
    mCritSec.Enter();
    mPitchShiftCount = count;
    if (mClients.size() != 0) {
        _CreatePitchShifters(static_cast<unsigned short>(count));
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA12C0.
void FusionVoicePool::_CreatePitchShifters(unsigned short count) {
    if (mNumPitchShifts == count) {
        return;
    }
    mCritSec.Enter();
    KillVoices();
    mNumPitchShifts = 0;
    mNumFreePitchShifts = 0;
    mCritSec.Exit();
    mPitchShifts.clear();
    if (count == 0) {
        return;
    }
    for (unsigned short remaining = count; remaining != 0; --remaining) {
        float sampleRate = mSampleRate;
        SmbPitchShift* pitchShift = new SmbPitchShift();
        pitchShift->SetSampleRate(sampleRate);
        pitchShift->Setup(kPitchShiftFrameSize, kPitchShiftOversampling);
        mPitchShifts.emplace_back(pitchShift);
    }
    mNumPitchShifts = count;
    mNumFreePitchShifts = count;
}

// Reconstructed from eboot.elf at 0xA14C0.
SmbPitchShift* FusionVoicePool::AcquirePitchShift() {
    if (mNumFreePitchShifts == 0) {
        return nullptr;
    }
    mCritSec.Enter();
    SmbPitchShift* pitchShift = nullptr;
    for (unsigned int i = 0; i < mNumPitchShifts; ++i) {
        PitchShiftSlot& slot = mPitchShifts[i];
        if (!slot.mInUse) {
            --mNumFreePitchShifts;
            slot.mInUse = true;
            pitchShift = slot.mPitchShift;
            break;
        }
    }
    mCritSec.Exit();
    return pitchShift;
}

// Reconstructed from eboot.elf at 0xA1540.
void FusionVoicePool::ReleasePitchShift(SmbPitchShift* pitchShift) {
    mCritSec.Enter();
    for (unsigned int i = 0; i < mNumPitchShifts; ++i) {
        PitchShiftSlot& slot = mPitchShifts[i];
        if (slot.mPitchShift == pitchShift) {
            if (slot.mInUse) {
                ++mNumFreePitchShifts;
                slot.mInUse = false;
            }
            break;
        }
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA15C0. A voice whose keyzone ranks zero,
// or below the new keyzone, is never stolen. Otherwise the candidate is the
// voice of the lowest priority, then a released voice before a held one,
// then the largest age, then the quieter envelope.
FusionVoice* FusionVoicePool::GetFreeVoice(
    const FusionSampler* sampler,
    unsigned int id,
    const FusionPatchCom::KeyzoneSettings* keyzone) {
    mCritSec.Enter();
    FusionVoice* result = nullptr;
    if (mNumVoices == 0) {
        mCritSec.Exit();
        return nullptr;
    }
    if (keyzone->mMaintainTime ? sampler == nullptr || mNumPitchShifts == 0 : sampler == nullptr) {
        mCritSec.Exit();
        return nullptr;
    }
    if (!keyzone->mSample || keyzone->mSample->Fail() || keyzone->mSample->GetAudioData()->IsEmpty()) {
        mCritSec.Exit();
        return nullptr;
    }

    FusionVoice* chosen = &mVoices[0];
    bool allBusy = true;
    unsigned int i = 0;
    for (; i < mNumVoices; ++i) {
        FusionVoice* voice = &mVoices[i];
        if (voice->IsWaitingForAttack()) {
            continue;
        }
        if (voice->MatchesIDs(sampler, id, keyzone) && voice->IsInUse()) {
            voice->Release();
        }
        if (!voice->IsInUse() && !voice->IsWaitingForAttack()) {
            chosen = voice;
            allBusy = false;
            break;
        }

        const FusionPatchCom::KeyzoneSettings* voiceKeyzone = voice->mKeyzone;
        if (voiceKeyzone != nullptr &&
            (voiceKeyzone->mPriority == 0 || voiceKeyzone->mPriority < keyzone->mPriority)) {
            continue;
        }
        unsigned char chosenPriority = VoicePriority(chosen);
        unsigned char voicePriority = VoicePriority(voice);
        if (chosenPriority > voicePriority) {
            continue;
        }
        if (chosenPriority < voicePriority) {
            chosen = voice;
            continue;
        }
        bool chosenReleasing = chosen->mState == FusionVoice::kStateReleasing;
        bool voiceReleasing = voice->mState == FusionVoice::kStateReleasing;
        if (chosenReleasing != voiceReleasing) {
            if (voiceReleasing) {
                chosen = voice;
            }
            continue;
        }
        if (voice->mAge > chosen->mAge) {
            chosen = voice;
            continue;
        }
        if (voice->mAge != chosen->mAge) {
            continue;
        }
        float voiceLevel = voice->mLevels[1] > voice->mLevels[0] ? voice->mLevels[1] : voice->mLevels[0];
        float chosenLevel = chosen->mLevels[1] > chosen->mLevels[0] ? chosen->mLevels[1] : chosen->mLevels[0];
        if (voiceLevel < chosenLevel) {
            chosen = voice;
        }
    }
    for (++i; i < mNumVoices; ++i) {
        FusionVoice* voice = &mVoices[i];
        if (voice->MatchesIDs(sampler, id, keyzone) && voice->IsInUse()) {
            voice->Release();
        }
    }

    if (chosen->IsWaitingForAttack()) {
        mCritSec.Exit();
        return nullptr;
    }
    if (allBusy && chosen->mKeyzone != nullptr &&
        (chosen->mKeyzone->mPriority < keyzone->mPriority || chosen->mKeyzone->mPriority == 0)) {
        mCritSec.Exit();
        return nullptr;
    }
    if (keyzone->mMaintainTime && chosen->mPitchShift == nullptr && mNumFreePitchShifts == 0) {
        mCritSec.Exit();
        return nullptr;
    }

    chosen->Kill();
    SmbPitchShift* pitchShift = nullptr;
    if (keyzone->mMaintainTime) {
        mCritSec.Enter();
        for (unsigned int slot = 0; slot < mNumPitchShifts; ++slot) {
            if (!mPitchShifts[slot].mInUse) {
                --mNumFreePitchShifts;
                mPitchShifts[slot].mInUse = true;
                pitchShift = mPitchShifts[slot].mPitchShift;
                break;
            }
        }
        mCritSec.Exit();
        pitchShift->SetShift(keyzone->mShiftCoarse, keyzone->mShiftFine);
    }
    if (chosen->AssignIDs(sampler, keyzone, id, pitchShift)) {
        result = chosen;
    }
    mCritSec.Exit();
    return result;
}

// Reconstructed from eboot.elf at 0xA1990. Unlocked; the scan runs to the
// hard limit.
bool FusionVoicePool::IsPlaying(const FusionSampler* sampler, unsigned char id) {
    for (unsigned int i = 0; i < mHardVoiceLimit; ++i) {
        FusionVoice* voice = &mVoices[i];
        if (voice->IsInUse() && voice->MatchesIDs(sampler, id, nullptr)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0xA1A10.
void FusionVoicePool::KillVoices(const FusionSampler* sampler) {
    mCritSec.Enter();
    for (unsigned int i = 0; i < mNumVoices; ++i) {
        if (mVoices[i].mSampler == sampler) {
            mVoices[i].Kill();
        }
    }
    mCritSec.Exit();
}

// Reconstructed from eboot.elf at 0xA1AA0.
void FusionVoicePool::KillVoices(const FusionPatchCom::KeyzoneSettings* keyzone) {
    mCritSec.Enter();
    for (unsigned int i = 0; i < mNumVoices; ++i) {
        if (mVoices[i].mKeyzone == keyzone) {
            mVoices[i].Kill();
        }
    }
    mCritSec.Exit();
}
