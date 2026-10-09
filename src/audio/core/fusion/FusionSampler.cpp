#include "audio/core/fusion/FusionSampler.h"

#include <cmath>
#include <cstring>

#include "audio/core/fusion/FusionVoice.h"
#include "audio/core/fusion/FusionVoicePool.h"
#include "audio/core/output/AudioRenderTarget.h"
#include "audio/core/system/SoundManager.h"
#include "audio/fmod/system/FmodPlatform.h"
#include "math/random/Rand.h"
#include "os/threading/CritSec.h"

// The globals at 0x19C8EB0 through 0x19C8FB0, which the static initializer
// at 0x9D6F0 sets up.
CritSec FusionSampler::sNoteActionCritSec;  // 0x19C8EB0

namespace {

// The specs of the sampler's parameters, as minimum, default and maximum.
// The volume spec's default lies above its maximum, so a reset clamps it to
// 0 dB. Names not in the reference map.
SPL::ParameterSpec sPanSpec = {-1.0f, 0.0f, 1.0f};                  // 0x19C8EC0
SPL::ParameterSpec sExpressionSpec = {0.0f, 1.0f, 1.0f};            // 0x19C8ECC
SPL::ParameterSpec sMinPitchBendCentsSpec = {-2400.0f, -200.0f, -10.0f};  // 0x19C8ED8
SPL::ParameterSpec sMaxPitchBendCentsSpec = {10.0f, 200.0f, 2400.0f};     // 0x19C8EE4
SPL::ParameterSpec sMaxNumVoicesSpec = {1.0f, 100.0f, 100.0f};      // 0x19C8EF0
SPL::ParameterSpec sVolumeDbSpec = {-96.0f, 12.0f, 0.0f};           // 0x19C8EFC
SPL::ParameterSpec sPortamentoTimeSpec = {0.0f, 300.0f, 10000.0f};  // 0x19C8F08, ms
SPL::ParameterSpec sStartPointMsSpec = {0.0f, 0.0f, 1000.0f};       // 0x19C8F14
SPL::ParameterSpec sFineTuneCentsSpec = {-100.0f, 0.0f, 100.0f};    // 0x19C8F20

// The curves the controllers map their 0-1 values through, set up by the
// first constructor. Names not in the reference map.
ExpInterpolator sFilterFrequencyInterpolator;  // 0x19C8F30, controller 74
ExpInterpolator sFilterQInterpolator;          // 0x19C8F50, controller 71
ExpInterpolator sLFOFrequencyInterpolator;     // 0x19C8F70, controllers 22-23
ExpInterpolator sDelayTimeInterpolator;        // 0x19C8F90, controller 26
bool sInterpolatorsReset;                      // 0x19C8FB0

// A MIDI data byte as a fraction of its maximum.
constexpr float kMidiValueScale = 1.0f / 127.0f;
// The gain of a MIDI volume or expression value: its square, or -96 dB for
// zero. Inlined into SetController. Name not in the reference map.
float MidiValueToGain(signed char value) {
    float exponent = value != 0 ? 2.0f * log10f(value * kMidiValueScale) : -4.8f;
    return powf(10.0f, exponent);
}

// The Hmx::Timer start and stop sequences the sampler inlines on its render
// target's CPU timer. Names not in the reference map.
void StartCpuTimer(AudioCpuTimer* timer) {
    if (timer != nullptr && timer->mRunning >= 0 && timer->mRunning++ == 0) {
        timer->mStartCycles = __builtin_ia32_rdtsc();
    }
}
void StopCpuTimer(AudioCpuTimer* timer) {
    if (timer != nullptr && timer->mRunning > 0 && --timer->mRunning == 0) {
        timer->mElapsedCycles += __builtin_ia32_rdtsc() - timer->mStartCycles;
    }
}

// Runs one band of the amp EQ over mono samples (direct form II). Inlined
// into Process. Name not in the reference map.
void FilterAmpEqBand(float* samples, int numFrames, const BiquadFilter::Coefs& coefs, double* history) {
    double w1 = history[0];
    double w2 = history[1];
    for (int i = 0; i < numFrames; ++i) {
        double w = static_cast<double>(samples[i]) - w1 * coefs.mA1 - w2 * coefs.mA2;
        samples[i] = static_cast<float>(w2 * coefs.mB2 + w1 * coefs.mB1 + w * coefs.mB0);
        w2 = w1;
        w1 = w;
    }
    history[0] = w1;
    history[1] = w2;
}

// Lets each slave render a block into the scratch view and adds what it
// made. Inlined twice into Process. Name not in the reference map.
void ProcessSlaves(
    eastl::vector<InstrumentGenerator*>& slaves, AudioBuffer<float>& buffer, AudioBuffer<float>& scratch) {
    for (unsigned long i = 0; i < slaves.size(); ++i) {
        slaves[i]->Process(scratch);
        if (!scratch.mCleared) {
            buffer.Accumulate(scratch.mChannelData, buffer.mNumChannels, buffer.mNumFrames);
            buffer.mCleared = false;
        }
    }
}

}  // namespace

// Reconstructed from eboot.elf at 0x95C40.
FusionSampler::FusionSampler()
    : InstrumentGenerator(nullptr),
      mStartPointTarget(
          ModulatorTarget::kStartPointStr,
          sStartPointMsSpec.mMax,
          "ms",
          ModulatorTarget::kDepthModeSubtractive,
          &mStartPointModulation),
      mPitchTarget(ModulatorTarget::kPitchStr, 100.0f, "cents", ModulatorTarget::kDepthModeCentered, &mPitchModulation),
      mControlRate(1.0f),
      mSpeed(1.0f),
      mBeat(0.0f),
      mTimeStretchAlgorithm(0),
      mTimeStretchFormantMode(0),
      mChannelVolume(sVolumeDbSpec),
      mChannelMute(false),
      mTrimVolume(sVolumeDbSpec),
      mExpression(sExpressionSpec),
      mPan(sPanSpec),
      mPortamentoEnabled(false),
      mPortamentoMode(kPortamentoLegato),
      mPortamentoTimeMs(sPortamentoTimeSpec),
      mPortamentoActive(false),
      mExtraPitchBend(0.0f),
      mPitchBendFactor(1.0f),
      mFineTuneCents(sFineTuneCentsSpec),
      mStartPointMs(sStartPointMsSpec),
      mADSRSettings(2),
      mLFOSettings(2),
      mLFOs(2),
      mRandomModulators(2),
      mVelocityModulators(2),
      mMaxNumVoices(static_cast<int>(sMaxNumVoicesSpec.mDefault)),
      mNumActiveVoices(0),
      mMinPitchBendCents(sMinPitchBendCentsSpec),
      mMaxPitchBendCents(sMaxPitchBendCentsSpec),
      mVoicePool(nullptr),
      mDelayEnabled(false),
      mBitCrusherEnabled(false),
      mDistortionEnabled(false),
      mAmpEqHistory(),
      mPatchCom(nullptr),
      mTempo(120.0f),
      mPlayScale(1.0f),
      mTranspose(0),
      mPresetIndex(0),
      mKeyzoneSelectMode(kSelectLayers),
      mCpuTimer(nullptr) {
    mNoteActionsPending = false;
    if (gAudioRenderTargets.mDefault != nullptr) {
        mCpuTimer = static_cast<AudioCpuTimer*>(
            gAudioRenderTargets.mDefault->GetTimer(reinterpret_cast<unsigned long>(Symbol("fusion").Str())));
    }
    mScratchChannels[0] = mScratch[0];
    mScratchChannels[1] = mScratch[1];
    SetSampleRate(0.0f);
    if (!sInterpolatorsReset) {
        sInterpolatorsReset = true;
        sFilterFrequencyInterpolator.Reset(Vector2{0.0f, 20.0f}, Vector2{1.0f, 20000.0f}, 4.335f);
        sFilterQInterpolator.Reset(Vector2{0.0f, 0.1f}, Vector2{1.0f, 10.0f}, 4.03f);
        sLFOFrequencyInterpolator.Reset(Vector2{0.0f, 0.1f}, Vector2{1.0f, 30.0f}, 2.74f);
        sDelayTimeInterpolator.Reset(Vector2{0.0f, 0.0001f}, Vector2{1.0f, 4.0f}, 2.0f);
    }
    ResetInstrumentState();
    ResetPatchRelatedState();
}

