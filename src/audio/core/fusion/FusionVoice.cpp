#include "audio/core/fusion/FusionVoice.h"

#include <algorithm>
#include <cmath>

#include "audio/core/decoders/AudioDecoder.h"
#include "audio/core/dsp/SmbPitchShift.h"
#include "audio/core/fusion/FusionSampler.h"
#include "audio/core/fusion/FusionVoicePool.h"
#include "audio/core/system/Audio.h"

namespace {

// Frames per control block: the envelopes, the filter ramps and the gain
// steps advance once per block. The binary reads it from an int constant of
// audio/Audio.o (0x124D440) that the map does not name. Name not in the
// reference map.
constexpr int kControlBlockFrames = 4;
// Ramp lengths of the filter coefficients and the filter gain. Names not in
// the reference map.
constexpr float kFilterRampSeconds = 0.015f;
constexpr float kFilterGainRampSeconds = 0.01f;
// Octaves of filter modulation at full depth: LFOs, then the assignable
// envelope. Names not in the reference map.
constexpr float kLFOFilterOctaves = 5.0f;
constexpr float kEnvelopeFilterOctaves = 10.0f;
// Semitones of pitch modulation from an LFO at full depth. Name not in the
// reference map.
constexpr double kLFOPitchSemitones = 2.0;
// Drift, in ms, that a tempo-synced voice tolerates, and the speed
// correction beyond it. Names not in the reference map.
constexpr float kTempoSyncToleranceMs = 5.0f;
constexpr float kTempoSyncCorrection = 0.01f;
// A voice whose peaks stay below this is silent. Name not in the reference
// map.
constexpr double kSilentLevel = 0.0001;

// Headroom applied to every voice, -3 dB. Written by the static initializer
// at 0xA0340. Name not in the reference map.
const float sVoiceGain = std::pow(10.0f, -3.0f / 20.0f);
// Also written by the static initializer at 0xA0340 and read nowhere in this
// build; the name assumes a stripped debug filter and is weak. Name not in
// the reference map.
[[maybe_unused]] int sDebugVoiceIndex = -1;

}  // namespace

const double FusionVoice::kMaxPitchOffsetCents = 14400.0;

// Reconstructed from eboot.elf at 0x9D920.
FusionVoice::FusionVoice()
    : mWaitingForAttack(false),
      mKeyzone(nullptr),
      mSampler(nullptr),
      mPcmDecoder(AudioDecoder::NewDecoderForFormat(AudioData::kEncodedPcm)),
      mXmaDecoder(nullptr),
      mMoggDecoder(nullptr),
      mDecoder(nullptr),
      mPitchShift(nullptr),
      mFilterOctaves(0.0f),
      mPanMix(),
      mLevels(),
      mPool(nullptr),
      mStartBeat(0.0f),
      mTempoCorrection(0.0f),
      mLastSyncPosition(0.0),
      mStartPosition(0.0) {
    SetSampleRate(Audio::GetSamplesPerSecond());
}

// Reconstructed from eboot.elf at 0x9DC50.
void FusionVoice::SetSampleRate(double sampleRate) {
    mSampleRate = sampleRate;
    mSecondsPerSample = 1.0 / sampleRate;
    mModEnvelope.Prepare(static_cast<float>(mSampleRate));
    mAmpEnvelope.Prepare(static_cast<float>(mSampleRate));
    mLFOs[0].Prepare(static_cast<float>(mSampleRate));
    mLFOs[1].Prepare(static_cast<float>(mSampleRate));
    float controlRate = static_cast<float>(mSampleRate) / kControlBlockFrames;
    mFilter.SetRate(controlRate, kFilterRampSeconds);
    mFilterGain.SetRate(controlRate, kFilterGainRampSeconds);
}

// Reconstructed from eboot.elf at 0x9DD30.
FusionVoice::~FusionVoice() {
    delete mXmaDecoder;
    mXmaDecoder = nullptr;
    delete mPcmDecoder;
    mPcmDecoder = nullptr;
    delete mMoggDecoder;
    mMoggDecoder = nullptr;
}

