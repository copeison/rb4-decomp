#pragma once

#include <cstddef>

#include "audio/core/dsp/BiquadFilter.h"
#include "audio/core/modulation/ADSR.h"
#include "audio/core/modulation/LFO.h"
#include "audio/core/modulation/Modulator.h"
#include "audio/core/resources/AudioSampleResource.h"
#include "entity/core/Component.h"
#include "entity/resources/Resource.h"
#include "utl/data/DataArray.h"

class FusionPatchResource;

// Sampler patch component (audio/FusionPatchCom.o). The component has not
// been reconstructed; only the settings FusionVoicePool and FusionSampler
// read are declared. Its property arrays are the map's PropArray<T>, of
// which only the storage pointer and the count are modelled.
class FusionPatchCom : public Component {
public:
    static Symbol sId;  // 0x19C8BB8
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x19C8BC0

    // The patch loaders FusionPatchResource uses; true when the patch
    // loaded. _LoadFromDTAFile reads the ".fusion" text file and passes it
    // to _LoadFromDataArray, whose name is not in the reference map.
    bool _LoadFromSXTFile(ResourcePath path);  // 0x83BB0
    bool _LoadFromDTAFile(ResourcePath path);  // 0x76B20
    bool _LoadFromDataArray(const DataArrayPtr& data, const ResourcePath& path);  // 0x76BB0
    // The patch in the ".fusion" text format.
    DataArrayPtr _SaveToDataArray();  // 0x79A50

    // The "portamento" struct. The map's PortamentoSettings::Mode is
    // FusionSampler::PortamentoMode here. Field names are not in the
    // reference map.
    struct PortamentoSettings {
        bool mEnabled;
        int mMode;
        float mTime;  // Seconds.
    };

    // One element of the "presets" array, the struct the registry names
    // "PresetSettings": the patch-wide sound a bank select chooses. Field
    // names follow the registry's properties; the map does not have them.
    struct PresetSettings {
        unsigned char mOpaque0[8];  // Not read by FusionSampler.
        float mVolume;              // "volume": the trim, in dB.
        float mPan;
        // The pitch bend ranges in cents, the downward one negative; the
        // registry's "max_downward_pitch_bend" and "max_upward_pitch_bend".
        float mMinPitchBendCents;
        float mMaxPitchBendCents;
        float mStartPointMs;     // "start_point"
        float mFineTuneCents;    // "fine_tune"
        int mMaxNumVoices;       // "max_num_voices"
        int mKeyzoneSelectMode;  // "keyzone_select_mode"
        PortamentoSettings mPortamento;
        BiquadFilter::Settings mFilter;
        // "delay"
        bool mDelayEnabled;
        bool mDelayBeatSync;
        float mDelayTime;
        float mDelayDefaultTempo;
        float mDelayDryGain;
        float mDelayWetGain;
        float mDelayFeedbackGain;
        // "distortion"
        bool mDistortionEnabled;
        float mDistortionInputGainDb;
        float mDistortionOutputGainDb;
        int mDistortionType;
        // "filter_1_pre_clip" to "filter_3_pre_clip", taken to be the three
        // bytes before the filters; FusionSampler does not read them.
        bool mDistortionFilterPreClip[3];
        BiquadFilter::Settings mDistortionFilters[3];
        bool mDistortionOversample;
        unsigned char mOpaque161[3];  // Padding as far as is known.
        // "bitcrusher"
        bool mBitCrusherEnabled;
        float mBitCrusherWet;          // Percent.
        float mBitCrusherCrushAmount;  // Bits.
        unsigned short mBitCrusherSampleAndHoldFactor;
        // "amp_simulation": the AmpSimulator parameters, then the model.
        float mAmpParameters[8];
        bool mAmpEnabled;
        int mAmpModel;
        BiquadFilter::Settings mAmpFilters[3];  // An EQ after the model.
        unsigned char mOpaque268[4];
        // The "adsrs", "lfos" and "modulators" arrays (PropArray storage).
        const void* mADSRsVtable;
        ADSR::Settings* mADSRs;
        unsigned int mNumADSRs;
        unsigned char mADSRsStorage[20];
        const void* mLFOsVtable;
        LFO::Settings* mLFOs;
        unsigned int mNumLFOs;
        unsigned char mLFOsStorage[20];
        const void* mModulatorsVtable;
        Modulator::Settings* mModulators;
        unsigned int mNumModulators;
        unsigned char mModulatorsStorage[20];
    };

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
        unsigned char mOpaque112[8];  // Not read by the sampler.