// Reconstructed from eboot.elf at 0x96610.
void FusionSampler::ResetPatchRelatedState() {
    ScopedCritSecPtr lock(GetBusLock());
    _ResetNoteActions();
    _SetPan(mPan.mSpec.mDefault);
    _SetTrimVolume(sVolumeDbSpec.mDefault);
    _SetMidiExpressionGain(mExpression.mSpec.mDefault, 0.0f, 0);
    SetPitchBend(0.0f, 0);
    mPitchBend.Finish();
    _SetMaxPitchBendCents(200.0f);
    _SetMinPitchBendCents(-200.0f);
    mFilterSettings = BiquadFilter::Settings();
    for (ADSR::Settings& settings : mADSRSettings) {
        settings = ADSR::Settings();
    }
    mADSRSettings[0].mTarget = ADSR::Settings::kTargetVolume;
    mADSRSettings[0].mEnabled = true;
    mADSRSettings[1].mTarget = ADSR::Settings::kTargetNone;
    mADSRSettings[1].mEnabled = false;
    for (LFO::Settings& settings : mLFOSettings) {
        settings = LFO::Settings();
    }
    mLFOSettings[0].mTarget = LFO::Settings::kTargetPan;
    mLFOSettings[1].mTarget = LFO::Settings::kTargetPitch;
    for (Modulator& modulator : mRandomModulators) {
        modulator.Reset();
    }
    mRandomModulators[0].SetTarget(&ModulatorTarget::kDummyTarget);
    mRandomModulators[1].SetTarget(&ModulatorTarget::kDummyTarget);
    for (Modulator& modulator : mVelocityModulators) {
        modulator.Reset();
    }
    mVelocityModulators[0].SetTarget(&ModulatorTarget::kDummyTarget);
    mVelocityModulators[1].SetTarget(&ModulatorTarget::kDummyTarget);
    mMaxNumVoices = static_cast<int>(sMaxNumVoicesSpec.mDefault);
    _SetPortamentoIsEnabled(false);
    // Middle C, without a glide.
    mPortamentoPitch.SetTarget(60.0f, nullptr, nullptr);
    mPortamentoPitch.Finish();
    _SetPortamentoMode(kPortamentoLegato);
    _SetPortamentoTimeMs(sPortamentoTimeSpec.mDefault);
    _SetDelayEnabled(false);
    mBitCrusherEnabled = false;
    mDistortionEnabled = false;
    std::memset(mLastNoteOnZones, -1, sizeof(mLastNoteOnZones));
    std::memset(mLastNoteOffZones, -1, sizeof(mLastNoteOffZones));
    std::memset(mHeldVelocities, 0, sizeof(mHeldVelocities));
    // No model: the amp passes the signal through.
    mAmpSimulator.SetModel(5);
}

