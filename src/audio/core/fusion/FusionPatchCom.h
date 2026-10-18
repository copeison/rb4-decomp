#pragma once

#include <cstddef>

#include "audio/core/dsp/BiquadFilter.h"
#include "audio/core/modulation/ADSR.h"
#include "audio/core/modulation/LFO.h"
#include "audio/core/modulation/Modulator.h"
#include "audio/core/resources/AudioSampleResource.h"
#include "audio/core/resources/MoggResource.h"
#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropRegistry.h"
#include "entity/resources/Resource.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataArray.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class FusionPatchResource;

// A Fusion sampler patch (audio/FusionPatchCom.o, 0x67740 to 0x95C3F): the
// keyzones that map notes and velocities to samples, and the presets, each
// a patch-wide sound (trim, filter, envelopes, LFOs, modulators and
// effects) that a bank select chooses. The editor shows the current
// preset's settings as properties without storage, whose accessors read and
// write that preset; "preset_data" registers the presets themselves. Its
// class id is "FusionPatchCom". The vtable at 0x18E2D68 has Component's 41
// slots. The object is 232 bytes.
class FusionPatchCom : public Component {
public:
    // The "portamento" struct. Field names are not in the reference map.
    struct PortamentoSettings {
        // The map's PortamentoSettings::Mode; FusionSampler::PortamentoMode
        // has the same values.
        enum Mode : int {
            kModeLegato = 0,
            kModePersistent = 1,
        };

        // The "mode" values with their help. At 0x84D10.
        static eastl::vector<AllowedValue<int>> GetAllowedModeValues();

        bool mEnabled;
        int mMode;    // A Mode.
        float mTime;  // Seconds.
    };

    // One element of "preset_data", the struct the registry names
    // "PresetSettings": the patch-wide sound a bank select chooses. The
    // registry groups the members into the structs "portamento", "filter",
    // "delay", "distortion", "bitcrusher" and "amp_simulation". Field names
    // follow the registry's properties; the map does not have them.
    struct PresetSettings {
        // At 0x679D0: the "Default" preset, with the amplitude envelope, two
        // LFOs aimed at the pan and the pitch, and four modulators.
        PresetSettings();

        Symbol mName;   // "name"
        float mVolume;  // "volume": the trim, in dB.
        float mPan;
        // The pitch bend ranges in cents, the downward one negative; the
        // properties "max_downward_pitch_bend" and "max_upward_pitch_bend"
        // show them in semitones.
        float mMinPitchBendCents;
        float mMaxPitchBendCents;
        float mStartPointMs;     // "start_point"
        float mFineTuneCents;    // "fine_tune_cents"; the editor's "fine_tune".
        int mMaxNumVoices;       // "max_num_voices"
        int mKeyzoneSelectMode;  // "keyzone_select_mode": a FusionSampler::KeyzoneSelectMode.
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
        int mDistortionType;  // A DistortionEffect::Settings::Type.
        // "filter_1_pre_clip" to "filter_3_pre_clip": each filter runs before
        // the clipper rather than after it.
        bool mDistortionFilterPreClip[3];
        BiquadFilter::Settings mDistortionFilters[3];  // "filter_1" to "filter_3"
        bool mDistortionOversample;
        // "bitcrusher", a struct that starts on a 4-byte boundary.
        alignas(4) bool mBitCrusherEnabled;
        float mBitCrusherWet;          // Percent.
        float mBitCrusherCrushAmount;  // Bits.
        int mBitCrusherSampleAndHoldFactor;
        // "amp_simulation": the "amp_model" struct, whose AmpSimulator
        // parameters "presence" to "output_gain" come first, then the EQ
        // after the model.
        float mAmpParameters[8];
        bool mAmpEnabled;
        int mAmpModel;  // "model_type": an AmpSimulator::Model.
        BiquadFilter::Settings mAmpFilters[3];
        PropArray<ADSR::Settings> mADSRs;          // "adsrs"
        PropArray<LFO::Settings> mLFOs;            // "lfos"
        PropArray<Modulator::Settings> mModulators;  // "modulators"
    };

    // One key and velocity range of a patch, an element of the "keyzones"
    // array (the struct the registry names "KeyzoneSettings"). Field names
    // follow the properties the registry (0x6C950) binds to their offsets;
    // they are not in the reference map.
    struct KeyzoneSettings {
        // The defaults PropArray<KeyzoneSettings> constructs (0x8A1F0): the
        // whole range from root note 60, priority 50, a random weight of 50
        // and a sampled tempo of 120.
        KeyzoneSettings()
            : mRootNote(60),
              mMinNote(0),
              mMaxNote(127),
              mMinVelocity(0),
              mMaxVelocity(127),
              mPad5(0),
              mPriority(50),
              mPan(0.0f),
              mLeftGain(0.5f),
              mRightGain(0.5f),
              mFineTune(0.0f),
              mFineTuneRatio(1.0f),
              mVolume(0.0f),
              mVolumeGain(1.0f),
              mUnpitched(false),
              mVelocityToVolume(true),
              mMaintainTime(false),
              mFormantMode(2),
              mTimeStretchAlgorithm(1),
              mTempoSync(false),
              mSampledTempo(120),
              mTriggerOnNoteOff(false),
              mRandomWeight(50.0f),
              mTrackMap(),
              mSample(nullptr),
              mSamplePath() {}