// Reconstructed from eboot.elf at 0x9DE50. The sampler's time-stretch mode,
// when set, replaces the keyzone's on the shifter the pool chose. The first
// decoder that takes the sample's format plays it.
bool FusionVoice::AssignIDs(
    const FusionSampler* sampler,
    const FusionPatchCom::KeyzoneSettings* keyzone,
    unsigned int id,
    SmbPitchShift* pitchShift) {
    const AudioData* data = keyzone->mSample->GetAudioData();
    if (data->IsEmpty()) {
        return false;
    }
    mKeyzone = keyzone;
    mSampler = sampler;
    mPitchShift = pitchShift;
    int algorithm;
    int formantMode;
    if (pitchShift != nullptr && sampler->GetTimeStretchMode(&algorithm, &formantMode)) {
        mPitchShift->SetTimeStretchMode(algorithm, formantMode);
    }
    mAmpEnvelope.mSettings = &mSampler->mADSRSettings[0];
    mModEnvelope.mSettings = &mSampler->mADSRSettings[1];
    mLFOs[0].UseSettings(&mSampler->mLFOSettings[0]);
    mLFOs[1].UseSettings(&mSampler->mLFOSettings[1]);
    SetSampleRate(mSampler->mSampleRate);
    mSampleRateRatio = static_cast<double>(data->GetSampleRate()) * mSecondsPerSample;

    mDecoder = nullptr;
    if (mPcmDecoder->CanDecode(static_cast<AudioData::EncodedFormat>(data->GetEncodedFormat()))) {
        mDecoder = mPcmDecoder;
        mDecoder->SetAudioData(data, pitchShift, keyzone->mTrackMap, mSampler);
    } else if (
        mMoggDecoder != nullptr &&
        mMoggDecoder->CanDecode(static_cast<AudioData::EncodedFormat>(data->GetEncodedFormat()))) {
        mDecoder = mMoggDecoder;
        mDecoder->SetAudioData(data, pitchShift, keyzone->mTrackMap, mSampler);
    } else if (
        mXmaDecoder != nullptr &&
        mXmaDecoder->CanDecode(static_cast<AudioData::EncodedFormat>(data->GetEncodedFormat()))) {
        mDecoder = mXmaDecoder;
        mDecoder->SetAudioData(data, pitchShift);
    }
    // The map's signature keeps the sampler const; the count is the
    // sampler's bookkeeping of its voices.
    ++const_cast<FusionSampler*>(mSampler)->mNumActiveVoices;
    mNoteId = static_cast<int>(id);
    mWaitingForAttack = true;
    return true;
}

// Reconstructed from eboot.elf at 0x9E130.
void FusionVoice::SetupLFO(int index, const LFO::Settings& settings) {
    if (index != 0) {
        mLFOs[1].UseSettings(&settings);
    } else {
        mLFOs[0].UseSettings(&settings);
    }
}

// Reconstructed from eboot.elf at 0x9E160.
void FusionVoice::_Attack() {
    mAmpEnvelope.Attack();
    mModEnvelope.Attack();
    mWaitingForAttack = false;
}

// Reconstructed from eboot.elf at 0x9E2E0.
void FusionVoice::Release() {
    mAmpEnvelope.Release();
    mModEnvelope.Release();
}

// Reconstructed from eboot.elf at 0x9E380.
void FusionVoice::FastRelease() {
    mAmpEnvelope.FastRelease();
    mModEnvelope.FastRelease();
}

// Reconstructed from eboot.elf at 0x9E440. The pool is locked because its
// steal and limit scans read the voice.
void FusionVoice::Kill() {
    if (mPool != nullptr) {
        mPool->Lock();
    }
    mLevels[0] = 0.0f;
    mLevels[1] = 0.0f;
    mNoteId = -1;
    mSampler = nullptr;
    if (mPool != nullptr && mPitchShift != nullptr) {
        mPool->ReleasePitchShift(mPitchShift);
    }
    mPitchShift = nullptr;
    mAmpEnvelope.Stop();
    mModEnvelope.Stop();
    mPosition = 0.0;
    mWaitingForAttack = false;
    mPitchOffset = 0.0;
    mNumFrames = 0.0;
    if (mDecoder != nullptr) {
        mDecoder->SetAudioData(nullptr, nullptr);
    }
    if (mPool != nullptr) {
        mPool->Unlock();
    }
}

