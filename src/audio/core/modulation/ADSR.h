#pragma once

#include <cstddef>

#include "audio/core/dsp/ParameterSpec.h"
#include "audio/core/modulation/ModulatorTarget.h"
#include "utl/containers/Vector.h"

// Attack-decay-sustain-release envelope (audio/ADSR.o, 0xBD740 to 0xBDBA7).
// The state transitions FusionVoice inlines are defined here.
class ADSR {
public:
    // One element of a Fusion patch's "adsrs" array. Field names follow the
    // properties the patch registry (0x69760) binds to their offsets.
    struct Settings {
        // "target": "none", "volume" or "filter_freq". Value names not in
        // the reference map.
        enum Target : unsigned char {
            kTargetNone = 0,
            kTargetVolume = 1,
            kTargetFilterFreq = 2,
            // StringToTarget's result for an unknown name; the value between
            // has no name.
            kTargetInvalid = 4,
        };

        // The table at 0x18E59C0; null past the last target. At 0xBD740.
        static const char* TargetToString(Target target);
        static Target StringToTarget(const char* name);  // 0xBD760
        // The editor values of the amplitude envelope's target ("Volume"
        // only) and of the assignable one ("None" or "Filter Freq"). At
        // 0xBD7C0 and 0xBD8B0.
        static eastl::vector<AllowedValue<unsigned char>> GetVolumeAllowedTargetValues();
        static eastl::vector<AllowedValue<unsigned char>> GetAssignableAllowedTargetValues();

        // The patch's defaults, built by FusionSampler's constructor at
        // 0x95C40.
        Settings()
            : mTarget(kTargetVolume),
              mEnabled(false),
              mAttack(0.0f),
              mDecay(0.0f),
              mSustain(1.0f),
              mRelease(1.0f),
              mDepth(0.5f) {}

        Target mTarget;
        bool mEnabled;
        float mAttack;   // Seconds.
        float mDecay;    // Seconds.
        float mSustain;  // Fraction of the peak.
        float mRelease;  // Seconds.
        float mDepth;
    };

    // One running envelope. The object is 40 bytes.
    class State {
    public:
        // Names not in the reference map.
        enum Stage : int {
            kStageAttack = 0,
            kStageDecay = 1,
            kStageSustain = 2,
            kStageRelease = 3,
            kStageOff = 4,
        };

        // Inlined into FusionVoice's constructor at 0x9D920.
        State() : mLevel(0.0f), mStage(kStageOff), mSamplesElapsed(0), mSettings(nullptr), mDepthMode(0) {}

        // Stores the sample rate. At 0xBDA20.
        void Prepare(float sampleRate);
        // The level shaped by the settings' depth and the depth mode. At
        // 0xBDA30.
        float GetValue() const;
        // Moves the level through the current stage. At 0xBDA90.
        void Advance(unsigned int numSamples);

        // Starts an idle envelope: the attack, or straight to the decay or
        // the sustain level when there is no attack. Inlined into
        // FusionVoice::_Attack at 0x9E160. Name not in the reference map.
        void Attack() {
            if (mStage != kStageOff) {
                return;
            }
            float attack = mSettings->mAttack;
            if (attack > 0.0f) {
                mStage = kStageAttack;
                float samples = attack * mSampleRate;
                mAttackRate = 1.0f / (samples == 0.0f ? 1.0f : samples);
            } else {
                LeavePeak();
            }
        }
        // Leaves the peak for the decay towards the sustain level, or for
        // the sustain level at once. Inlined into Attack and Advance. Name
        // not in the reference map.
        void LeavePeak() {
            if (mSettings->mSustain < 1.0f && mSettings->mDecay > 0.0f) {
                mLevel = 1.0f;
                mStage = kStageDecay;
                float samples = mSettings->mDecay * mSampleRate;
                mDecayRate = (1.0f - mSettings->mSustain) / (samples == 0.0f ? 1.0f : samples);
            } else {
                mLevel = mSettings->mSustain;
                mStage = kStageSustain;
            }
        }
        // Starts the release from the current level. Inlined into
        // FusionVoice::Release at 0x9E2E0. Name not in the reference map.
        void Release() {
            if (mStage == kStageRelease || mStage == kStageOff) {
                return;
            }
            float release = mSettings->mRelease;
            if (release > 0.0f) {
                mStage = kStageRelease;
                mReleaseRate = mLevel / (release * mSampleRate);
            } else {
                Stop();
            }
        }
        // Releases within kFastReleaseSeconds at most. Inlined into
        // FusionVoice::FastRelease at 0x9E380. Name not in the reference map.
        void FastRelease() {
            if (mStage == kStageOff) {
                return;
            }
            mStage = kStageRelease;
            float seconds = mSettings->mSustain * mSettings->mRelease;
            float samples = (seconds < kFastReleaseSeconds ? seconds : kFastReleaseSeconds) * mSampleRate;
            if (samples > 0.0f) {
                mReleaseRate = 1.0f / samples;
            } else {
                Stop();
            }
        }
        // Silences the envelope at once. Inlined into FusionVoice::Kill at
        // 0x9E440. Name not in the reference map.
        void Stop() {
            mLevel = 0.0f;
            mStage = kStageOff;
            mSamplesElapsed = 0;
        }

        // Longest fast release. Name not in the reference map.
        static constexpr float kFastReleaseSeconds = 0.025f;

        // Field names are not in the reference map.
        float mSampleRate;
        float mLevel;
        Stage mStage;
        // Samples advanced since the envelope started; Stop clears it.
        // FusionVoicePool steals the voice with the larger count first.
        unsigned int mSamplesElapsed;
        const Settings* mSettings;
        float mAttackRate;   // Level per sample.
        float mDecayRate;
        float mReleaseRate;
        // How GetValue applies the depth, in the order of
        // ModulatorTarget::DepthMode: additive, subtractive or centered.
        // Zeroed by the constructor.
        int mDepthMode;
    };
};

// The editors' limits of the settings' fields, at 0x19B00FC through
// 0x19B0134. Nothing in this build reads them.
extern SPL::ParameterSpec kAttackTimeParamSpec;
extern SPL::ParameterSpec kDecayTimeParamSpec;
extern SPL::ParameterSpec kReleaseTimeParamSpec;
extern SPL::ParameterSpec kSustainLevelParamSpec;
extern SPL::ParameterSpec kDepthLevelParamSpec;

static_assert(offsetof(ADSR::Settings, mAttack) == 4);
static_assert(offsetof(ADSR::Settings, mDepth) == 20);
static_assert(sizeof(ADSR::Settings) == 24);
static_assert(offsetof(ADSR::State, mStage) == 8);
static_assert(offsetof(ADSR::State, mSamplesElapsed) == 12);
static_assert(offsetof(ADSR::State, mSettings) == 16);
static_assert(offsetof(ADSR::State, mAttackRate) == 24);
static_assert(offsetof(ADSR::State, mDepthMode) == 36);
static_assert(sizeof(ADSR::State) == 40);
