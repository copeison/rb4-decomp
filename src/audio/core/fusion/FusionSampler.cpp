#include "audio/core/fusion/FusionSampler.h"

#include "audio/core/fusion/FusionVoicePool.h"
#include "os/threading/CritSec.h"

// Reconstructed from eboot.elf at 0x972C0. The pool's rate becomes the
// sampler's.
void FusionSampler::SetVoicePool(FusionVoicePool* pool) {
    if (mVoicePool == pool) {
        return;
    }
    ScopedCritSecPtr lock(GetBusLock());
    if (pool != nullptr) {
        pool->AddClient(this);
    }
    if (mVoicePool != nullptr) {
        mVoicePool->KillVoices(this);
        mVoicePool->RemoveClient(this);
    }
    mVoicePool = pool;
    if (pool != nullptr) {
        SetSampleRate(pool->mSampleRate);
    }
}

// Reconstructed from eboot.elf at 0x973C0. SetVoicePool(nullptr) is inlined
// there.
void FusionSampler::VoicePoolWillDestruct(const FusionVoicePool* pool) {
    if (mVoicePool == pool) {
        ScopedCritSecPtr lock(GetBusLock());
        SetVoicePool(nullptr);
    }
}

// Reconstructed from eboot.elf at 0x979D0.
void FusionSampler::SetBeat(float beat) {
    ScopedCritSecPtr lock(GetBusLock());
    mBeat = beat;
}

// Reconstructed from eboot.elf at 0x9A420. A portamento instrument plays one
// note at a time.
int FusionSampler::GetMaxNumVoices() const {
    return mPortamentoEnabled ? 1 : mMaxNumVoices;
}

// Reconstructed from eboot.elf at 0x9A440.
FusionSampler::PortamentoMode FusionSampler::_GetPortamentoMode() const {
    return mPortamentoMode;
}

// Reconstructed from eboot.elf at 0x9A450.
float FusionSampler::_GetCurrentPortamentoPitch() const {
    return mPortamentoPitch.mValue;
}

// Reconstructed from eboot.elf at 0x9A460.
float FusionSampler::_GetPan() const {
    return mPan;
}

// Reconstructed from eboot.elf at 0x9A470.
float FusionSampler::_GetFineTuneCents() const {
    return mFineTuneCents;
}

// Reconstructed from eboot.elf at 0x9A480. The bend being ramped to.
float FusionSampler::GetPitchBend(signed char) const {
    return mPitchBend.mTarget;
}

// Reconstructed from eboot.elf at 0x9A5E0.
float FusionSampler::_GetPitchBendFactor() const {
    return mPitchBendFactor;
}

// Reconstructed from eboot.elf at 0x9A5F0. The channel is ignored.
void FusionSampler::SetExtraPitchBend(float bend, signed char) {
    mExtraPitchBend = bend;
}

// Reconstructed from eboot.elf at 0x9A8D0. The channel is ignored.
float FusionSampler::GetMidiChannelGain(signed char) const {
    return mChannelGain.mValue;
}

// Reconstructed from eboot.elf at 0x9A8E0. Muting ramps the voices' gain to
// zero. The channel is ignored.
void FusionSampler::SetMidiChannelMute(bool mute, signed char) {
    if (mute == mChannelMute) {
        return;
    }
    ScopedCritSecPtr lock(GetBusLock());
    mChannelMute = mute;
    mMuteGain.SetTarget(mute ? 0.0f : 1.0f, nullptr, nullptr);
}

// Reconstructed from eboot.elf at 0x9A9A0.
float FusionSampler::_GetTrimVolume() const {
    return mTrimVolume;
}

// Reconstructed from eboot.elf at 0x9A9B0.
float FusionSampler::_GetTrimGain() const {
    return mTrimGain;
}

// Reconstructed from eboot.elf at 0x9A9C0.
float FusionSampler::_GetStartPointMs() const {
    return mStartPointMs;
}