        // At 0x76A60.
        bool ContainsNoteAndVelocity(unsigned char note, unsigned char velocity) const;

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
        // applies to its pan mix: 0.5 -/+ half the pan, set with it by the
        // text loader's "pan_right" (the map's SetPanRight). The "pan"
        // property's setter does not update them.
        float mLeftGain;
        float mRightGain;
        float mFineTune;  // Cents.
        // exp2(mFineTune / 1200), stored by the "fine_tune" setter: the
        // read-only "fine_tune_adjustment".
        float mFineTuneRatio;
        float mVolume;  // dB.
        // 10^(mVolume / 20), stored by the "volume" setter.
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
        // "track_map": the Mogg tracks the keyzone's channels play, one per
        // MIDI controller from 52.
        PropArray<TrackChannelInfo> mTrackMap;
        // The loaded sample, owned by the component's mSampleResources.
        AudioSampleResource* mSample;
        ResourcePath mSamplePath;  // "sample"
    };

    // Copies the properties, not the resource or the loaded samples.
    // Inlined into _Imprint; the map has it in the object.
    FusionPatchCom(const FusionPatchCom& other)
        : Component(other),
          mCurrentPreset(other.mCurrentPreset),
          mPresets(other.mPresets),
          mKeyzones(other.mKeyzones),
          mPairSlots(other.mPairSlots),
          mModulatorSlots(other.mModulatorSlots),
          mResource(nullptr),
          mSampleResources() {}

    FusionPatchCom();            // 0x67740
    ~FusionPatchCom() override;  // slots 0-1: 0x67D70, 0x68020

    // Component slots.
    Symbol GetId() const override;         // slot 4: 0x85710
    Symbol GetClassName() const override;  // slot 5: 0x85720
    int CurrentRev() const override;       // slot 7: 0x85730
    bool IsA(Symbol type) const override;  // slot 8: 0x85750
    Component* AsComponent() override;     // slot 9: 0x85780
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x85790
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x859F0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x85A00
    // Slot 29 at 0x84320: loads each keyzone's sample. The map's
    // _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;

    // Appends a keyzone of the default settings playing the sample at the
    // path, loading it when the file exists, and returns it. The resource's
    // entity hears "pre_create_keyzone" with the new index and
    // "post_create_keyzone". The map's signature returns nothing. At
    // 0x84EF0.
    KeyzoneSettings* AddKeyZone(const char* path);
    // The same with the ranges and options; null when no keyzone was added.
    // The map's overload takes the five notes and velocities and two flags;
    // this build adds the time-stretch settings, the tempo sync and the
    // sampled tempo, and an int it does not read. No caller in this build.
    // At 0x84E80.
    KeyzoneSettings* AddKeyZone(
        const char* path,
        signed char minNote,
        signed char maxNote,
        signed char rootNote,
        signed char minVelocity,
        signed char maxVelocity,
        bool unpitched,
        bool velocityToVolume,
        bool maintainTime,
        int timeStretchAlgorithm,
        int formantMode,
        int unused,
        bool tempoSync,
        short sampledTempo);
    // Appends a copy of the keyzone. At 0x853B0.
    void CloneKeyZone(unsigned long index);
    // Silences the keyzone's voices and removes it, between the
    // "pre_destroy_keyzone" and "post_destroy_keyzone" messages. At 0x85420.
    void DeleteKeyZone(unsigned long index);
    // Sends "keyzone_property_changed" with the index to the resource's
    // entity and marks the resource for reloading. At 0x848C0.
    void _KeyZonePropertyChanged(unsigned long index);
    // The sample at the path: a Mogg sample for ".mogg" files, otherwise a
    // WaveResource. The component is not read. Name not in the reference
    // map. At 0x84600.
    ResourcePtr<AudioSampleResource> _LoadSample(ResourcePath path) const;
    // The path relative to the patch file's folder, or "" without a
    // resource. At 0x76710.
    String _GetPatchRelativePath(ResourcePath path) const;
    // The sample path relative to the patch file's folder as a resource
    // path. At 0x76860.
    ResourcePath _GetFullPathFromSample(const String& sample) const;