        // At 0x76A60.
        bool ContainsNoteAndVelocity(unsigned char note, unsigned char velocity) const;
    };

    // The component's fields. Names are not in the reference map.
    // The preset FusionSampler::LoadPatch starts from: the
    // "preset_options" "current" property, as far as is known.
    unsigned long mCurrentPreset;
    // The "presets" array.
    const void* mPresetsVtable;
    PresetSettings* mPresets;
    unsigned int mNumPresets;
    unsigned char mPresetsStorage[20];
    // The "keyzones" array; FusionSampler reads this object, which
    // KeyzoneArray models.
    const void* mKeyzonesVtable;
    KeyzoneSettings* mKeyzones;
    unsigned int mNumKeyzones;
    unsigned char mKeyzonesStorage[20];
    unsigned char mOpaque112[80];
    // The resource that created the component, set by
    // FusionPatchResource::CreateEntity (0x5C060).
    FusionPatchResource* mResource;
};

// The "keyzones" array object as FusionSampler's keyzone search
// (0x981F0) takes it: the map's PropArray<FusionPatchCom::KeyzoneSettings>.
// Name not in the reference map.
struct FusionKeyzoneArray {
    const void* mVtable;
    FusionPatchCom::KeyzoneSettings* mData;
    unsigned int mSize;
    unsigned char mStorage[20];
};

static_assert(sizeof(FusionKeyzoneArray) == 40);
static_assert(offsetof(FusionPatchCom::PresetSettings, mVolume) == 8);
static_assert(offsetof(FusionPatchCom::PresetSettings, mMinPitchBendCents) == 16);
static_assert(offsetof(FusionPatchCom::PresetSettings, mMaxNumVoices) == 32);
static_assert(offsetof(FusionPatchCom::PresetSettings, mKeyzoneSelectMode) == 36);
static_assert(offsetof(FusionPatchCom::PresetSettings, mPortamento) == 40);
static_assert(offsetof(FusionPatchCom::PresetSettings, mFilter) == 52);
static_assert(offsetof(FusionPatchCom::PresetSettings, mDelayEnabled) == 68);
static_assert(offsetof(FusionPatchCom::PresetSettings, mDelayTime) == 72);
static_assert(offsetof(FusionPatchCom::PresetSettings, mDelayDryGain) == 80);
static_assert(offsetof(FusionPatchCom::PresetSettings, mDistortionEnabled) == 92);
static_assert(offsetof(FusionPatchCom::PresetSettings, mDistortionType) == 104);
static_assert(offsetof(FusionPatchCom::PresetSettings, mDistortionFilters) == 112);
static_assert(offsetof(FusionPatchCom::PresetSettings, mDistortionOversample) == 160);
static_assert(offsetof(FusionPatchCom::PresetSettings, mBitCrusherEnabled) == 164);
static_assert(offsetof(FusionPatchCom::PresetSettings, mBitCrusherWet) == 168);
static_assert(offsetof(FusionPatchCom::PresetSettings, mBitCrusherSampleAndHoldFactor) == 176);
static_assert(offsetof(FusionPatchCom::PresetSettings, mAmpParameters) == 180);
static_assert(offsetof(FusionPatchCom::PresetSettings, mAmpEnabled) == 212);
static_assert(offsetof(FusionPatchCom::PresetSettings, mAmpModel) == 216);
static_assert(offsetof(FusionPatchCom::PresetSettings, mAmpFilters) == 220);
static_assert(offsetof(FusionPatchCom::PresetSettings, mADSRs) == 280);
static_assert(offsetof(FusionPatchCom::PresetSettings, mLFOs) == 320);
static_assert(offsetof(FusionPatchCom::PresetSettings, mModulators) == 360);
static_assert(sizeof(FusionPatchCom::PresetSettings) == 392);
static_assert(offsetof(FusionPatchCom, mCurrentPreset) == 24);
static_assert(offsetof(FusionPatchCom, mPresets) == 40);
static_assert(offsetof(FusionPatchCom, mNumPresets) == 48);
static_assert(offsetof(FusionPatchCom, mKeyzonesVtable) == 72);
static_assert(offsetof(FusionPatchCom, mNumKeyzones) == 88);
static_assert(offsetof(FusionPatchCom, mResource) == 192);

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
static_assert(sizeof(FusionPatchCom::KeyzoneSettings) == 120);
