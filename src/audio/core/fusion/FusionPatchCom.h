#pragma once

#include <cstddef>

#include "audio/core/resources/AudioSampleResource.h"
#include "audio/core/resources/Resource.h"

// Sampler patch component (audio/FusionPatchCom.o). The component has not
// been reconstructed; only the keyzone settings FusionVoicePool reads are
// declared.
class FusionPatchCom {
public:
    // One key and velocity range of a patch, an element of the "keyzones"
    // array. Field names are not in the reference map; they follow the
    // properties the registry (0x6C950) binds to their offsets.
    struct KeyzoneSettings {
        unsigned char mRootNote;
        unsigned char mMinNote;
        unsigned char mMaxNote;
        unsigned char mMinVelocity;
        unsigned char mMaxVelocity;
        unsigned char mPad5;  // No property and no reader.
        // "priority", the voice-stealing rank: a voice whose rank is zero,
        // or lower than the new note's, is never stolen for it.
        unsigned char mPriority;
        float mPan;
        // Gains of the sample's left and right channels, which FusionVoice
        // applies to its pan mix. No property sets them; that they derive
        // from "pan" is an inference.
        float mLeftGain;
        float mRightGain;
        float mFineTune;  // Cents.
        // exp2(mFineTune / 1200), stored by the "fine_tune" setter
        // (0x87C30).
        float mFineTuneRatio;
        float mVolume;  // dB.
        // 10^(mVolume / 20), stored by the "volume" setter (0x87B40).
        float mVolumeGain;
        bool mUnpitched;
        bool mVelocityToVolume;
        // "maintain_time": the voice keeps its duration while shifting
        // pitch, so it needs one of the pool's shifters.
        alignas(4) bool mMaintainTime;
        // Handed to SmbPitchShift::SetTimeStretchMode: the formant mode (1
        // with "maintain_formant", else 2) and the "algorithm" index.
        int mFormantMode;
        int mTimeStretchAlgorithm;
        bool mTempoSync;
        short mSampledTempo;
        bool mTriggerOnNoteOff;
        float mRandomWeight;
        // The "track_map" array object; its layout is not modelled.
        unsigned char mTrackMap[40];
        ResourcePtr<AudioSampleResource> mSample;
    };
};

static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mPriority) == 6);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mPan) == 8);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mLeftGain) == 12);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mFineTune) == 20);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mVolumeGain) == 32);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mUnpitched) == 36);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mMaintainTime) == 40);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mTempoSync) == 52);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mSampledTempo) == 54);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mTriggerOnNoteOff) == 56);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mRandomWeight) == 60);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mTrackMap) == 64);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mFormantMode) == 44);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mSample) == 104);