    // The patch loaders FusionPatchResource uses; true when the patch
    // loaded. _LoadFromDTAFile reads the ".fusion" text file and passes it
    // to _LoadFromDataArray, whose name is not in the reference map.
    // _LoadFromDataArray reads the "version", the "keymap" keyzones (the
    // pan as "pan_right", the map's KeyzoneSettings::SetPanRight) and the
    // presets; it and the ".sxt" loader are not reconstructed.
    bool _LoadFromSXTFile(ResourcePath path);  // 0x83BB0
    bool _LoadFromDTAFile(ResourcePath path);  // 0x76B20
    bool _LoadFromDataArray(const DataArrayPtr& data, const ResourcePath& path);  // 0x76BB0
    // The patch in the ".fusion" text format. Not reconstructed.
    DataArrayPtr _SaveToDataArray();  // 0x79A50

    // The "keyzone_select_mode" values with their help. At 0x84A80.
    static eastl::vector<AllowedValue<int>> GetAllowedKeyzoneSelectModeValues();

    // Registers the class: the class's Init shape without the superclass
    // flag. Inline in the map; this build emits it in audio/SoundManager.o
    // at 0x5350.
    static void Init();
    // The class factory. Emitted in audio/SoundManager.o at 0xD880.
    static Component* _Create();
    // Registers the struct types and the properties, and sets the class
    // description.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x68040

    static Symbol sId;  // 0x19C8BB8, "FusionPatchCom"
    // The class symbol GameObject::CreateComponent takes; also
    // "FusionPatchCom". Name not in the reference map.
    static Symbol sClassName;           // 0x19C8BC0
    static PropRegistry sPropRegistry;  // 0x19C8BD0
    static ComMetaData sMetaData;       // 0x19C8C70

    // The component's fields. Names are not in the reference map.
    // The preset the editor shows and FusionSampler::LoadPatch starts from:
    // "preset_options"' "current".
    unsigned long mCurrentPreset;
    PropArray<PresetSettings> mPresets;    // "preset_data"
    PropArray<KeyzoneSettings> mKeyzones;  // "keyzones"
    // Two placeholder elements behind the editor's "adsrs" and "lfos"
    // arrays, and four behind "modulators": their members are accessors of
    // the current preset's element of the same index.
    PropArray<int> mPairSlots;
    PropArray<int> mModulatorSlots;
    // The resource that created the component, set by
    // FusionPatchResource::CreateEntity (0x5C060).
    FusionPatchResource* mResource;
    // The keyzones' samples, which the keyzones point at.
    eastl::vector<ResourcePtr<AudioSampleResource>> mSampleResources;
};

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
static_assert(offsetof(FusionPatchCom::PresetSettings, mDistortionFilterPreClip) == 108);
static_assert(offsetof(FusionPatchCom::PresetSettings, mDistortionFilters) == 112);
static_assert(offsetof(FusionPatchCom::PresetSettings, mDistortionOversample) == 160);
static_assert(offsetof(FusionPatchCom::PresetSettings, mBitCrusherEnabled) == 164);
static_assert(offsetof(FusionPatchCom::PresetSettings, mBitCrusherWet) == 168);
static_assert(offsetof(FusionPatchCom::PresetSettings, mBitCrusherSampleAndHoldFactor) == 176);
static_assert(offsetof(FusionPatchCom::PresetSettings, mAmpParameters) == 180);
static_assert(offsetof(FusionPatchCom::PresetSettings, mAmpEnabled) == 212);
static_assert(offsetof(FusionPatchCom::PresetSettings, mAmpModel) == 216);
static_assert(offsetof(FusionPatchCom::PresetSettings, mAmpFilters) == 220);
static_assert(offsetof(FusionPatchCom::PresetSettings, mADSRs) == 272);
static_assert(offsetof(FusionPatchCom::PresetSettings, mLFOs) == 312);
static_assert(offsetof(FusionPatchCom::PresetSettings, mModulators) == 352);
static_assert(sizeof(FusionPatchCom::PresetSettings) == 392);

static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mPriority) == 6);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mPan) == 8);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mLeftGain) == 12);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mFineTune) == 20);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mVolume) == 28);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mVolumeGain) == 32);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mUnpitched) == 36);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mMaintainTime) == 40);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mFormantMode) == 44);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mTempoSync) == 52);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mSampledTempo) == 54);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mTriggerOnNoteOff) == 56);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mRandomWeight) == 60);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mTrackMap) == 64);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mSample) == 104);
static_assert(offsetof(FusionPatchCom::KeyzoneSettings, mSamplePath) == 112);
static_assert(sizeof(FusionPatchCom::KeyzoneSettings) == 120);

static_assert(offsetof(FusionPatchCom, mCurrentPreset) == 24);
static_assert(offsetof(FusionPatchCom, mPresets) == 32);
static_assert(offsetof(FusionPatchCom, mKeyzones) == 72);
static_assert(offsetof(FusionPatchCom, mPairSlots) == 112);
static_assert(offsetof(FusionPatchCom, mModulatorSlots) == 152);
static_assert(offsetof(FusionPatchCom, mResource) == 192);
static_assert(offsetof(FusionPatchCom, mSampleResources) == 200);
static_assert(sizeof(FusionPatchCom) == 232);