// Reconstructed from eboot.elf at 0x9E520. The buffer becomes a stereo view
// of one control block; Process points it at the output.
void FusionVoice::Init(FusionVoicePool* pool, char id, bool skipDecoders) {
    mPool = pool;
    mId = id;
    mNoteId = -1;
    mSampler = nullptr;
    mPitchShift = nullptr;
    mPosition = 0.0;
    if (mBuffer.mConfig.mNumFrames < kControlBlockFrames ||
        static_cast<unsigned int>(mBuffer.mConfig.mNumChannels) < 2) {
        mBuffer.Configure(
            AudioBufferConfig(2, kControlBlockFrames, 0.0f, false), AudioBufferBase::kCleanupNone);
    }
    if (!skipDecoders) {
        mMoggDecoder = AudioDecoder::NewDecoderForFormat(AudioData::kEncodedMogg);
        mXmaDecoder = AudioDecoder::NewDecoderForFormat(AudioData::kEncodedXma);
    }
    mWaitingForAttack = false;
}

// Reconstructed from eboot.elf at 0x9E6A0.
void FusionVoice::SetPitchOffset(double cents) {
    mPitchOffset = cents;
}

// Reconstructed from eboot.elf at 0x9E6B0. Each LFO restarts at its initial
// phase or joins the sampler's free-running one. The filter starts from its
// target with the gain ramp finished.
void FusionVoice::_PrepareWithPitchOffsetAndGain(double pitchOffsetCents, float gain) {
    for (int i = 0; i < 2; ++i) {
        if (mLFOs[i].mSettings->mRetrigger) {
            mLFOs[i].Retrigger();
        } else {
            mLFOs[i].SetPhase(mSampler->mLFOs[i].GetPhase());
        }
    }
    double cents = pitchOffsetCents;
    if (cents > kMaxPitchOffsetCents) {
        cents = kMaxPitchOffsetCents;
    } else {
        cents = std::max(cents, -kMaxPitchOffsetCents);
    }
    mPitchRatio = std::exp2(cents / 1200.0);
    mPlaybackRatio = mSampleRateRatio;
    mGain = gain * mKeyzone->mVolumeGain;
    float channelGain = mSampler->GetMidiChannelGain(0);
    mOutputGain = mSampler->_GetTrimGain() * channelGain * mSampler->mExpressionGain.mValue * mGain * sVoiceGain;

    mFilterGain.SetTarget(1.0f, nullptr, nullptr);
    mFilterGain.Finish();
    mFilter.ResetHistory();
    mFilterFrequency = mSampler->mFilterSettings.mFrequency;
    mFilterOctaves = _ComputeOctaveShift();
    _ApplyModsToFilter(true);
    mFilter.FinishRamp();
    mFilterGain.SetTarget(1.0f, nullptr, nullptr);
    mFilterGain.Finish();

    _CalculatePanMix(&mPanMix[0], &mPanMix[1], &mPanMix[2], &mPanMix[3]);
    mLevels[0] = 0.0f;
    mLevels[1] = 0.0f;
    mDecoder->SetFrame(0);
}

// Inlined into 0x9E6B0 and 0x9EBA0 in this build.
inline float FusionVoice::_ComputeOctaveShift() {
    float octaves = 0.0f;
    if (mLFOs[0].mSettings->mTarget == LFO::Settings::kTargetFilterFreq) {
        octaves += mLFOs[0].GetValue() * kLFOFilterOctaves;
    }
    if (mLFOs[1].mSettings->mTarget == LFO::Settings::kTargetFilterFreq) {
        octaves += mLFOs[1].GetValue() * kLFOFilterOctaves;
    }
    if (mModEnvelope.mSettings->mTarget == ADSR::Settings::kTargetFilterFreq) {
        octaves += mModEnvelope.GetValue() * kEnvelopeFilterOctaves;
    }
    return octaves;
}

// Reconstructed from eboot.elf at 0x9EBA0. A modulation jump of more than an
// octave, or a cutoff change of more than an octave, first ramps the filter
// gain down (deeper for larger cutoff ratios); _UpdateFilterSettings then
// ramps the coefficients. Smaller changes ramp the coefficients directly
// once both ramps are idle.
void FusionVoice::_ApplyModsToFilter(bool force) {
    float octaves = _ComputeOctaveShift();
    float jump = octaves - mFilterOctaves > 0.0f ? octaves - mFilterOctaves : mFilterOctaves - octaves;
    mFilterOctaves = octaves;
    float scale = std::exp2(octaves);
    mFilterSettings = mSampler->mFilterSettings;
    float frequency = mFilterSettings.mFrequency;
    float ratio = mFilterFrequency / frequency;
    float modulated = frequency * scale;
    mFilterFrequency = frequency;
    if (modulated != frequency || force) {
        mFilterSettings.mFrequency = modulated;
    }
    if (ratio > 1.0f) {
        ratio = 1.0f / ratio;
    }
    float ratioSquared = ratio * ratio;
    if (jump > 1.0f) {
        if (mFilterGain.IsIdle()) {
            float dip = (1.0f - jump) + (ratio < 0.5f ? ratioSquared : 1.0f);
            mFilterGain.SetTarget(std::max(0.0f, dip), _UpdateFilterSettings, this);
        }
    } else if (ratio < 0.5f) {
        if (mFilterGain.IsIdle()) {
            mFilterGain.SetTarget(ratioSquared, _UpdateFilterSettings, this);
        }
    } else if (mFilterGain.IsIdle() && mFilter.mRampProgress == 1.0f) {
        BiquadFilter::Coefs coefs(static_cast<float>(mSampleRate));
        coefs.MakeFromSettings(mFilterSettings);
        mFilter.SetTargetCoefs(coefs, nullptr, nullptr);
    }
}