// Reconstructed from eboot.elf at 0x96E20.
FusionSampler::~FusionSampler() {
    _DumpAllInstrumentSlaves();
    if (mVoicePool != nullptr) {
        ScopedCritSecPtr lock(GetBusLock());
        if (mVoicePool != nullptr) {
            mVoicePool->KillVoices(this);
            mVoicePool->RemoveClient(this);
        }
        mVoicePool = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x97190.
void FusionSampler::_DumpAllInstrumentSlaves() {
    ScopedCritSec lock(mInstrumentLock);
    for (unsigned long i = 0; i < mSlaveHandles.size(); ++i) {
        AudioGenerator* generator = theSoundManager.LockIfOwned(mSlaveHandles[i]);
        if (generator == nullptr) {
            continue;
        }
        if (generator->IsInstrument()) {
            InstrumentGenerator* slave = static_cast<InstrumentGenerator*>(generator);
            if (slave != nullptr) {
                slave->ClearGeneratorFlag();
                --slave->mRefCount;
            }
        } else {
            --generator->mRefCount;
        }
    }
    mPreEffectSlaves.clear();
    mPostEffectSlaves.clear();
    mSlaveHandles.clear();
}

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

// Reconstructed from eboot.elf at 0x974A0. The channel is ignored.
void FusionSampler::NoteOn(signed char note, signed char velocity, signed char, float startOffsetMs) {
    ScopedCritSec lock(sNoteActionCritSec);
    mNoteActionsPending = true;
    NoteAction& action = mNoteActions[note];
    if (velocity == 0 || action.mVelocity < velocity) {
        action.mVelocity = velocity;
        action.mStartOffsetMs = startOffsetMs;
    }
}

// Reconstructed from eboot.elf at 0x97530.
bool FusionSampler::IsNotePlaying(signed char note) {
    if (mVoicePool == nullptr) {
        return false;
    }
    signed char velocity;
    {
        ScopedCritSec lock(sNoteActionCritSec);
        velocity = mNoteActions[note].mVelocity;
    }
    if (velocity > 0) {
        return true;
    }
    if (mNumActiveVoices == 0) {
        return false;
    }
    return mVoicePool->IsPlaying(this, static_cast<unsigned char>(note));
}

// Reconstructed from eboot.elf at 0x975D0: a note-on of zero velocity. The
// channel is ignored.
void FusionSampler::NoteOff(signed char note, signed char) {
    NoteOn(note, 0, 0, 0.0f);
}

// Reconstructed from eboot.elf at 0x975F0.
void FusionSampler::SetSpeed(float speed, bool) {
    ScopedCritSecPtr lock(GetBusLock());
    mSpeed = 0.0f < speed ? speed : 0.0001f;
    _SetPortamentoTimeMs(mPortamentoTimeMs.mValue);
    mDelay.SetSpeed(mSpeed);
    mDelay.SetDelaySeconds(mDelay.mDelayTime);
}

// Reconstructed from eboot.elf at 0x97760.
void FusionSampler::_SetPortamentoTime(float seconds) {
    _SetPortamentoTimeMs(seconds * 1000.0f);
}

// Reconstructed from eboot.elf at 0x97840.
float FusionSampler::_GetPortamentoTime() const {
    return mPortamentoTimeMs.mValue * 0.001f;
}

// Reconstructed from eboot.elf at 0x97870. The LFO settings take the tempo
// as their default.
void FusionSampler::SetTempo(float tempo) {
    if (tempo == mTempo) {
        return;
    }
    mTempo = tempo;
    for (LFO::Settings& settings : mLFOSettings) {
        settings.mDefaultTempo = tempo;
    }
    mLFOs[0].UseSettings(&mLFOSettings[0]);
    mLFOs[1].UseSettings(&mLFOSettings[1]);
    _UpdateVoiceLFOS();
    mDelay.SetTempo(tempo);
}

// Reconstructed from eboot.elf at 0x97920. The pool is not checked.
void FusionSampler::_UpdateVoiceLFOS() {
    ScopedCritSecPtr lock(GetBusLock());
    for (unsigned int i = 0; i < mVoicePool->mNumVoices; ++i) {
        FusionVoice& voice = mVoicePool->mVoices[i];
        if (voice.mSampler == this) {
            voice.SetupLFO(0, mLFOSettings[0]);
            voice.SetupLFO(1, mLFOSettings[1]);
        }
    }
}

// Reconstructed from eboot.elf at 0x979D0.
void FusionSampler::SetBeat(float beat) {
    ScopedCritSecPtr lock(GetBusLock());
    mBeat = beat;
}

// Reconstructed from eboot.elf at 0x97A20.
void FusionSampler::_ResetNoteActions() {
    ScopedCritSec lock(sNoteActionCritSec);
    for (NoteAction& action : mNoteActions) {
        action.mVelocity = -1;
        action.mStartOffsetMs = 0.0f;
    }
    mNoteActionsPending = false;
}

// Reconstructed from eboot.elf at 0x97A90. Takes the queued notes, binds
// the render target's pool on first use, and plays them from the highest
// note down. Note-ons stop once the voice limit is spent; a note-off also
// plays the note-off keyzones at the held velocity.
void FusionSampler::_ProcessNoteActions() {
    if (!mNoteActionsPending) {
        return;
    }
    NoteAction actions[128];
    {
        ScopedCritSec lock(sNoteActionCritSec);
        std::memcpy(actions, mNoteActions, sizeof(actions));
        _ResetNoteActions();
    }
    if (mVoicePool == nullptr && mRenderTarget != nullptr) {
        SetVoicePool(mRenderTarget->GetVoicePool());
    }
    int remaining = mMaxNumVoices;
    for (int note = 127; note >= 0; --note) {
        signed char velocity = actions[note].mVelocity;
        if (velocity == -1) {
            continue;
        }
        if (velocity != 0) {
            if (remaining > 0) {
                _ProcessNoteOn(note, velocity, true, actions[note].mStartOffsetMs);
                --remaining;
                mHeldVelocities[note] = velocity;
            }
        } else {
            _ProcessNoteOff(note);
            signed char held = mHeldVelocities[note];
            if (held > 0) {
                _ProcessNoteOn(note, held, false, 0.0f);
                --remaining;
            }
            mHeldVelocities[note] = 0;
        }
    }
}

// Reconstructed from eboot.elf at 0x97CB0. Voices already releasing are
// skipped; a legato glide ends with the note.
void FusionSampler::_ProcessNoteOff(unsigned char note) {
    if (mVoicePool == nullptr) {
        return;
    }
    for (unsigned int i = 0; i < mVoicePool->mNumVoices; ++i) {
        FusionVoice& voice = mVoicePool->mVoices[i];
        ADSR::State::Stage stage = voice.mAmpEnvelope.mStage;
        if (stage != ADSR::State::kStageRelease && stage != ADSR::State::kStageOff &&
            voice.MatchesIDs(this, note, nullptr)) {
            voice.Release();
            if (mPortamentoMode == kPortamentoLegato) {
                mPortamentoActive = false;
            }
        }
    }
}

// Reconstructed from eboot.elf at 0x97D50. The note, moved by the
// transpose, selects the keyzones; the selection mode picks all of them or
// one, by weight or in turn. The random modes skip zones of zero weight.
// The pool then sheds the voices over its limits, and the remaining count
// becomes the sampler's.
void FusionSampler::_ProcessNoteOn(unsigned char note, unsigned char velocity, bool noteOn, float startOffsetMs) {
    if (mVoicePool == nullptr) {
        return;
    }
    if (mPatchCom != nullptr) {
        signed char* lastZones = noteOn ? mLastNoteOnZones : mLastNoteOffZones;
        int numZones = static_cast<int>(mPatchCom->mNumKeyzones);
        if (numZones > 0) {
            const FusionPatchCom::KeyzoneSettings* zones = mPatchCom->mKeyzones;
            unsigned char key = static_cast<unsigned char>(note + mTranspose);
            unsigned short candidates[128];
            int numCandidates = 0;
            for (int i = 0; i < numZones; ++i) {
                const FusionPatchCom::KeyzoneSettings& zone = zones[i];
                if (!zone.mSample || !zone.ContainsNoteAndVelocity(key, velocity) ||
                    noteOn != !zone.mTriggerOnNoteOff) {
                    continue;
                }
                if ((mKeyzoneSelectMode == kSelectRandom || mKeyzoneSelectMode == kSelectRandomWithRepetition) &&
                    zone.mRandomWeight == 0.0f) {
                    continue;
                }
                candidates[numCandidates++] = static_cast<unsigned short>(i);
            }
            if (numCandidates == 1 || (numCandidates > 1 && mKeyzoneSelectMode == kSelectLayers)) {
                for (int i = 0; i < numCandidates; ++i) {
                    _TryKeyOnZone(note, key, velocity, startOffsetMs, &mPatchCom->mKeyzones[candidates[i]]);
                }
            } else if (numCandidates > 1) {
                int chosen = -1;
                if (mKeyzoneSelectMode == kSelectCycle) {
                    chosen = (lastZones[note] + 1) % static_cast<unsigned int>(numCandidates);
                } else if (mKeyzoneSelectMode == kSelectRandom || mKeyzoneSelectMode == kSelectRandomWithRepetition) {
                    long excluded = mKeyzoneSelectMode == kSelectRandom ? lastZones[note] : -1;
                    float total = 0.0f;
                    for (int i = 0; i < numCandidates; ++i) {
                        if (i != excluded) {
                            total += mPatchCom->mKeyzones[candidates[i]].mRandomWeight;
                        }
                    }
                    float pick = gRand.Float() * total;
                    chosen = 0;
                    for (int i = 0; i < numCandidates; ++i) {
                        if (i == excluded) {
                            continue;
                        }
                        pick -= mPatchCom->mKeyzones[candidates[i]].mRandomWeight;
                        chosen = i;
                        if (pick < 0.0f) {
                            break;
                        }
                    }
                }
                if (chosen >= 0) {
                    lastZones[note] = static_cast<signed char>(chosen);
                    _TryKeyOnZone(note, key, velocity, startOffsetMs, &mPatchCom->mKeyzones[candidates[chosen]]);
                }
            }
        }
    }
    mVoicePool->FastReleaseExcessVoices(nullptr);
    unsigned int numVoices = mVoicePool->FastReleaseExcessVoices(this);
    mNumActiveVoices = static_cast<int>(numVoices);
    mVoiceMeter.Update(static_cast<int>(numVoices));
}

// Reconstructed from eboot.elf at 0x981F0. The random modes skip zones of
// zero weight.
int FusionSampler::_FindKeyzones(
    unsigned char key,
    unsigned char velocity,
    bool noteOn,
    const FusionKeyzoneArray& keyzones,
    unsigned short* indices) {
    int count = 0;
    int numZones = static_cast<int>(keyzones.mSize);
    for (int i = 0; i < numZones; ++i) {
        const FusionPatchCom::KeyzoneSettings& zone = keyzones.mData[i];
        if (!zone.mSample || !zone.ContainsNoteAndVelocity(key, velocity) || noteOn != !zone.mTriggerOnNoteOff) {
            continue;
        }
        if ((mKeyzoneSelectMode == kSelectRandom || mKeyzoneSelectMode == kSelectRandomWithRepetition) &&
            zone.mRandomWeight == 0.0f) {
            continue;
        }
        indices[count++] = static_cast<unsigned short>(i);
    }
    return count;
}

// Reconstructed from eboot.elf at 0x982D0. The modulators feed the start
// offset and the pitch offset: the random ones a value in [0, 1) and the
// velocity ones the velocity. Velocity scales the gain only when the zone
// asks for it. A portamento note glides from the last one, or starts the
// glide at its own pitch.
bool FusionSampler::_TryKeyOnZone(
    unsigned char note,
    unsigned char key,
    unsigned char velocity,
    float startOffsetMs,
    const FusionPatchCom::KeyzoneSettings* keyzone) {
    FusionVoice* voice = mVoicePool->GetFreeVoice(this, note, keyzone);
    if (voice == nullptr) {
        return false;
    }
    mStartPointModulation = 0.0f;
    mPitchModulation = 0.0f;
    for (Modulator& modulator : mRandomModulators) {
        modulator.Modulate(gRand.Float());
    }
    float velocityValue = velocity * kMidiValueScale;
    for (Modulator& modulator : mVelocityModulators) {
        modulator.Modulate(velocityValue);
    }
    float gain = keyzone->mVelocityToVolume ? velocityValue : 1.0f;
    if (mPortamentoEnabled) {
        mPortamentoPitch.SetTarget(static_cast<float>(key), nullptr, nullptr);
        if (!mPortamentoActive) {
            mPortamentoPitch.Finish();
        }
        mPortamentoActive = true;
    }
    double startMs = static_cast<double>(mStartPointMs.mValue) + static_cast<double>(startOffsetMs) +
        static_cast<double>(mStartPointModulation);
    if (startMs < 0.0) {
        startMs = 0.0;
    }
    voice->SetPitchOffset(static_cast<double>(mPitchModulation + mFineTuneCents.mValue));
    voice->AttackWithTargetNote(key, gain, startMs);
    return true;
}

// Reconstructed from eboot.elf at 0x98570.
void FusionSampler::KillAllVoices() {
    ScopedCritSecPtr lock(GetBusLock());
    _ResetNoteActions();
    if (mVoicePool != nullptr) {
        mVoicePool->KillVoices(this);
    }
    mNumActiveVoices = 0;
}

// Reconstructed from eboot.elf at 0x98630. Note 127 is left out.
void FusionSampler::AllNotesOff() {
    ScopedCritSecPtr lock(GetBusLock());
    for (int note = 0; note != 127; ++note) {
        NoteOff(static_cast<signed char>(note), 0);
    }
}

// Reconstructed from eboot.elf at 0x98690.
void FusionSampler::_ApplyModulatorSettings(const Modulator::Settings* settings, Modulator& modulator) {
    modulator.SetTarget(_GetModulatorTargetByType(settings->mTarget));
    modulator.SetRangeMagnitude(settings->mRange);
    modulator.SetDepth(settings->mDepth);
}

// Reconstructed from eboot.elf at 0x986F0.
ModulatorTarget* FusionSampler::_GetModulatorTargetByType(ModulatorTarget::Target target) {
    if (target == ModulatorTarget::kTargetPitch) {
        return &mPitchTarget;
    }
    if (target == ModulatorTarget::kTargetStartPoint) {
        return &mStartPointTarget;
    }
    return &ModulatorTarget::kDummyTarget;
}

// Reconstructed from eboot.elf at 0x98720.
void FusionSampler::LoadPatch(const ResourcePtr<FusionPatchResource>& patch) {
    ScopedCritSecPtr lock(GetBusLock());
    mPatch = patch.Get();
    ResetPatchRelatedState();
    if (mPatch && !mPatch->Fail()) {
        mPatchCom = mPatch->GetPatch();
        mPresetIndex = mPatchCom->mCurrentPreset;
        _LoadPreset();
    } else {
        mPatchCom = nullptr;
    }
}

// Reconstructed from eboot.elf at 0x987E0. The patch component is not
// checked. Only the filter's type, switch, frequency and Q are taken.
void FusionSampler::_LoadPreset() {
    unsigned long lastPreset = static_cast<unsigned long>(mPatchCom->mNumPresets) - 1;
    if (mPresetIndex > lastPreset) {
        mPresetIndex = lastPreset;
    }
    const FusionPatchCom::PresetSettings& preset = mPatchCom->mPresets[mPresetIndex];
    _SetTrimVolume(preset.mVolume);
    _SetMinPitchBendCents(preset.mMinPitchBendCents);
    _SetMaxPitchBendCents(preset.mMaxPitchBendCents);
    _SetMaxNumVoices(preset.mMaxNumVoices);
    _SetStartPointMs(preset.mStartPointMs);
    _SetFineTuneCents(preset.mFineTuneCents);
    _SetPan(preset.mPan);
    mKeyzoneSelectMode = static_cast<KeyzoneSelectMode>(preset.mKeyzoneSelectMode);
    mADSRSettings[0] = preset.mADSRs[0];
    mADSRSettings[1] = preset.mADSRs[1];
    mFilterSettings.mFrequency = preset.mFilter.mFrequency;
    mFilterSettings.mQ = preset.mFilter.mQ;
    mFilterSettings.mType = preset.mFilter.mType;
    mFilterSettings.mEnabled = preset.mFilter.mEnabled;
    mLFOSettings[0] = preset.mLFOs[0];
    mLFOSettings[1] = preset.mLFOs[1];
    _ApplyModulatorSettings(&preset.mModulators[0], mRandomModulators[0]);
    _ApplyModulatorSettings(&preset.mModulators[1], mRandomModulators[1]);
    _ApplyModulatorSettings(&preset.mModulators[2], mVelocityModulators[0]);
    _ApplyModulatorSettings(&preset.mModulators[3], mVelocityModulators[1]);
    _SetPortamentoIsEnabled(preset.mPortamento.mEnabled);
    _SetPortamentoMode(static_cast<PortamentoMode>(preset.mPortamento.mMode));
    _SetPortamentoTime(preset.mPortamento.mTime);

    _SetDelayEnabled(preset.mDelayEnabled);
    mDelay.SetBeatSync(preset.mDelayBeatSync);
    mDelay.SetDelaySeconds(preset.mDelayTime);
    mDelay.SetDryGain(preset.mDelayDryGain);
    mDelay.SetWetGain(preset.mDelayWetGain);
    mDelay.SetFeedbackGain(preset.mDelayFeedbackGain);
    mDelay.FinishGainRamps();

    _SetBitCrusherEnabled(preset.mBitCrusherEnabled);
    mBitCrusher.ClearHistory();
    mBitCrusher.SetWet(preset.mBitCrusherWet, false);
    mBitCrusher.SetCrush(preset.mBitCrusherCrushAmount, false);
    mBitCrusher.SetSampleHoldFactor(preset.mBitCrusherSampleAndHoldFactor);

    mDistortionEnabled = preset.mDistortionEnabled;
    mDistortion.SetSampleRate(static_cast<int>(mSampleRate));
    mDistortion.SetInputGainDb(preset.mDistortionInputGainDb);
    mDistortion.SetOutputGainDb(preset.mDistortionOutputGainDb);
    mDistortion.SetType(preset.mDistortionType);
    mDistortion.SetOversample(preset.mDistortionOversample);
    for (int i = 0; i < 3; ++i) {
        mDistortion.SetupFilter(i, preset.mDistortionFilters[i]);
    }

    mAmpEnabled = preset.mAmpEnabled;
    mAmpSimulator.SetModel(preset.mAmpModel);
    for (int i = 0; i < AmpSimulator::kNumParameters; ++i) {
        mAmpSimulator.SetParameter(i, preset.mAmpParameters[i]);
    }
    for (int band = 0; band < 3; ++band) {
        mAmpEqEnabled[band] = preset.mAmpFilters[band].mEnabled;
        if (mAmpEqEnabled[band]) {
            mAmpEqCoefs[band].mSampleRate = static_cast<float>(mSampleRate);
            mAmpEqCoefs[band].MakeFromSettings(preset.mAmpFilters[band]);
            mAmpEqHistory[band][0] = 0.0;
            mAmpEqHistory[band][1] = 0.0;
        }
    }
}

// Reconstructed from eboot.elf at 0x99430.
void FusionSampler::_SetTrimVolume(float db) {
    ScopedCritSecPtr lock(GetBusLock());
    mTrimVolume.Set(db);
    mTrimGain = powf(10.0f, mTrimVolume.mValue * 0.05f);
}

// Reconstructed from eboot.elf at 0x99510.
void FusionSampler::_SetMinPitchBendCents(float cents) {
    ScopedCritSecPtr lock(GetBusLock());
    mMinPitchBendCents.Set(cents);
    _UpdatePitchBendFactor();
}

// Reconstructed from eboot.elf at 0x99630.
void FusionSampler::_SetMaxPitchBendCents(float cents) {
    ScopedCritSecPtr lock(GetBusLock());
    mMaxPitchBendCents.Set(cents);
    _UpdatePitchBendFactor();
}

// Reconstructed from eboot.elf at 0x99750.
void FusionSampler::_SetMaxNumVoices(int count) {
    ScopedCritSecPtr lock(GetBusLock());
    mMaxNumVoices = count;
}

// Reconstructed from eboot.elf at 0x997A0.
void FusionSampler::_SetStartPointMs(float ms) {
    ScopedCritSecPtr lock(GetBusLock());
    mStartPointMs.Set(ms);
}

// Reconstructed from eboot.elf at 0x99830.
void FusionSampler::_SetFineTuneCents(float cents) {
    ScopedCritSecPtr lock(GetBusLock());
    mFineTuneCents.Set(cents);
}

// Reconstructed from eboot.elf at 0x998C0.
void FusionSampler::_SetPan(float pan) {
    ScopedCritSecPtr lock(GetBusLock());
    mPan.Set(pan);
    mPanRamp.SetTarget(mPan.mValue, nullptr, nullptr);
}

// Reconstructed from eboot.elf at 0x999B0.
void FusionSampler::_SetPortamentoIsEnabled(bool enabled) {
    ScopedCritSecPtr lock(GetBusLock());
    mPortamentoEnabled = enabled;
}

// Reconstructed from eboot.elf at 0x99A00.
void FusionSampler::_SetPortamentoMode(PortamentoMode mode) {
    ScopedCritSecPtr lock(GetBusLock());
    mPortamentoMode = mode;
}

// Reconstructed from eboot.elf at 0x99A50. The line holds 2.5 seconds of
// stereo.
void FusionSampler::_SetDelayEnabled(bool enabled) {
    ScopedCritSecPtr lock(GetBusLock());
    mDelayEnabled = enabled;
    if (enabled) {
        mDelay.Prepare(static_cast<float>(mSampleRate), 2, 2500.0f);
    } else {
        mDelay.mBuffer.Release();
    }
}

// Reconstructed from eboot.elf at 0x99B80.
void FusionSampler::_SetBitCrusherEnabled(bool enabled) {
    if (mBitCrusherEnabled != enabled && enabled) {
        mBitCrusher.ClearHistory();
    }
    mBitCrusherEnabled = enabled;
}

// Reconstructed from eboot.elf at 0x99BC0.
void FusionSampler::Prepare(float sampleRate, unsigned int numChannels, unsigned int blockSize, bool) {
    AudioBus::Prepare(1.0f, numChannels, blockSize, false);
    SetSpeed(1.0f, false);
    SetSampleRate(sampleRate);
}

// Reconstructed from eboot.elf at 0x99C10. The pan, gain and mute ramps
// take kMinGainRampMs and the pitch bend 5 ms; each jumps to its target.
void FusionSampler::SetSampleRate(float sampleRate) {
    if (static_cast<float>(mSampleRate) == sampleRate) {
        return;
    }
    AudioBus::SetSampleRate(sampleRate);
    mControlRate = sampleRate / static_cast<float>(static_cast<unsigned int>(mBlockSize));
    mPanRamp.SetRateMs(mControlRate, kMinGainRampMs);
    mPanRamp.SetTarget(mPan.mValue, nullptr, nullptr);
    mPanRamp.Finish();
    mExpressionGain.SetRateMs(mControlRate, kMinGainRampMs);
    mExpressionGain.Finish();
    mChannelGain.SetRateMs(mControlRate, kMinGainRampMs);
    mChannelGain.Finish();
    mMuteGain.SetRateMs(mControlRate, kMinGainRampMs);
    mMuteGain.SetTarget(mChannelMute ? 0.0f : 1.0f, nullptr, nullptr);
    mMuteGain.Finish();
    mPitchBend.SetRateMs(mControlRate, 5.0f);
    mPitchBend.SetTarget(0.0f, nullptr, nullptr);
    mPitchBend.Finish();
    for (unsigned long i = 0; i < mLFOSettings.size(); ++i) {
        mLFOs[i].Prepare(sampleRate);
        mLFOs[i].UseSettings(&mLFOSettings[i]);
    }
    _SetPortamentoTimeMs(mPortamentoTimeMs.mValue);
    if (mDelayEnabled) {
        mDelay.Prepare(sampleRate, 2, 2500.0f);
    }
    mBitCrusher.Setup(2, static_cast<int>(sampleRate));
}

// Reconstructed from eboot.elf at 0x9A1A0.
ModulatorTarget* FusionSampler::_GetModulatorTargetByName(const char* name) {
    if (std::strcmp(name, mStartPointTarget.mName) == 0) {
        return &mStartPointTarget;
    }
    if (std::strcmp(name, mPitchTarget.mName) == 0) {
        return &mPitchTarget;
    }
    return &ModulatorTarget::kDummyTarget;
}

// Reconstructed from eboot.elf at 0x9A200. Releases the slaves and restores
// the channel, the playback state and the track gains for a new play.
void FusionSampler::ResetInstrumentState() {
    _DumpAllInstrumentSlaves();
    float fadeSecs = kMinGainRampMs * 0.001f;
    SetMidiChannelGain(1.0f, fadeSecs, 0);
    _SetMidiExpressionGain(1.0f, fadeSecs, 0);
    SetMidiChannelMute(false, 0);
    mSpeed = 1.0f;
    mBeat = 0.0f;
    mTempo = 120.0f;
    mPlayScale = 1.0f;
    mTranspose = 0;
    mTimeStretchFormantMode = 0;
    mTimeStretchAlgorithm = 0;
    for (float& gain : mTrackGains) {
        gain = 1.0f;
    }
}

// Reconstructed from eboot.elf at 0x9A2C0. The channel is ignored.
void FusionSampler::_SetMidiExpressionGain(float gain, float fadeSecs, signed char) {
    ScopedCritSecPtr lock(GetBusLock());
    float fadeMs = fadeSecs * 1000.0f;
    if (fadeMs < kMinGainRampMs) {
        fadeMs = kMinGainRampMs;
    }
    mExpression.Set(gain);
    mExpressionGain.SetRateMs(mControlRate, fadeMs);
    mExpressionGain.SetTarget(mExpression.mValue, nullptr, nullptr);
}

// Reconstructed from eboot.elf at 0x9A3F0.
void FusionSampler::ResetMidiState() {
    AllNotesOff();
    SetPitchBend(0.0f, 0);
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
    return mPan.mValue;
}

// Reconstructed from eboot.elf at 0x9A470.
float FusionSampler::_GetFineTuneCents() const {
    return mFineTuneCents.mValue;
}

// Reconstructed from eboot.elf at 0x9A480. The bend being ramped to.
float FusionSampler::GetPitchBend(signed char) const {
    return mPitchBend.mTarget;
}

// Reconstructed from eboot.elf at 0x9A490. The bend runs from -1 to 1; the
// channel is ignored.
void FusionSampler::SetPitchBend(float bend, signed char) {
    ScopedCritSecPtr lock(GetBusLock());
    mPitchBend.SetTarget(bend, nullptr, nullptr);
}

// Reconstructed from eboot.elf at 0x9A520.
float FusionSampler::_GetMinPitchBendCents() const {
    return mMinPitchBendCents.mValue;
}

// Reconstructed from eboot.elf at 0x9A530. A bend scales the range of its
// direction; the extra bend adds semitones.
void FusionSampler::_UpdatePitchBendFactor() {
    ScopedCritSecPtr lock(GetBusLock());
    float bend = mPitchBend.mValue;
    float cents = 0.0f < bend ? bend * mMaxPitchBendCents.mValue : -(bend * mMinPitchBendCents.mValue);
    mPitchBendFactor = exp2f((mExtraPitchBend * 100.0f + cents) * (1.0f / 1200.0f));
}

// Reconstructed from eboot.elf at 0x9A5D0.
float FusionSampler::_GetMaxPitchBendCents() const {
    return mMaxPitchBendCents.mValue;
}

// Reconstructed from eboot.elf at 0x9A5E0.
float FusionSampler::_GetPitchBendFactor() const {
    return mPitchBendFactor;
}

// Reconstructed from eboot.elf at 0x9A5F0. The channel is ignored.
void FusionSampler::SetExtraPitchBend(float bend, signed char) {
    mExtraPitchBend = bend;
}

// Reconstructed from eboot.elf at 0x9A600. The channel is ignored.
void FusionSampler::SetMidiChannelVolume(float volumeDb, float fadeSecs, signed char) {
    ScopedCritSecPtr lock(GetBusLock());
    mChannelVolume.Set(volumeDb);
    float gain = powf(10.0f, mChannelVolume.mValue * 0.05f);
    float fadeMs = fadeSecs * 1000.0f;
    if (fadeMs < kMinGainRampMs) {
        fadeMs = kMinGainRampMs;
    }
    mChannelGain.SetRateMs(mControlRate, fadeMs);
    mChannelGain.SetTarget(gain, nullptr, nullptr);
}

// Reconstructed from eboot.elf at 0x9A740: the channel gain in decibels,
// -96 for silence.
float FusionSampler::GetMidiChannelVolume(signed char) const {
    float gain = mChannelGain.mValue;
    if (gain == 0.0f) {
        return -96.0f;
    }
    return log10f(gain) * 20.0f;
}

// Reconstructed from eboot.elf at 0x9A780. The volume parameter follows the
// gain in decibels. The channel is ignored.
void FusionSampler::SetMidiChannelGain(float gain, float fadeSecs, signed char) {
    ScopedCritSecPtr lock(GetBusLock());
    mChannelVolume.Set(gain == 0.0f ? -96.0f : log10f(gain) * 20.0f);
    float fadeMs = fadeSecs * 1000.0f;
    if (fadeMs < kMinGainRampMs) {
        fadeMs = kMinGainRampMs;
    }
    mChannelGain.SetRateMs(mControlRate, fadeMs);
    mChannelGain.SetTarget(gain, nullptr, nullptr);
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
    return mTrimVolume.mValue;
}

// Reconstructed from eboot.elf at 0x9A9B0.
float FusionSampler::_GetTrimGain() const {
    return mTrimGain;
}

// Reconstructed from eboot.elf at 0x9A9C0.
float FusionSampler::_GetStartPointMs() const {
    return mStartPointMs.mValue;
}

// Reconstructed from eboot.elf at 0x9A9D0. The LFOs advance by the block at
// the sampler's speed.
void FusionSampler::_PrepareToProcess(unsigned int numFrames) {
    mPanRamp.Advance();
    mExpressionGain.Advance();
    mPortamentoPitch.Advance();
    _UpdatePitchBendFactor();
    for (unsigned long i = 0; i < mLFOSettings.size(); ++i) {
        mLFOs[i].Advance(static_cast<unsigned int>(static_cast<float>(numFrames) * mSpeed));
    }
}

// Reconstructed from eboot.elf at 0x9ACC0. Renders the sampler's voices
// into the block, mixes the slaves before and after the effects, and runs
// the distortion, the bit crusher, the amp with its EQ (on the left channel,
// copied to the right) and the delay. A silent sampler only runs the delay's
// tail. The binary returns whatever the bus unlock leaves.
bool FusionSampler::Process(AudioBuffer<float>& buffer) {
    StartCpuTimer(mCpuTimer);
    ScopedCritSecPtr lock(GetBusLock());
    CallPreProcessCallbacks(buffer.mNumValidFrames, static_cast<float>(mSampleRate), -1, -1, true);
    mChannelGain.Advance();
    mMuteGain.Advance();
    mPitchBend.Advance();
    buffer.mCleared = true;
    _ProcessNoteActions();
    if (mNumActiveVoices == 0 && mSlaveHandles.empty()) {
        mVoiceMeter.Update(0.0);
        buffer.Clear();
        if (mDelayEnabled) {
            buffer.Clear();
            buffer.mCleared = false;
            mDelay.Process(buffer);
        }
        if (mBitCrusherEnabled) {
            mBitCrusher.ClearHistory();
        }
        StopCpuTimer(mCpuTimer);
        return true;
    }
    if (mVoicePool == nullptr) {
        StopCpuTimer(mCpuTimer);
        return true;
    }

    mProcessTimer.Start();
    int numChannels = buffer.mConfig.mNumChannels;
    int numFrames = buffer.mConfig.mNumFrames;
    buffer.Clear();
    _PrepareToProcess(numFrames);
    int numPlaying = 0;
    for (unsigned int i = 0; i < mVoicePool->mNumVoices; ++i) {
        FusionVoice& voice = mVoicePool->mVoices[i];
        if (voice.mSampler != this) {
            continue;
        }
        if (voice.Process(mScratchChannels, numChannels, numFrames, mSpeed, mTempo) != 0) {
            ++numPlaying;
            buffer.mCleared = false;
            buffer.Accumulate(mScratchChannels, numChannels, numFrames);
        }
    }

    // A view of the scratch channels for the slaves.
    AudioBuffer<float> scratch;
    AudioBufferConfig config(numChannels, numFrames, static_cast<float>(mSampleRate), false);
    scratch.Release();
    scratch.mConfig = config;
    scratch.mNumChannels = config.mNumChannels;
    scratch.mNumFrames = config.mNumFrames;
    scratch.mFirstValidFrame = 0;
    scratch.mNumValidFrames = config.mNumFrames;
    scratch.mLastFrame = config.mNumFrames - 1;
    scratch.mCleared = false;
    for (int channel = 0; channel < numChannels; ++channel) {
        scratch.mChannelData[channel] = mScratchChannels[channel];
    }
    ProcessSlaves(mPreEffectSlaves, buffer, scratch);

    if (mDistortionEnabled) {
        mDistortion.Process(buffer);
    }
    if (mBitCrusherEnabled) {
        mBitCrusher.Process(buffer, buffer);
    }
    if (mAmpEnabled) {
        float* samples = buffer.mChannelData[0];
        mAmpSimulator.Process(&samples, &samples, buffer.mNumFrames);
        for (int band = 0; band < 3; ++band) {
            if (mAmpEqEnabled[band]) {
                FilterAmpEqBand(samples, buffer.mNumFrames, mAmpEqCoefs[band], mAmpEqHistory[band]);
            }
        }
        std::memcpy(buffer.mChannelData[1], samples, sizeof(float) * buffer.mNumFrames);
    }
    if (mDelayEnabled) {
        mDelay.Process(buffer);
    }
    if ((numPlaying | mPortamentoMode) == 0) {
        mPortamentoActive = false;
    }
    ProcessSlaves(mPostEffectSlaves, buffer, scratch);

    mNumActiveVoices = numPlaying;
    mProcessTimer.Stop();
    StopCpuTimer(mCpuTimer);
    return true;
}

// Reconstructed from eboot.elf at 0x9BE10: the controller as a 7-bit value.
// The lsb is always zero; an unhandled controller leaves the msb. The volume
// and expression values are multiplied by 127 twice, wrapping in 8 bits.
void FusionSampler::GetController(ControllerID controller, signed char& msb, signed char& lsb, signed char) const {
    lsb = 0;
    float value;
    switch (static_cast<int>(controller)) {
    case kBankSelect:
        msb = static_cast<signed char>(mPresetIndex);
        return;
    case kPortamentoTime:
        value = (mPortamentoTimeMs.mValue - sPortamentoTimeSpec.mMin) / (sPortamentoTimeSpec.mMax - sPortamentoTimeSpec.mMin);
        break;
    case kVolume:
    case kExpression: {
        float exponent;
        if (controller == kVolume) {
            exponent = mChannelVolume.mValue * 0.025f;
        } else {
            float gain = mExpression.mValue;
            exponent = gain == 0.0f ? -2.4f : log10f(gain) * 0.5f;
        }
        int scaled = static_cast<int>(powf(10.0f, exponent) * 127.0f);
        msb = static_cast<signed char>(static_cast<unsigned char>(scaled) * 127);
        return;
    }
    case kPan:
        msb = static_cast<signed char>(static_cast<int>((mPan.mValue + 1.0f) * 64.0f));
        return;
    case 14:  // The crusher's wet percentage.
        msb = static_cast<signed char>(static_cast<int>(mBitCrusher.mWet * 1.27f));
        return;
    case 15:  // The crusher's bits.
        msb = static_cast<signed char>(static_cast<int>(mBitCrusher.mCrush * (127.0f / 15.0f)));
        return;
    case 16: {  // The sample-and-hold factor.
        unsigned short factor = static_cast<unsigned short>(static_cast<int>(mBitCrusher.mSampleHoldFactor.mValue));
        msb = static_cast<signed char>(static_cast<int>((static_cast<float>(factor) + -1.0f) * (127.0f / 15.0f)));
        return;
    }
    case 22:
    case 23:  // An LFO's frequency.
        value = sLFOFrequencyInterpolator.ClampReverseEval(mLFOSettings[controller != 22].mFrequency);
        break;
    case 24:
    case 25:  // An LFO's depth.
        value = mLFOSettings[controller != 24].mDepth;
        break;
    case 26:  // The delay time.
        value = sDelayTimeInterpolator.ClampReverseEval(mDelay.mDelayTime);
        break;
    case 27:
        value = mDelay.mDryGain.mValue;
        break;
    case 28:
        value = mDelay.mWetGain.mValue;
        break;
    case 29:
        value = mDelay.mFeedbackGain.mValue;
        break;
    case 30:  // The pitch bend.
        value = GetPitchBend(0);
        break;
    case 31:  // The start point.
        value = (mStartPointMs.mValue - sStartPointMsSpec.mMin) / (sStartPointMsSpec.mMax - sStartPointMsSpec.mMin);
        break;
    case kPortamento:
        msb = mPortamentoEnabled ? 127 : 0;
        return;
    case kResonance:
        value = sFilterQInterpolator.ClampReverseEval(mFilterSettings.mQ);
        break;
    case kReleaseTime:
        msb = static_cast<signed char>(static_cast<int>((mADSRSettings[0].mRelease + -0.005f) * (127.0f / 1.995f)));
        return;
    case kAttackTime:
        msb = static_cast<signed char>(static_cast<int>((mADSRSettings[0].mAttack + -0.005f) * (127.0f / 1.995f)));
        return;
    case kBrightness:
        value = sFilterFrequencyInterpolator.ClampReverseEval(mFilterSettings.mFrequency);
        break;
    default:
        return;
    }
    msb = static_cast<signed char>(static_cast<int>(value * 127.0f));
}

// Reconstructed from eboot.elf at 0x9C060: a 7-bit controller value. Only
// the msb is read; the channel is ignored. Besides the standard controllers,
// 14-16 drive the bit crusher, 22-25 the LFOs' frequency and depth, 26-29
// the delay, 30 the pitch bend, 31 the start point and 52-59 the track
// gains. A bank select loads that preset of the patch.
void FusionSampler::SetController(ControllerID controller, signed char msb, signed char, signed char) {
    ScopedCritSecPtr lock(GetBusLock());
    float value = msb * kMidiValueScale;
    switch (static_cast<int>(controller)) {
    case kBankSelect:
        mPresetIndex = static_cast<unsigned long>(static_cast<long>(msb));
        _LoadPreset();
        break;
    case kPortamentoTime:
        _SetPortamentoTimeMs(value * (sPortamentoTimeSpec.mMax - sPortamentoTimeSpec.mMin) + sPortamentoTimeSpec.mMin);
        break;
    case kVolume:
        SetMidiChannelVolume(msb != 0 ? log10f(value) * 40.0f : -96.0f, 0.0f, 0);
        break;
    case kPan:
        mPan.Set(msb * (1.0f / 64.0f) + -1.0f);
        mPanRamp.SetTarget(mPan.mValue, nullptr, nullptr);
        break;
    case kExpression:
        _SetMidiExpressionGain(MidiValueToGain(msb), 0.0f, 0);
        break;
    case 14:
        mBitCrusher.SetWet(msb * (100.0f / 127.0f), false);
        break;
    case 15:
        mBitCrusher.SetCrush(msb * (15.0f / 127.0f), false);
        break;
    case 16:
        mBitCrusher.SetSampleHoldFactor(static_cast<unsigned short>(static_cast<int>(msb * (15.0f / 127.0f) + 1.5f)));
        break;
    case 22:
    case 23: {
        int index = controller != 22;
        mLFOSettings[index].mFrequency = sLFOFrequencyInterpolator.ClampEval(value);
        mLFOs[index].UseSettings(&mLFOSettings[index]);
        _UpdateVoiceLFOS();
        break;
    }
    case 24:
    case 25: {
        int index = controller != 24;
        mLFOSettings[index].mDepth = value;
        mLFOs[index].UseSettings(&mLFOSettings[index]);
        _UpdateVoiceLFOS();
        break;
    }
    case 26:
        mDelay.SetDelaySeconds(sDelayTimeInterpolator.ClampEval(value));
        break;
    case 27:
        mDelay.SetDryGain(value);
        break;
    case 28:
        mDelay.SetWetGain(value);
        break;
    case 29:
        mDelay.SetFeedbackGain(value);
        break;
    case 30:
        SetPitchBend(value, 0);
        break;
    case 31:
        _SetStartPointMs(value * (sStartPointMsSpec.mMax - sStartPointMsSpec.mMin) + sStartPointMsSpec.mMin);
        break;
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
        mTrackGains[controller - 52] = MidiValueToGain(msb);
        break;
    case kPortamento:
        _SetPortamentoIsEnabled(msb > 0);
        break;
    case kResonance:
        mFilterSettings.mQ = sFilterQInterpolator.ClampEval(value);
        break;
    case kReleaseTime:
        mADSRSettings[0].mRelease = msb * (1.995f / 127.0f) + 0.005f;
        break;
    case kAttackTime:
        mADSRSettings[0].mAttack = msb * (1.995f / 127.0f) + 0.005f;
        break;
    case kBrightness:
        mFilterSettings.mFrequency = sFilterFrequencyInterpolator.ClampEval(value);
        break;
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0x9C9E0.
void FusionSampler::SetTrackGain(int track, signed char value) {
    if (track <= 8) {
        (&mTrackGains[0])[track] = MidiValueToGain(value);
    }
}

// Reconstructed from eboot.elf at 0x9CA40. Only instrument generators can be
// slaves.
bool FusionSampler::AddSlave(unsigned int handle, InstrumentSlaveType type) {
    AudioGenerator* generator = theSoundManager.LockIfOwned(handle);
    if (generator == nullptr) {
        return false;
    }
    if (!generator->IsInstrument()) {
        --generator->mRefCount;
        return false;
    }
    InstrumentGenerator* slave = static_cast<InstrumentGenerator*>(generator);
    if (slave == nullptr) {
        return false;
    }
    _AddSlaveGenerator(slave, type);
    mSlaveHandles.push_back(handle);
    --slave->mRefCount;
    return true;
}

// Reconstructed from eboot.elf at 0x9CB90.
void FusionSampler::_AddSlaveGenerator(InstrumentGenerator* slave, InstrumentSlaveType type) {
    ScopedCritSecPtr lock(GetBusLock());
    if (type == kSlaveBeforeEffects) {
        mPreEffectSlaves.push_back(slave);
    } else {
        mPostEffectSlaves.push_back(slave);
    }
}

// Reconstructed from eboot.elf at 0x9CD90.
bool FusionSampler::RemoveSlave(unsigned int handle) {
    AudioGenerator* generator = theSoundManager.LockIfOwned(handle);
    if (generator == nullptr) {
        return false;
    }
    if (!generator->IsInstrument()) {
        --generator->mRefCount;
        return false;
    }
    InstrumentGenerator* slave = static_cast<InstrumentGenerator*>(generator);
    if (slave == nullptr) {
        return false;
    }
    bool removed;
    {
        ScopedCritSecPtr lock(GetBusLock());
        removed = _RemoveSlaveGenerator(slave);
        for (unsigned int* entry = mSlaveHandles.begin(); entry != mSlaveHandles.end(); ++entry) {
            if (*entry == handle) {
                mSlaveHandles.erase(entry);
                removed = true;
                break;
            }
        }
    }
    --slave->mRefCount;
    return removed;
}

// Reconstructed from eboot.elf at 0x9CE90.
bool FusionSampler::_RemoveSlaveGenerator(InstrumentGenerator* slave) {
    ScopedCritSecPtr lock(GetBusLock());
    bool removed = false;
    for (InstrumentGenerator** entry = mPreEffectSlaves.begin(); entry != mPreEffectSlaves.end(); ++entry) {
        if (*entry == slave) {
            mPreEffectSlaves.erase(entry);
            removed = true;
            break;
        }
    }
    for (InstrumentGenerator** entry = mPostEffectSlaves.begin(); entry != mPostEffectSlaves.end(); ++entry) {
        if (*entry == slave) {
            mPostEffectSlaves.erase(entry);
            removed = true;
            break;
        }
    }
    return removed;
}

// Reconstructed from eboot.elf at 0x9CFA0.
ResourcePath FusionSampler::GetPatchPath() const {
    if (mPatch) {
        return mPatch->mPath;
    }
    return ResourcePath("");
}