// Reconstructed from eboot.elf at 0x9EE90. The pan is the sampler's plus the
// LFOs aimed at it, clamped to [-1, 1]; each source channel is then scaled by
// the keyzone's gain for it.
void FusionVoice::_CalculatePanMix(
    float* leftToLeft, float* leftToRight, float* rightToLeft, float* rightToRight) const {
    float pan = mSampler->mPanRamp.mValue;
    if (mLFOs[0].mSettings->mTarget == LFO::Settings::kTargetPan) {
        pan += mLFOs[0].GetValue();
    }
    if (mLFOs[1].mSettings->mTarget == LFO::Settings::kTargetPan) {
        pan += mLFOs[1].GetValue();
    }
    pan = pan > 1.0f ? 1.0f : std::max(pan, -1.0f);
    *rightToRight = std::min(pan + 1.0f, 1.0f);
    *leftToLeft = std::min(1.0f - pan, 1.0f);
    *leftToRight = 1.0f - *leftToLeft;
    *rightToLeft = 1.0f - *rightToRight;
    *leftToLeft *= mKeyzone->mLeftGain;
    *leftToRight *= mKeyzone->mLeftGain;
    *rightToRight *= mKeyzone->mRightGain;
    *rightToLeft *= mKeyzone->mRightGain;
}

// Reconstructed from eboot.elf at 0x9EFB0. A tempo-synced keyzone started
// late counts its start beat back by the offset at the sampled tempo. A
// start past the end kills the voice unless the data allows it.
void FusionVoice::AttackWithTargetNote(unsigned char note, float gain, double startOffsetMs) {
    mNote = note;
    const FusionPatchCom::KeyzoneSettings* keyzone = mKeyzone;
    mStartBeat = mSampler->mBeat;
    if (startOffsetMs > 0.0 && keyzone->mTempoSync) {
        mStartBeat += static_cast<float>(-startOffsetMs / 60000.0 * keyzone->mSampledTempo);
    }
    double noteOffsetCents =
        keyzone->mUnpitched ? 0.0 : static_cast<signed char>(note - keyzone->mRootNote) * 100.0;
    _PrepareWithPitchOffsetAndGain(noteOffsetCents + mPitchOffset, gain);
    mPosition = startOffsetMs / 1000.0 * mSampler->mSampleRate * mSampleRateRatio;
    mStartPosition = mPosition;
    const AudioData* data = mKeyzone->mSample->GetAudioData();
    mNumFrames = static_cast<unsigned int>(data->GetNumFrames());
    // Slot 12 of AudioData gates a start past the end; AudioSampleResource.h
    // calls it GetSampleFormat, but here it reads as a loop test.
    if (mNumFrames < mPosition && data->GetSampleFormat() == 0) {
        Kill();
        return;
    }
    mTempoCorrection = 0.0f;
    mLastSyncPosition = mPosition;
    _Attack();
}

// Reconstructed from eboot.elf at 0x9F110.
bool FusionVoice::IsWaitingForAttack() const {
    return mWaitingForAttack;
}

// Reconstructed from eboot.elf at 0x9F120. A released voice stays in use
// while its output is audible.
bool FusionVoice::IsInUse() const {
    return mLevels[0] > 0.0f || mLevels[1] > 0.0f || mAmpEnvelope.mStage != ADSR::State::kStageOff;
}

// Reconstructed from eboot.elf at 0x9F150.
void FusionVoice::_UpdateFilterSettings(void* voice) {
    FusionVoice* self = static_cast<FusionVoice*>(voice);
    BiquadFilter::Coefs coefs(static_cast<float>(self->mSampleRate));
    coefs.MakeFromSettings(self->mFilterSettings);
    self->mFilter.SetTargetCoefs(coefs, _RestoreFilterGain, self);
}

// Reconstructed from eboot.elf at 0x9F250.
void FusionVoice::_RestoreFilterGain(void* voice) {
    static_cast<FusionVoice*>(voice)->mFilterGain.SetTarget(1.0f, nullptr, nullptr);
}

// Reconstructed from eboot.elf at 0x9F2A0. The decoder renders straight into
// the channels, one control block at a time, with the volume envelope and a
// gain stepped towards the new output gain; the filter then runs over the
// block unless it passes the signal through. The pan mix ramps across the
// call. A voice whose envelope has finished, or that has run out of sample,
// dies once its output falls silent, unless portamento holds it.
unsigned int FusionVoice::Process(
    float** channels, unsigned int /* numChannels */, unsigned int numFrames, float speed, float tempo) {
    if (!IsInUse()) {
        return 0;
    }
    if (mSampler == nullptr || mKeyzone == nullptr || mDecoder == nullptr) {
        return 0;
    }
    float frames = static_cast<float>(numFrames);
    unsigned int lfoSamples = static_cast<unsigned int>(frames * speed);
    mLFOs[0].Advance(lfoSamples);
    mLFOs[1].Advance(lfoSamples);
    double playbackRatio = mPlaybackRatio;
    float pitchBendFactor = mSampler->_GetPitchBendFactor();
    double semitones = 0.0;
    if (mSampler->mPortamentoEnabled) {
        semitones = static_cast<double>(mSampler->_GetCurrentPortamentoPitch()) - mNote;
    }
    if (mLFOs[0].mSettings->mTarget == LFO::Settings::kTargetPitch) {
        semitones += kLFOPitchSemitones * mLFOs[0].GetValue();
    }
    if (mLFOs[1].mSettings->mTarget == LFO::Settings::kTargetPitch) {
        semitones += kLFOPitchSemitones * mLFOs[1].GetValue();
    }
    double pitchModRatio = std::exp2(semitones / 12.0);
    float fineTuneRatio = mKeyzone->mFineTuneRatio;
    float* left = channels[0];
    float* right = channels[1];
    ADSR::State::Stage stage = mAmpEnvelope.mStage;

    _ApplyModsToFilter(false);
    float channelGain = mSampler->GetMidiChannelGain(0) * mSampler->mMuteGain.mValue;
    float targetGain =
        mSampler->_GetTrimGain() * channelGain * mSampler->mExpressionGain.mValue * mGain * sVoiceGain;
    float startGain = mOutputGain;
    mOutputGain = targetGain;
    double pitchRatio = mPitchRatio;
    // The call goes through the AudioGenerator base.
    AudioGenerator* generator = const_cast<FusionSampler*>(mSampler);
    float playScale = generator->GetPlayScale();
    bool bypassFilter = mFilter.IsPassThrough() && mFilterGain.mValue == 1.0f && mFilterGain.mTarget == 1.0f;

    const AudioData* data = mKeyzone->mSample->GetAudioData();
    unsigned int channelMap[2] = {0, static_cast<unsigned int>(data->GetNumChannels()) > 1};
    const FusionPatchCom::KeyzoneSettings* keyzone = mKeyzone;
    float timeRatio = speed;
    if (keyzone->mMaintainTime && tempo > 0.0f && keyzone->mTempoSync) {
        float sampledTempo = keyzone->mSampledTempo;
        float tempoRatio = tempo / sampledTempo * speed;
        float beat = mSampler->mBeat;
        if (beat > mStartBeat && mPosition > mLastSyncPosition) {
            float expectedMs = (beat - mStartBeat) * 60000.0f / sampledTempo;
            mLastSyncPosition = mPosition;
            float playedFrames;
            if (mPitchShift->HasPositionOffset()) {
                playedFrames = static_cast<float>(mStartPosition) + mPitchShift->GetPositionOffset();
            } else {
                playedFrames = static_cast<float>(mPosition);
            }
            float playedMs = playedFrames * 1000.0f / data->GetSampleRate();
            float driftMs = playedMs - expectedMs;
            if (driftMs > kTempoSyncToleranceMs) {
                mTempoCorrection = tempoRatio * -kTempoSyncCorrection;
            } else {
                mTempoCorrection = driftMs < -kTempoSyncToleranceMs ? tempoRatio * kTempoSyncCorrection : 0.0f;
            }
        }
        timeRatio = std::max(0.0f, tempoRatio + mTempoCorrection);
    }

    float frameStep = 1.0f / frames;
    if (numFrames != 0) {
        double rate = static_cast<double>(pitchBendFactor) * playbackRatio * pitchModRatio * fineTuneRatio;
        float gainStep = (targetGain - startGain) * frameStep * static_cast<float>(kControlBlockFrames);
        double shiftRatio = static_cast<double>(playScale) * pitchRatio;
        float gain = startGain;
        float* blockLeft = left;
        float* blockRight = right;
        unsigned int remaining = numFrames;
        unsigned int done = 0;
        do {
            unsigned int blockFrames = std::min(static_cast<unsigned int>(kControlBlockFrames), remaining);
            mBuffer.mChannelData[0] = blockLeft;
            mBuffer.mChannelData[1] = blockRight;
            mBuffer.mNumFrames = blockFrames;
            unsigned int envelopeSamples = static_cast<unsigned int>(static_cast<float>(blockFrames) * speed);
            mAmpEnvelope.Advance(envelopeSamples);
            mModEnvelope.Advance(envelopeSamples);
            float amplitude = mAmpEnvelope.GetValue() * gain;
            float gains[2] = {amplitude, amplitude};
            // The sample's sustain loop holds until the release.
            mPosition = mDecoder->Render(
                mBuffer,
                mPosition,
                rate,
                shiftRatio,
                timeRatio,
                stage != ADSR::State::kStageRelease,
                channelMap,
                gains);
            mDecoder->SetFrame(static_cast<unsigned int>(mPosition));
            if (!bypassFilter) {
                mFilter.AdvanceRamp();
                mFilterGain.Advance();
                mFilter.Filter(blockLeft, blockFrames, 0, mFilterGain.mValue);
                mFilter.Filter(blockRight, blockFrames, 1, mFilterGain.mValue);
            }
            blockLeft += blockFrames;
            blockRight += blockFrames;
            gain += gainStep;
            remaining -= blockFrames;
            done += blockFrames;
        } while (done < numFrames);
    }

    float mix[4];
    _CalculatePanMix(&mix[0], &mix[1], &mix[2], &mix[3]);
    float startMix[4];
    float mixStep[4];
    for (int i = 0; i < 4; ++i) {
        startMix[i] = mPanMix[i];
        mPanMix[i] = mix[i];
        mixStep[i] = (mix[i] - startMix[i]) * frameStep;
    }
    left = channels[0];
    right = channels[1];
    if (startMix[2] == 0.0f && startMix[1] == 0.0f && mixStep[3] == 0.0f && mixStep[2] == 0.0f &&
        mixStep[0] == 0.0f && mixStep[1] == 0.0f) {
        for (unsigned int i = 0; i < numFrames; ++i) {
            left[i] *= startMix[0];
            right[i] *= startMix[3];
        }
    } else {
        for (unsigned int i = 0; i < numFrames; ++i) {
            float inLeft = left[i];
            float inRight = right[i];
            left[i] = inRight * startMix[2] + inLeft * startMix[0];
            right[i] = inRight * startMix[3] + inLeft * startMix[1];
            for (int j = 0; j < 4; ++j) {
                startMix[j] += mixStep[j];
            }
        }
    }

    float peakLeft = 0.0f;
    float peakRight = 0.0f;
    for (unsigned int i = 0; i < numFrames; ++i) {
        peakLeft = std::max(peakLeft, std::fabs(left[i]));
        peakRight = std::max(peakRight, std::fabs(right[i]));
    }
    mLevels[0] = peakLeft;
    mLevels[1] = peakRight;
    bool envelopeOff = mAmpEnvelope.mStage == ADSR::State::kStageOff;
    bool playing = !envelopeOff && mPosition < mNumFrames;
    bool audible = peakLeft >= kSilentLevel || peakRight >= kSilentLevel || playing;
    if ((envelopeOff || !mSampler->mPortamentoEnabled) && !audible) {
        Kill();
    }
    return numFrames;
}

// Reconstructed from eboot.elf at 0xA0320.
bool FusionVoice::MatchesIDs(
    const FusionSampler* sampler,
    unsigned int id,
    const FusionPatchCom::KeyzoneSettings* keyzone) {
    if (mNoteId != static_cast<int>(id) || mSampler != sampler) {
        return false;
    }
    return keyzone == nullptr || mKeyzone == keyzone;
}
