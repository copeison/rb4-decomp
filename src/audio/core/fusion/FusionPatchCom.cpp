// audio/FusionPatchCom.o (0x67740 to 0x95C3F). The property accessors and
// tag functions of the registry builders (0x863A0 to 0x897D0, and the
// std::function bodies from 0x8DCB0 to 0x95A50) are the builders' lambdas;
// the PropArray element operations (0x85C50 to 0x8A9B0) are the template's.
// Of the patch loaders only _LoadFromDTAFile is reconstructed.
#include "audio/core/fusion/FusionPatchCom.h"

#include <cmath>
#include <cstring>
#include <new>

#include "audio/core/dsp/AmpSimulator.h"
#include "audio/core/dsp/DistortionEffect.h"
#include "audio/core/fusion/FusionVoicePool.h"
#include "audio/core/modulation/ModulatorTarget.h"
#include "audio/core/output/AudioRenderTarget.h"
#include "audio/core/resources/FusionPatchResource.h"
#include "audio/core/resources/MoggSampleResource.h"
#include "audio/core/resources/WaveResource.h"
#include "entity/core/Entity.h"
#include "math/interp/Interp.h"
#include "os/files/File.h"
#include "os/memory/MemMgr.h"
#include "utl/data/DataFile.h"
#include "utl/files/FileUtl.h"
#include "utl/messages/Message.h"
#include "utl/text/MakeString.h"

namespace {

using PresetSettings = FusionPatchCom::PresetSettings;
using KeyzoneSettings = FusionPatchCom::KeyzoneSettings;

// The preset the editor's preset properties read and write: the current
// one. Inlined into every accessor and tag function. Name not in the
// reference map.
PresetSettings& CurrentPreset(const Component& component) {
    const FusionPatchCom& patch = static_cast<const FusionPatchCom&>(component);
    return patch.mPresets[patch.mCurrentPreset];
}
PresetSettings& CurrentPreset(const PropAccessorArgs& args) {
    return CurrentPreset(*args.mComponent);
}
PresetSettings& CurrentPreset(const TagArgs& args) {
    return CurrentPreset(args.mComponent);
}

// The value of the path's node `back` places from its end: the index of an
// element the path passes through. Name not in the reference map.
unsigned long PathIndex(const PropPath& path, int back) {
    return path.mNodes[static_cast<int>(path.mSize) - back].mValue;
}
// The index of the element whose member the path names: the node before
// the last. Names not in the reference map.
unsigned long ElementIndex(const PropAccessorArgs& args) {
    return PathIndex(*static_cast<const PropPath*>(args.mPath), 2);
}
unsigned long ElementIndex(const TagArgs& args) {
    return PathIndex(args.mPath, 2);
}

// The value a setter receives. Name not in the reference map.
template <class T>
const T& NewValue(const PropAccessorArgs& args) {
    return *static_cast<const T*>(args.mValue);
}

// Marks the patch's resource for reloading after an edit: the change
// callback of most properties. Name not in the reference map.
void MarkPatchChanged(const PropAccessorArgs& args) {
    FusionPatchResource* const resource = static_cast<FusionPatchCom*>(args.mComponent)->mResource;
    if (resource != nullptr) {
        resource->mFileChangedOnDisk = true;
    }
}

// The current preset's element of an editor array that the accessor or
// tag reaches. Names not in the reference map.
ADSR::Settings& ElementADSR(const PropAccessorArgs& args) {
    return CurrentPreset(args).mADSRs[ElementIndex(args)];
}
ADSR::Settings& ElementADSR(const TagArgs& args) {
    return CurrentPreset(args).mADSRs[ElementIndex(args)];
}
LFO::Settings& ElementLFO(const PropAccessorArgs& args) {
    return CurrentPreset(args).mLFOs[ElementIndex(args)];
}
LFO::Settings& ElementLFO(const TagArgs& args) {
    return CurrentPreset(args).mLFOs[ElementIndex(args)];
}
Modulator::Settings& ElementModulator(const PropAccessorArgs& args) {
    return CurrentPreset(args).mModulators[ElementIndex(args)];
}
Modulator::Settings& ElementModulator(const TagArgs& args) {
    return CurrentPreset(args).mModulators[ElementIndex(args)];
}
// The keyzone whose member the accessor or tag reaches. Names not in the
// reference map.
KeyzoneSettings& ElementKeyzone(const PropAccessorArgs& args) {
    return static_cast<FusionPatchCom*>(args.mComponent)->mKeyzones[ElementIndex(args)];
}
KeyzoneSettings& ElementKeyzone(const TagArgs& args) {
    return static_cast<const FusionPatchCom&>(args.mComponent).mKeyzones[ElementIndex(args)];
}

// Reconstructed from eboot.elf at 0x91330. A note's name and octave,
// "(C4)" for 60, from the first of the function's three name tables (local
// statics from 0x19C6AD0). Name not in the reference map.
const char* NoteName(unsigned char note) {
    static const char* const kNames[12] = {
        "C", "C#/Db", "D", "D#/Eb", "E", "F", "F#/Gb", "G", "G#/Ab", "A", "A#/Bb", "B"};
    FormatString format("(%s%d)");
    format << kNames[note % 12] << static_cast<unsigned int>(note / 12 - 1);
    return format.Str();
}

// Removes the array's element, as the keyzone deletion and the preset
// removal inline it. Name not in the reference map.
void RemoveElement(PropArrayBase& array, unsigned long index) {
    array.Destruct(1, array.ElementAt(index));
    if (index < array.mSize - 1) {
        array.Move(array.mSize - 1 - index, array.ElementAt(index), array.ElementAt(index + 1));
    }
    --array.mSize;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x95B70).
// The int it first sets to -1 (0x19C8BB0) comes from a shared header and is
// not modelled.
Symbol FusionPatchCom::sId("FusionPatchCom");
Symbol FusionPatchCom::sClassName("FusionPatchCom");
PropRegistry FusionPatchCom::sPropRegistry;
ComMetaData FusionPatchCom::sMetaData;

// Reconstructed from eboot.elf at 0x67740. The patch starts with the
// default preset, whose first ADSR is the amplitude envelope at full depth.
FusionPatchCom::FusionPatchCom() : mCurrentPreset(0), mResource(nullptr) {
    const PresetSettings preset;
    mPresets._Insert(mPresets.size(), &preset);
    mModulatorSlots.Resize(4);
    mPairSlots.Resize(2);
    ADSR::Settings& amp = mPresets[mCurrentPreset].mADSRs[0];
    amp.mTarget = ADSR::Settings::kTargetVolume;
    amp.mDepth = 1.0f;
    amp.mEnabled = true;
}

// Reconstructed from eboot.elf at 0x679D0. The LFOs default to the pan and
// the pitch.
FusionPatchCom::PresetSettings::PresetSettings()
    : mName(),
      mVolume(0.0f),
      mPan(0.0f),
      mMinPitchBendCents(-200.0f),
      mMaxPitchBendCents(200.0f),
      mStartPointMs(0.0f),
      mFineTuneCents(0.0f),
      mMaxNumVoices(32),
      mKeyzoneSelectMode(0),
      mPortamento{false, PortamentoSettings::kModeLegato, 1.0f},
      mFilter(),
      mDelayEnabled(false),
      mDelayBeatSync(false),
      mDelayTime(0.0f),
      mDelayDefaultTempo(120.0f),
      mDelayDryGain(0.5f),
      mDelayWetGain(0.5f),
      mDelayFeedbackGain(0.0f),
      mDistortionEnabled(false),
      mDistortionInputGainDb(0.0f),
      mDistortionOutputGainDb(0.0f),
      mDistortionType(0),
      mDistortionFilterPreClip{false, false, false},
      mDistortionFilters(),
      mDistortionOversample(false),
      mBitCrusherEnabled(false),
      mBitCrusherWet(0.0f),
      mBitCrusherCrushAmount(0.0f),
      mBitCrusherSampleAndHoldFactor(1),
      mAmpParameters{6.0f, 7.0f, 4.0f, 7.0f, 6.0f, 6.0f, 0.0f, 10.0f},
      mAmpEnabled(false),
      mAmpModel(0),
      mAmpFilters() {
    static Symbol sDefaultName;
    if (sDefaultName == Symbol()) {
        sDefaultName = Symbol("Default");
    }
    mName = sDefaultName;
    mADSRs.Resize(2);
    mLFOs.Resize(2);
    mModulators.Resize(4);
    mLFOs[0].mTarget = LFO::Settings::kTargetPan;
    mLFOs[1].mTarget = LFO::Settings::kTargetPitch;
}

// Reconstructed from eboot.elf at 0x67D70. The samples are released, then
// the arrays are destroyed.
FusionPatchCom::~FusionPatchCom() {}

// Reconstructed from eboot.elf at 0x76A60. The binary reads the note and
// velocity limits as signed bytes (the map's AddKeyZone takes them as signed
// char), so the casts keep its comparisons.
bool FusionPatchCom::KeyzoneSettings::ContainsNoteAndVelocity(
    unsigned char note, unsigned char velocity) const {
    return note >= static_cast<signed char>(mMinNote) && note <= static_cast<signed char>(mMaxNote) &&
           velocity >= static_cast<signed char>(mMinVelocity) &&
           velocity <= static_cast<signed char>(mMaxVelocity);
}

namespace {

// The registry builders _Init calls, in its order. Each registers a group
// of properties, most of them accessors of the current preset without
// storage. Every property registration builds a "prop" symbol it does not
// use, and the struct and array registrations "struct", "array" and
// "struct_item". Names not in the reference map.

// Reconstructed from eboot.elf at 0x68D60. The current preset's scalar
// settings. The pitch bend ranges show in semitones; the maximum voice count
// is read-only while portamento makes the patch monophonic.
void RegisterPresetProps(PropRegistry& registry) {
    const float volumeType = 0.0f;
    FloatMetadata& volume =
        TypeSpecificMetadata(registry.RegisterProp("volume", -1, kPropertyFloat, 0), volumeType);
    static_cast<void>(Symbol("prop"));
    volume.mDisplayName = String("trim");
    volume.mHelp = String("The volume expression (CC11) of this channel (in dB).");
    volume.mSlider = true;
    volume.mMin = -96.0f;
    volume.mMax = 0.0f;
    volume.mUnits = Symbol("dB");
    volume.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    volume.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mVolume; };
    volume.mSetter = [](const PropAccessorArgs& args) { CurrentPreset(args).mVolume = NewValue<float>(args); };

    const float panType = 0.0f;
    FloatMetadata& pan = TypeSpecificMetadata(registry.RegisterProp("pan", -1, kPropertyFloat, 0), panType);
    static_cast<void>(Symbol("prop"));
    pan.mHelp = String("The left/right pan value for this patch.");
    pan.mSlider = true;
    pan.mMin = -1.0f;
    pan.mMax = 1.0f;
    pan.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    pan.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mPan; };
    pan.mSetter = [](const PropAccessorArgs& args) { CurrentPreset(args).mPan = NewValue<float>(args); };

    const float upwardType = 0.0f;
    FloatMetadata& upward =
        TypeSpecificMetadata(registry.RegisterProp("max_upward_pitch_bend", -1, kPropertyFloat, 0), upwardType);
    static_cast<void>(Symbol("prop"));
    upward.mDisplayName = String("upward");
    upward.mHelp = String("Maximum upward pitch bend amount.");
    upward.mMin = 0.0f;
    upward.mMax = 24.0f;
    upward.mSlider = true;
    upward.mUnits = Symbol("semitones");
    upward.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    upward.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mMaxPitchBendCents = NewValue<float>(args) * 100.0f;
    };
    upward.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mMaxPitchBendCents * 0.01f; };

    const float downwardType = 0.0f;
    FloatMetadata& downward = TypeSpecificMetadata(
        registry.RegisterProp("max_downward_pitch_bend", -1, kPropertyFloat, 0), downwardType);
    static_cast<void>(Symbol("prop"));
    downward.mDisplayName = String("downward");
    downward.mHelp = String("Maximum downward pitch bend amount.");
    downward.mMin = 0.0f;
    downward.mMax = 24.0f;
    downward.mSlider = true;
    downward.mUnits = Symbol("semitones");
    downward.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    downward.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mMinPitchBendCents = NewValue<float>(args) * -100.0f;
    };
    downward.mGetter = [](const PropAccessorArgs& args) {
        return CurrentPreset(args).mMinPitchBendCents * -0.01f;
    };

    const int voicesType = 0;
    IntMetadata<int>& voices =
        TypeSpecificMetadata(registry.RegisterProp("max_num_voices", -1, kPropertyInt, 0), voicesType);
    static_cast<void>(Symbol("prop"));
    voices.mDisplayName = String("polyphony");
    voices.mHelp = String(
        "The maximum number of active voices this channel can use simultaneously (note that released "
        "voices that are in the decay phase do not count toward this limit).");
    voices.mReadOnly._Set([](const TagArgs& args) { return CurrentPreset(args).mPortamento.mEnabled; });
    voices.mMin = 1;
    voices.mMax = 100;
    voices.mUnits = Symbol("Voices");
    voices.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    voices.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mMaxNumVoices; };
    voices.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mMaxNumVoices = NewValue<int>(args);
    };

    const float startType = 0.0f;
    FloatMetadata& start =
        TypeSpecificMetadata(registry.RegisterProp("start_point", -1, kPropertyFloat, 0), startType);
    static_cast<void>(Symbol("prop"));
    start.mSlider = true;
    start.mHelp =
        String("The offset (in ms) from the beginning of the sample, from which to start playback.");
    start.mMin = 0.0f;
    start.mMax = 1000.0f;
    start.mUnits = Symbol("ms");
    start.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    start.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mStartPointMs; };
    start.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mStartPointMs = NewValue<float>(args);
    };

    const float tuneType = 0.0f;
    FloatMetadata& tune = TypeSpecificMetadata(registry.RegisterProp("fine_tune", -1, kPropertyFloat, 0), tuneType);
    static_cast<void>(Symbol("prop"));
    tune.mSlider = true;
    tune.mHelp = String("The fine tuning offset for this patch, in cents.");
    tune.mMin = -100.0f;
    tune.mMax = 100.0f;
    tune.mUnits = Symbol("cents");
    tune.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    tune.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mFineTuneCents; };
    tune.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mFineTuneCents = NewValue<float>(args);
    };

    const int modeType = 0;
    IntMetadata<int>& mode =
        TypeSpecificMetadata(registry.RegisterProp("keyzone_select_mode", -1, kPropertyInt, 0), modeType);
    static_cast<void>(Symbol("prop"));
    mode.mHelp = String("How overlapping keyzones are dealt with...");
    mode.mAllowedValues._Set([](const TagArgs& args) {
        static_cast<void>(args);
        return FusionPatchCom::GetAllowedKeyzoneSelectModeValues();
    });
    mode.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    mode.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mKeyzoneSelectMode; };
    mode.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mKeyzoneSelectMode = NewValue<int>(args);
    };
}

// The "%s %d" name of an editor array's element. Inlined into the display
// name functions. Name not in the reference map.
String ElementName(const char* name, unsigned long index) {
    FormatString format("%s %d");
    format << name << index;
    return String(format.Str());
}

// Reconstructed from eboot.elf at 0x69760. The current preset's two ADSRs,
// edited through the placeholder array. The first is the amplitude
// envelope: always enabled, aimed at the volume, at a fixed depth.
void RegisterADSRProps(PropRegistry& registry) {
    PropInfo& array = registry.RegisterPropArray(
        "adsrs", offsetof(FusionPatchCom, mPairSlots), sizeof(PropArray<int>));
    ArrayMetadata& arrayMetadata = static_cast<ArrayMetadata&>(*array.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    arrayMetadata.mFixedSize = true;
    PropRegistry::RegisterPropArrayItem(array, kPropertyStruct, sizeof(int));
    PropRegistry& members =
        static_cast<GroupMetadata&>(*arrayMetadata.mItemInfo.mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct_item"));

    const bool enabledType = false;
    BoolMetadata& enabled =
        TypeSpecificMetadata(members.RegisterProp("enabled", -1, kPropertyBool, 0), enabledType);
    static_cast<void>(Symbol("prop"));
    enabled.mDisplayName._Set([](const TagArgs& args) { return ElementName("ADSR", ElementIndex(args)); });
    enabled.mHelp = String("Is this ADSR enabled and affecting audio output?");
    enabled.mReadOnly._Set([](const TagArgs& args) { return ElementIndex(args) == 0; });
    enabled.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    enabled.mGetter = [](const PropAccessorArgs& args) { return ElementADSR(args).mEnabled; };
    enabled.mSetter = [](const PropAccessorArgs& args) { ElementADSR(args).mEnabled = NewValue<bool>(args); };

    const unsigned char targetType = 0;
    IntMetadata<unsigned char>& target =
        TypeSpecificMetadata(members.RegisterProp("target", -1, kPropertyUInt8, 0), targetType);
    static_cast<void>(Symbol("prop"));
    target.mHelp = String("The property being affected by this ADSR.");
    target.mAllowedValues._Set([](const TagArgs& args) {
        if (ElementIndex(args) != 0) {
            return ADSR::Settings::GetAssignableAllowedTargetValues();
        }
        return ADSR::Settings::GetVolumeAllowedTargetValues();
    });
    target.mReadOnly._Set([](const TagArgs& args) { return !ElementADSR(args).mEnabled; });
    target.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    target.mGetter = [](const PropAccessorArgs& args) -> unsigned char { return ElementADSR(args).mTarget; };
    target.mSetter = [](const PropAccessorArgs& args) {
        ElementADSR(args).mTarget = static_cast<ADSR::Settings::Target>(NewValue<unsigned char>(args));
    };

    const float depthType = 0.0f;
    FloatMetadata& depth = TypeSpecificMetadata(members.RegisterProp("depth", -1, kPropertyFloat, 0), depthType);
    static_cast<void>(Symbol("prop"));
    depth.mSlider = true;
    depth.mHelp = String("The depth of the effect produced by this ADSR.");
    depth.mMin = 0.0f;
    depth.mMax = 1.0f;
    depth.mReadOnly._Set([](const TagArgs& args) {
        return ElementIndex(args) == 0 || !ElementADSR(args).mEnabled;
    });
    depth.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    depth.mGetter = [](const PropAccessorArgs& args) { return ElementADSR(args).mDepth; };
    depth.mSetter = [](const PropAccessorArgs& args) { ElementADSR(args).mDepth = NewValue<float>(args); };

    const float attackType = 0.0f;
    FloatMetadata& attack = TypeSpecificMetadata(members.RegisterProp("attack", -1, kPropertyFloat, 0), attackType);
    static_cast<void>(Symbol("prop"));
    attack.mSlider = true;
    attack.mHelp = String("The attack time of this ADSR, in seconds.");
    attack.mMin = 0.0f;
    attack.mMax = 10.0f;
    attack.mSliderLogResolution = LogInterpolator::CalculateBiasForMid(0.0f, 1.0f, 10.0f);
    attack.mUnits = Symbol("seconds");
    attack.mReadOnly._Set([](const TagArgs& args) { return !ElementADSR(args).mEnabled; });
    attack.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    attack.mGetter = [](const PropAccessorArgs& args) { return ElementADSR(args).mAttack; };
    attack.mSetter = [](const PropAccessorArgs& args) { ElementADSR(args).mAttack = NewValue<float>(args); };

    const float decayType = 0.0f;
    FloatMetadata& decay = TypeSpecificMetadata(members.RegisterProp("decay", -1, kPropertyFloat, 0), decayType);
    static_cast<void>(Symbol("prop"));
    decay.mSlider = true;
    decay.mHelp = String("The decay time of this ADSR in seconds.");
    decay.mMin = 0.0f;
    decay.mMax = 10.0f;
    decay.mSliderLogResolution = LogInterpolator::CalculateBiasForMid(0.0f, 1.0f, 10.0f);
    decay.mUnits = Symbol("seconds");
    decay.mReadOnly._Set([](const TagArgs& args) { return !ElementADSR(args).mEnabled; });
    decay.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    decay.mGetter = [](const PropAccessorArgs& args) { return ElementADSR(args).mDecay; };
    decay.mSetter = [](const PropAccessorArgs& args) { ElementADSR(args).mDecay = NewValue<float>(args); };

    const float sustainType = 0.0f;
    FloatMetadata& sustain =
        TypeSpecificMetadata(members.RegisterProp("sustain", -1, kPropertyFloat, 0), sustainType);
    static_cast<void>(Symbol("prop"));
    sustain.mSlider = true;
    sustain.mHelp = String("The sustain level of this ADSR, as a fractional amount.");
    sustain.mMin = 0.0f;
    sustain.mMax = 1.0f;
    sustain.mReadOnly._Set([](const TagArgs& args) { return !ElementADSR(args).mEnabled; });
    sustain.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    sustain.mGetter = [](const PropAccessorArgs& args) { return ElementADSR(args).mSustain; };
    sustain.mSetter = [](const PropAccessorArgs& args) { ElementADSR(args).mSustain = NewValue<float>(args); };

    const float releaseType = 0.0f;
    FloatMetadata& release =
        TypeSpecificMetadata(members.RegisterProp("release", -1, kPropertyFloat, 0), releaseType);
    static_cast<void>(Symbol("prop"));
    release.mSlider = true;
    release.mHelp = String("The release time of this ADSR, in seconds.");
    release.mMin = 0.0f;
    release.mMax = 10.0f;
    release.mSliderLogResolution = LogInterpolator::CalculateBiasForMid(0.0f, 1.0f, 10.0f);
    release.mUnits = Symbol("seconds");
    release.mReadOnly._Set([](const TagArgs& args) { return !ElementADSR(args).mEnabled; });
    release.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    release.mGetter = [](const PropAccessorArgs& args) { return ElementADSR(args).mRelease; };
    release.mSetter = [](const PropAccessorArgs& args) { ElementADSR(args).mRelease = NewValue<float>(args); };
}

// Reconstructed from eboot.elf at 0x6A520. The current preset's two LFOs,
// edited through the same placeholder array as the ADSRs. A disabled LFO's
// settings are read-only, as is the initial phase without retriggering and
// the default tempo without beat sync.
void RegisterLFOProps(PropRegistry& registry) {
    PropInfo& array = registry.RegisterPropArray(
        "lfos", offsetof(FusionPatchCom, mPairSlots), sizeof(PropArray<int>));
    ArrayMetadata& arrayMetadata = static_cast<ArrayMetadata&>(*array.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    arrayMetadata.mFixedSize = true;
    PropRegistry::RegisterPropArrayItem(array, kPropertyStruct, sizeof(int));
    PropRegistry& members =
        static_cast<GroupMetadata&>(*arrayMetadata.mItemInfo.mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct_item"));

    const bool enabledType = false;
    BoolMetadata& enabled =
        TypeSpecificMetadata(members.RegisterProp("enabled", -1, kPropertyBool, 0), enabledType);
    static_cast<void>(Symbol("prop"));
    enabled.mDisplayName._Set([](const TagArgs& args) { return ElementName("LFO", ElementIndex(args)); });
    enabled.mHelp = String("Is this LFO enabled and affecting audio output?");
    enabled.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    enabled.mGetter = [](const PropAccessorArgs& args) { return ElementLFO(args).mEnabled; };
    enabled.mSetter = [](const PropAccessorArgs& args) { ElementLFO(args).mEnabled = NewValue<bool>(args); };

    const unsigned char targetType = 0;
    IntMetadata<unsigned char>& target =
        TypeSpecificMetadata(members.RegisterProp("target", -1, kPropertyUInt8, 0), targetType);
    static_cast<void>(Symbol("prop"));
    target.mHelp = String("The property being affected by this LFO.");
    target.mAllowedValues = LFO::Settings::GetAllowedTargetValues();
    target.mReadOnly._Set([](const TagArgs& args) { return !ElementLFO(args).mEnabled; });
    target.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    target.mGetter = [](const PropAccessorArgs& args) -> unsigned char { return ElementLFO(args).mTarget; };
    target.mSetter = [](const PropAccessorArgs& args) {
        ElementLFO(args).mTarget = static_cast<LFO::Settings::Target>(NewValue<unsigned char>(args));
    };

    const float depthType = 0.0f;
    FloatMetadata& depth = TypeSpecificMetadata(members.RegisterProp("depth", -1, kPropertyFloat, 0), depthType);
    static_cast<void>(Symbol("prop"));
    depth.mSlider = true;
    depth.mHelp = String("The depth of the effect produced by this LFO.");
    depth.mMin = 0.0f;
    depth.mMax = 1.0f;
    depth.mReadOnly._Set([](const TagArgs& args) { return !ElementLFO(args).mEnabled; });
    depth.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    depth.mGetter = [](const PropAccessorArgs& args) { return ElementLFO(args).mDepth; };
    depth.mSetter = [](const PropAccessorArgs& args) { ElementLFO(args).mDepth = NewValue<float>(args); };

    const unsigned char shapeType = 0;
    IntMetadata<unsigned char>& shape =
        TypeSpecificMetadata(members.RegisterProp("shape", -1, kPropertyUInt8, 0), shapeType);
    static_cast<void>(Symbol("prop"));
    shape.mHelp = String("The shape of the effect produced by this LFO.");
    shape.mAllowedValues = LFO::Settings::GetAllowedShapeValues();
    shape.mReadOnly._Set([](const TagArgs& args) { return !ElementLFO(args).mEnabled; });
    shape.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    shape.mGetter = [](const PropAccessorArgs& args) -> unsigned char { return ElementLFO(args).mShape; };
    shape.mSetter = [](const PropAccessorArgs& args) {
        ElementLFO(args).mShape = static_cast<LFO::Settings::Shape>(NewValue<unsigned char>(args));
    };

    const float frequencyType = 0.0f;
    FloatMetadata& frequency =
        TypeSpecificMetadata(members.RegisterProp("frequency", -1, kPropertyFloat, 0), frequencyType);
    static_cast<void>(Symbol("prop"));
    frequency.mSlider = true;
    frequency.mHelp = String("The frequency of the effect produced by this LFO.");
    frequency.mMin = 0.1f;
    frequency.mMax = 30.0f;
    frequency.mSliderLogResolution = LogInterpolator::CalculateBiasForMid(0.1f, 4.0f, 30.0f);
    frequency.mUnits = Symbol("Hz");
    frequency.mReadOnly._Set([](const TagArgs& args) { return !ElementLFO(args).mEnabled; });
    frequency.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    frequency.mGetter = [](const PropAccessorArgs& args) { return ElementLFO(args).mFrequency; };
    frequency.mSetter = [](const PropAccessorArgs& args) {
        ElementLFO(args).mFrequency = NewValue<float>(args);
    };

    const bool retriggerType = false;
    BoolMetadata& retrigger =
        TypeSpecificMetadata(members.RegisterProp("retrigger", -1, kPropertyBool, 0), retriggerType);
    static_cast<void>(Symbol("prop"));
    retrigger.mHelp = String("Should the LFO retrigger the initial phase when it starts?");
    retrigger.mReadOnly._Set([](const TagArgs& args) { return !ElementLFO(args).mEnabled; });
    retrigger.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    retrigger.mGetter = [](const PropAccessorArgs& args) { return ElementLFO(args).mRetrigger; };
    retrigger.mSetter = [](const PropAccessorArgs& args) {
        ElementLFO(args).mRetrigger = NewValue<bool>(args);
    };

    const float phaseType = 0.0f;
    FloatMetadata& phase =
        TypeSpecificMetadata(members.RegisterProp("initial_phase", -1, kPropertyFloat, 0), phaseType);
    static_cast<void>(Symbol("prop"));
    phase.mSlider = true;
    phase.mHelp = String("Initial phase if retrigger option is selected.");
    phase.mMin = -180.0f;
    phase.mMax = 180.0f;
    phase.mDecimalPlaces = 0;
    phase.mReadOnly._Set([](const TagArgs& args) {
        const LFO::Settings& lfo = ElementLFO(args);
        return !lfo.mEnabled || !lfo.mRetrigger;
    });
    phase.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    phase.mGetter = [](const PropAccessorArgs& args) { return ElementLFO(args).mInitialPhase; };
    phase.mSetter = [](const PropAccessorArgs& args) {
        ElementLFO(args).mInitialPhase = NewValue<float>(args);
    };

    const bool beatSyncType = false;
    BoolMetadata& beatSync =
        TypeSpecificMetadata(members.RegisterProp("beat_sync", -1, kPropertyBool, 0), beatSyncType);
    static_cast<void>(Symbol("prop"));
    beatSync.mHelp = String("Should the LFO frequency adjust to tempo?");
    beatSync.mReadOnly._Set([](const TagArgs& args) { return !ElementLFO(args).mEnabled; });
    beatSync.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    beatSync.mGetter = [](const PropAccessorArgs& args) { return ElementLFO(args).mBeatSync; };
    beatSync.mSetter = [](const PropAccessorArgs& args) { ElementLFO(args).mBeatSync = NewValue<bool>(args); };

    const float tempoType = 0.0f;
    FloatMetadata& tempo =
        TypeSpecificMetadata(members.RegisterProp("default_tempo", -1, kPropertyFloat, 0), tempoType);
    static_cast<void>(Symbol("prop"));
    tempo.mSlider = true;
    tempo.mHelp = String("The frequency of the effect produced by this LFO.");
    tempo.mMin = 20.0f;
    tempo.mMax = 240.0f;
    tempo.mUnits = Symbol("BPM");
    tempo.mReadOnly._Set([](const TagArgs& args) {
        const LFO::Settings& lfo = ElementLFO(args);
        return !lfo.mEnabled || !lfo.mBeatSync;
    });
    tempo.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    tempo.mGetter = [](const PropAccessorArgs& args) { return ElementLFO(args).mDefaultTempo; };
    tempo.mSetter = [](const PropAccessorArgs& args) {
        ElementLFO(args).mDefaultTempo = NewValue<float>(args);
    };
}

// A filter of the current preset. The selectors name the preset's filter,
// the distortion's three and the amp simulation's three. Names not in the
// reference map.
using FilterSelector = BiquadFilter::Settings& (*)(PresetSettings& preset);
BiquadFilter::Settings& PresetFilter(PresetSettings& preset) {
    return preset.mFilter;
}
BiquadFilter::Settings& DistortionFilter1(PresetSettings& preset) {
    return preset.mDistortionFilters[0];
}
BiquadFilter::Settings& DistortionFilter2(PresetSettings& preset) {
    return preset.mDistortionFilters[1];
}
BiquadFilter::Settings& DistortionFilter3(PresetSettings& preset) {
    return preset.mDistortionFilters[2];
}
BiquadFilter::Settings& AmpFilter1(PresetSettings& preset) {
    return preset.mAmpFilters[0];
}
BiquadFilter::Settings& AmpFilter2(PresetSettings& preset) {
    return preset.mAmpFilters[1];
}
BiquadFilter::Settings& AmpFilter3(PresetSettings& preset) {
    return preset.mAmpFilters[2];
}

// Registers the members of a filter struct: "enabled", "type",
// "frequency", "q" and "gain", read-only while the filter is disabled; the
// gain applies only to the peaking and shelf types. The binary repeats
// these registrations inline for each of the seven filters, each with its
// own accessors, as the instantiations do. `displayName` names "enabled"
// when given. Name not in the reference map.
template <FilterSelector Select>
void RegisterFilterMembers(PropRegistry& registry, const char* displayName) {
    const bool enabledType = false;
    BoolMetadata& enabled =
        TypeSpecificMetadata(registry.RegisterProp("enabled", -1, kPropertyBool, 0), enabledType);
    static_cast<void>(Symbol("prop"));
    if (displayName != nullptr) {
        enabled.mDisplayName = String(displayName);
    }
    enabled.mHelp = String("Is this filter enabled and affecting audio output?");
    enabled.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    enabled.mGetter = [](const PropAccessorArgs& args) { return Select(CurrentPreset(args)).mEnabled; };
    enabled.mSetter = [](const PropAccessorArgs& args) {
        Select(CurrentPreset(args)).mEnabled = NewValue<bool>(args);
    };

    const signed char typeType = 0;
    IntMetadata<signed char>& type =
        TypeSpecificMetadata(registry.RegisterProp("type", -1, kPropertyInt8, 0), typeType);
    static_cast<void>(Symbol("prop"));
    type.mHelp = String("The type of filtering to be performed on audio output.");
    type.mAllowedValues = BiquadFilter::Settings::GetAllowedTypeValues();
    type.mReadOnly._Set([](const TagArgs& args) { return !Select(CurrentPreset(args)).mEnabled; });
    type.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    type.mGetter = [](const PropAccessorArgs& args) -> signed char {
        return static_cast<signed char>(Select(CurrentPreset(args)).mType);
    };
    type.mSetter = [](const PropAccessorArgs& args) {
        Select(CurrentPreset(args)).mType = static_cast<BiquadFilter::Settings::Type>(NewValue<signed char>(args));
    };

    const float frequencyType = 0.0f;
    FloatMetadata& frequency =
        TypeSpecificMetadata(registry.RegisterProp("frequency", -1, kPropertyFloat, 0), frequencyType);
    static_cast<void>(Symbol("prop"));
    frequency.mSlider = true;
    frequency.mHelp = String("The frequency of the filter effect.");
    frequency.mMin = 20.0f;
    frequency.mMax = 20000.0f;
    frequency.mSliderLogResolution = LogInterpolator::CalculateBiasForMid(20.0f, 1000.0f, 20000.0f);
    frequency.mUnits = Symbol("Hz");
    frequency.mReadOnly._Set([](const TagArgs& args) { return !Select(CurrentPreset(args)).mEnabled; });
    frequency.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    frequency.mGetter = [](const PropAccessorArgs& args) { return Select(CurrentPreset(args)).mFrequency; };
    frequency.mSetter = [](const PropAccessorArgs& args) {
        Select(CurrentPreset(args)).mFrequency = NewValue<float>(args);
    };

    const float qType = 0.0f;
    FloatMetadata& q = TypeSpecificMetadata(registry.RegisterProp("q", -1, kPropertyFloat, 0), qType);
    static_cast<void>(Symbol("prop"));
    q.mSlider = true;
    q.mHelp = String("The Q value of the filter effect.");
    q.mMin = 0.1f;
    q.mMax = 10.0f;
    q.mSliderLogResolution = LogInterpolator::CalculateBiasForMid(0.1f, 0.71f, 10.0f);
    // The shelves take a slope and the peaking filter a bandwidth.
    q.mUnits._Set([](const TagArgs& args) {
        const int filterType = static_cast<signed char>(Select(CurrentPreset(args)).mType);
        if (filterType == BiquadFilter::Settings::kTypeLowShelf ||
            filterType == BiquadFilter::Settings::kTypeHighShelf) {
            return Symbol(" db/octave");
        }
        if (filterType == BiquadFilter::Settings::kTypePeaking) {
            return Symbol(" octaves");
        }
        return Symbol(" Q");
    });
    q.mReadOnly._Set([](const TagArgs& args) { return !Select(CurrentPreset(args)).mEnabled; });
    q.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    q.mGetter = [](const PropAccessorArgs& args) { return Select(CurrentPreset(args)).mQ; };
    q.mSetter = [](const PropAccessorArgs& args) { Select(CurrentPreset(args)).mQ = NewValue<float>(args); };

    const float gainType = 0.0f;
    FloatMetadata& gain = TypeSpecificMetadata(registry.RegisterProp("gain", -1, kPropertyFloat, 0), gainType);
    static_cast<void>(Symbol("prop"));
    gain.mSlider = true;
    gain.mHelp = String("Gain or Boost dB.");
    gain.mMin = -96.0f;
    gain.mMax = 12.0f;
    gain.mUnits = Symbol("dB");
    gain.mDecimalPlaces = 2;
    gain.mReadOnly._Set([](const TagArgs& args) {
        const BiquadFilter::Settings& filter = Select(CurrentPreset(args));
        if (!filter.mEnabled) {
            return true;
        }
        return filter.mType < BiquadFilter::Settings::kTypePeaking ||
               filter.mType > BiquadFilter::Settings::kTypeHighShelf;
    });
    gain.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    gain.mGetter = [](const PropAccessorArgs& args) { return Select(CurrentPreset(args)).mGain; };
    gain.mSetter = [](const PropAccessorArgs& args) {
        Select(CurrentPreset(args)).mGain = NewValue<float>(args);
    };
}

// The member registry of a struct property without storage, registered at
// offset 0 with no size: its members are accessors. Inlined into the
// builders. Name not in the reference map.
PropRegistry& RegisterAccessorStruct(PropRegistry& registry, const char* name) {
    PropRegistry& members = static_cast<GroupMetadata&>(
                                *registry.RegisterProp(name, 0, kPropertyStruct, 0).mMetadata.Get())
                                .mRegistry;
    static_cast<void>(Symbol("struct"));
    return members;
}

// Reconstructed from eboot.elf at 0x6B4E0. The current preset's filter.
void RegisterFilterProps(PropRegistry& registry) {
    RegisterFilterMembers<PresetFilter>(RegisterAccessorStruct(registry, "filter"), "filter");
}

// Reconstructed from eboot.elf at 0x6BE60. The current preset's
// portamento; its mode and time are read-only while it is disabled.
void RegisterPortamentoProps(PropRegistry& registry) {
    PropRegistry& members = RegisterAccessorStruct(registry, "portamento");

    const bool enabledType = false;
    BoolMetadata& enabled =
        TypeSpecificMetadata(members.RegisterProp("enabled", -1, kPropertyBool, 0), enabledType);
    static_cast<void>(Symbol("prop"));
    enabled.mDisplayName = String("portamento");
    enabled.mHelp = String("Is this patch using portamento to slur note pitches?");
    enabled.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    enabled.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mPortamento.mEnabled; };
    enabled.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mPortamento.mEnabled = NewValue<bool>(args);
    };

    const int modeType = 0;
    IntMetadata<int>& mode = TypeSpecificMetadata(members.RegisterProp("mode", -1, kPropertyInt, 0), modeType);
    static_cast<void>(Symbol("prop"));
    mode.mHelp = String(
        "The portamento mode.Legato mode means that when no notes are playing, the portamento pitch is "
        "forgotten.After a period of silence, the next note will attack with no pitch bend.Persistent "
        "mode means that the portamento pitch will be remembered,and the next note will bend from that "
        "pitch regardless of a period of silence.");
    mode.mAllowedValues = FusionPatchCom::PortamentoSettings::GetAllowedModeValues();
    mode.mReadOnly._Set([](const TagArgs& args) { return !CurrentPreset(args).mPortamento.mEnabled; });
    mode.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    mode.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mPortamento.mMode; };
    mode.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mPortamento.mMode = NewValue<int>(args);
    };

    const float timeType = 0.0f;
    FloatMetadata& time = TypeSpecificMetadata(members.RegisterProp("time", -1, kPropertyFloat, 0), timeType);
    static_cast<void>(Symbol("prop"));
    time.mDisplayName = String("time");
    time.mSlider = true;
    time.mHelp = String("The amount of time over which portamento occurs.");
    time.mMin = 0.0f;
    time.mMax = 10.0f;
    time.mSliderLogResolution = LogInterpolator::CalculateBiasForMid(0.0f, 0.25f, 10.0f);
    time.mUnits = Symbol("seconds");
    time.mDefault = 0.1f;
    time.mReadOnly._Set([](const TagArgs& args) { return !CurrentPreset(args).mPortamento.mEnabled; });
    time.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    time.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mPortamento.mTime; };
    time.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mPortamento.mTime = NewValue<float>(args);
    };
}

// Reconstructed from eboot.elf at 0x6C3D0. The current preset's four
// modulators: two randomizers, then two velocity modulators. The range's
// prefix and units follow the target: milliseconds of start point or cents
// of pitch.
void RegisterModulatorProps(PropRegistry& registry) {
    PropInfo& array = registry.RegisterPropArray(
        "modulators", offsetof(FusionPatchCom, mModulatorSlots), sizeof(PropArray<int>));
    ArrayMetadata& arrayMetadata = static_cast<ArrayMetadata&>(*array.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    arrayMetadata.mFixedSize = true;
    PropRegistry::RegisterPropArrayItem(array, kPropertyStruct, sizeof(int));
    PropRegistry& members =
        static_cast<GroupMetadata&>(*arrayMetadata.mItemInfo.mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct_item"));

    const unsigned char targetType = 0;
    IntMetadata<unsigned char>& target =
        TypeSpecificMetadata(members.RegisterProp("target", -1, kPropertyUInt8, 0), targetType);
    static_cast<void>(Symbol("prop"));
    target.mDisplayName._Set([](const TagArgs& args) {
        const String kind(ElementIndex(args) < 2 ? "randomizer" : "velocity mod");
        return ElementName(kind.c_str(), ElementIndex(args));
    });
    target.mHelp = String("The property being affected by this modulator.");
    target.mAllowedValues = ModulatorTarget::GetAllowedTargetValues();
    target.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    target.mGetter = [](const PropAccessorArgs& args) -> unsigned char { return ElementModulator(args).mTarget; };
    target.mSetter = [](const PropAccessorArgs& args) {
        ElementModulator(args).mTarget = static_cast<ModulatorTarget::Target>(NewValue<unsigned char>(args));
    };

    const float rangeType = 0.0f;
    FloatMetadata& range = TypeSpecificMetadata(members.RegisterProp("range", -1, kPropertyFloat, 0), rangeType);
    static_cast<void>(Symbol("prop"));
    range.mMin = 0.0f;
    range.mMax = 1000000.0f;
    range.mPrefix._Set([](const TagArgs& args) {
        switch (ElementModulator(args).mTarget) {
        case ModulatorTarget::kTargetStartPoint:
            return Symbol(ModulatorTarget::GetMathSymbolForDepthMode(ModulatorTarget::kDepthModeSubtractive));
        case ModulatorTarget::kTargetPitch:
            return Symbol(ModulatorTarget::GetMathSymbolForDepthMode(ModulatorTarget::kDepthModeCentered));
        default:
            return Symbol();
        }
    });
    range.mUnits._Set([](const TagArgs& args) {
        switch (ElementModulator(args).mTarget) {
        case ModulatorTarget::kTargetStartPoint:
            return Symbol("ms");
        case ModulatorTarget::kTargetPitch:
            return Symbol("cents");
        default:
            return Symbol();
        }
    });
    range.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    range.mGetter = [](const PropAccessorArgs& args) { return ElementModulator(args).mRange; };
    range.mSetter = [](const PropAccessorArgs& args) { ElementModulator(args).mRange = NewValue<float>(args); };

    const float depthType = 0.0f;
    FloatMetadata& depth = TypeSpecificMetadata(members.RegisterProp("depth", -1, kPropertyFloat, 0), depthType);
    static_cast<void>(Symbol("prop"));
    depth.mSlider = true;
    depth.mMin = 0.0f;
    depth.mMax = 1.0f;
    depth.mHelp = String("The depth of the effect produced by this velocity modulator.");
    depth.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    depth.mGetter = [](const PropAccessorArgs& args) { return ElementModulator(args).mDepth; };
    depth.mSetter = [](const PropAccessorArgs& args) { ElementModulator(args).mDepth = NewValue<float>(args); };
}

// Reconstructed from eboot.elf at 0x6C950. The keyzones, stored in the
// component. Inserting an element adds a keyzone and the remove hook
// deletes one, so the samples and the resource's entity follow. Changing
// a zone's sample, note range or priority sends "keyzone_property_changed";
// the time-stretch settings are read-only unless "maintain_time" is set,
// and the random weight unless the preset selects keyzones at random. The
// track map shows only for samples of more than two channels.
void RegisterKeyzoneProps(PropRegistry& registry) {
    PropInfo& array =
        registry.RegisterPropArray("keyzones", offsetof(FusionPatchCom, mKeyzones), sizeof(PropArray<KeyzoneSettings>));
    ArrayMetadata& arrayMetadata = static_cast<ArrayMetadata&>(*array.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    // The remove hook's path ends at the removed element. The field is
    // modelled as void*.
    arrayMetadata.mHook2 = reinterpret_cast<void*>(
        static_cast<void (*)(const PropAccessorArgs&)>([](const PropAccessorArgs& args) {
            static_cast<FusionPatchCom*>(args.mComponent)
                ->DeleteKeyZone(PathIndex(*static_cast<const PropPath*>(args.mPath), 1));
        }));
    // The insertion passes the new zone's sample path in the field after
    // the storage (weak).
    arrayMetadata.mInsert = [](const PropAccessorArgs& args) {
        static_cast<FusionPatchCom*>(args.mComponent)->AddKeyZone(static_cast<const char*>(args.mOldValue));
    };
    PropRegistry::RegisterPropArrayItem(array, kPropertyStruct, sizeof(KeyzoneSettings));
    GroupMetadata& item = static_cast<GroupMetadata&>(*arrayMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const KeyzoneSettings prototype;
    // The element's path ends at its index.
    item.mDisplayName._Set([](const TagArgs& args) {
        const FusionPatchCom& patch = static_cast<const FusionPatchCom&>(args.mComponent);
        FormatString format("Sample: %s");
        format << patch.mKeyzones[PathIndex(args.mPath, 1)].mSamplePath.mPath;
        return String(format.Str());
    });
    PropRegistry& members = item.mRegistry;

    ResourcePathMetadata& sample = TypeSpecificMetadata(
        members.RegisterProp(
            "sample", offsetof(KeyzoneSettings, mSamplePath), kPropertyResourcePath, sizeof(ResourcePath)),
        prototype.mSamplePath);
    sample.mDefault = prototype.mSamplePath;
    static_cast<void>(Symbol("prop"));
    sample.mDisplayName = String("sample");
    sample.mHelp = String("The sample file for this keyzone.");
    sample.mCustomLabel = Symbol("Sample files");
    sample.mCustomExtensions = {Symbol("wav"), Symbol("mogg")};
    sample.mIsRelativeToEntity = true;
    sample.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<FusionPatchCom*>(args.mComponent)->_KeyZonePropertyChanged(ElementIndex(args));
    };

    BoolMetadata& noteOff = TypeSpecificMetadata(
        members.RegisterProp(
            "trigger_on_note_off", offsetof(KeyzoneSettings, mTriggerOnNoteOff), kPropertyBool, sizeof(bool)),
        prototype.mTriggerOnNoteOff);
    noteOff.mDefault = prototype.mTriggerOnNoteOff;
    static_cast<void>(Symbol("prop"));
    noteOff.mHelp = String("Triggered on note off instead of note on.");
    // Read-only without a sample, and for samples whose audio data reports
    // a nonzero slot 12.
    noteOff.mReadOnly._Set([](const TagArgs& args) {
        AudioSampleResource* const sampleResource = ElementKeyzone(args).mSample;
        if (sampleResource == nullptr) {
            return true;
        }
        return sampleResource->GetAudioData()->GetSampleFormat() != 0;
    });
    noteOff.mSetter = [](const PropAccessorArgs& args) {
        ElementKeyzone(args).mTriggerOnNoteOff = NewValue<bool>(args);
    };
    noteOff.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    IntMetadata<unsigned char>& priority = TypeSpecificMetadata(
        members.RegisterProp("priority", offsetof(KeyzoneSettings, mPriority), kPropertyUInt8, 1),
        prototype.mPriority);
    priority.mDefault = prototype.mPriority;
    static_cast<void>(Symbol("prop"));
    priority.mHelp = String(
        "Voice stealing priority. 0 = never steal, 1 = highest priority, 100 = lowest priority.");
    priority.mMin = 0;
    priority.mMax = 100;
    priority.mSetter = [](const PropAccessorArgs& args) {
        ElementKeyzone(args).mPriority = NewValue<unsigned char>(args);
    };
    priority.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<FusionPatchCom*>(args.mComponent)->_KeyZonePropertyChanged(ElementIndex(args));
    };

    // The note and velocity limits are registered as signed bytes.
    const signed char minNoteDefault = static_cast<signed char>(prototype.mMinNote);
    IntMetadata<signed char>& minNote = TypeSpecificMetadata(
        members.RegisterProp("min_note", offsetof(KeyzoneSettings, mMinNote), kPropertyInt8, 1), minNoteDefault);
    minNote.mDefault = minNoteDefault;
    static_cast<void>(Symbol("prop"));
    minNote.mHelp = String("The minimum MIDI note at which this keyzone will sound.");
    minNote.mMin = 0;
    minNote.mMax._Set([](const TagArgs& args) { return static_cast<signed char>(ElementKeyzone(args).mMaxNote); });
    minNote.mUnits._Set([](const TagArgs& args) { return Symbol(NoteName(ElementKeyzone(args).mMinNote)); });
    minNote.mSetter = [](const PropAccessorArgs& args) {
        ElementKeyzone(args).mMinNote = NewValue<signed char>(args);
    };
    minNote.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<FusionPatchCom*>(args.mComponent)->_KeyZonePropertyChanged(ElementIndex(args));
    };

    const signed char maxNoteDefault = static_cast<signed char>(prototype.mMaxNote);
    IntMetadata<signed char>& maxNote = TypeSpecificMetadata(
        members.RegisterProp("max_note", offsetof(KeyzoneSettings, mMaxNote), kPropertyInt8, 1), maxNoteDefault);
    maxNote.mDefault = maxNoteDefault;
    static_cast<void>(Symbol("prop"));
    maxNote.mHelp = String("The maximum MIDI note at which this keyzone will sound.");
    maxNote.mMax = 127;
    maxNote.mMin._Set([](const TagArgs& args) { return static_cast<signed char>(ElementKeyzone(args).mMinNote); });
    maxNote.mUnits._Set([](const TagArgs& args) { return Symbol(NoteName(ElementKeyzone(args).mMaxNote)); });
    maxNote.mSetter = [](const PropAccessorArgs& args) {
        ElementKeyzone(args).mMaxNote = NewValue<signed char>(args);
    };
    maxNote.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<FusionPatchCom*>(args.mComponent)->_KeyZonePropertyChanged(ElementIndex(args));
    };

    const signed char rootNoteDefault = static_cast<signed char>(prototype.mRootNote);
    IntMetadata<signed char>& rootNote = TypeSpecificMetadata(
        members.RegisterProp("root_note", offsetof(KeyzoneSettings, mRootNote), kPropertyInt8, 1), rootNoteDefault);
    rootNote.mDefault = rootNoteDefault;
    static_cast<void>(Symbol("prop"));
    rootNote.mHelp =
        String("The root MIDI note of this keyzone, at which no pitch adjustment is performed on the sample.");
    rootNote.mMin = 0;
    rootNote.mMax = 127;
    rootNote.mUnits._Set([](const TagArgs& args) { return Symbol(NoteName(ElementKeyzone(args).mRootNote)); });
    rootNote.mSetter = [](const PropAccessorArgs& args) {
        ElementKeyzone(args).mRootNote = NewValue<signed char>(args);
    };
    rootNote.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    const signed char minVelocityDefault = static_cast<signed char>(prototype.mMinVelocity);
    IntMetadata<signed char>& minVelocity = TypeSpecificMetadata(
        members.RegisterProp("min_velocity", offsetof(KeyzoneSettings, mMinVelocity), kPropertyInt8, 1),
        minVelocityDefault);
    minVelocity.mDefault = minVelocityDefault;
    static_cast<void>(Symbol("prop"));
    minVelocity.mHelp = String("The minimum velocity at which this keyzone will sound.");
    minVelocity.mMin = 0;
    minVelocity.mMax = 127;
    minVelocity.mSetter = [](const PropAccessorArgs& args) {
        ElementKeyzone(args).mMinVelocity = NewValue<signed char>(args);
    };
    minVelocity.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    const signed char maxVelocityDefault = static_cast<signed char>(prototype.mMaxVelocity);
    IntMetadata<signed char>& maxVelocity = TypeSpecificMetadata(
        members.RegisterProp("max_velocity", offsetof(KeyzoneSettings, mMaxVelocity), kPropertyInt8, 1),
        maxVelocityDefault);
    maxVelocity.mDefault = maxVelocityDefault;
    static_cast<void>(Symbol("prop"));
    maxVelocity.mHelp = String("The max velocity at which this keyzone will sound.");
    maxVelocity.mMin = 0;
    maxVelocity.mMax = 127;
    maxVelocity.mSetter = [](const PropAccessorArgs& args) {
        ElementKeyzone(args).mMaxVelocity = NewValue<signed char>(args);
    };
    maxVelocity.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    FloatMetadata& volume = TypeSpecificMetadata(
        members.RegisterProp("volume", offsetof(KeyzoneSettings, mVolume), kPropertyFloat, sizeof(float)),
        prototype.mVolume);
    volume.mDefault = prototype.mVolume;
    static_cast<void>(Symbol("prop"));
    volume.mSlider = true;
    // The map's KeyzoneSettings::SetVolumeDb.
    volume.mSetter = [](const PropAccessorArgs& args) {
        KeyzoneSettings& keyzone = ElementKeyzone(args);
        const float volumeDb = NewValue<float>(args);
        keyzone.mVolume = volumeDb;
        keyzone.mVolumeGain = powf(10.0f, volumeDb * 0.05f);
    };
    volume.mHelp = String("The base volume level for this particular keyzone, in dB.");
    volume.mMin = -96.0f;
    volume.mMax = 0.0f;
    volume.mUnits = Symbol("dB");
    volume.mDecimalPlaces = 2;
    volume.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    FloatMetadata& pan = TypeSpecificMetadata(
        members.RegisterProp("pan", offsetof(KeyzoneSettings, mPan), kPropertyFloat, sizeof(float)), prototype.mPan);
    pan.mDefault = prototype.mPan;
    static_cast<void>(Symbol("prop"));
    pan.mSlider = true;
    pan.mHelp = String("The pan offset of this particular keyzone.");
    pan.mMin = -1.0f;
    pan.mMax = 1.0f;
    pan.mSetter = [](const PropAccessorArgs& args) { ElementKeyzone(args).mPan = NewValue<float>(args); };
    pan.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    FloatMetadata& tune = TypeSpecificMetadata(
        members.RegisterProp("fine_tune", offsetof(KeyzoneSettings, mFineTune), kPropertyFloat, sizeof(float)),
        prototype.mFineTune);
    tune.mDefault = prototype.mFineTune;
    static_cast<void>(Symbol("prop"));
    tune.mSlider = true;
    // The map's KeyzoneSettings::SetFineTuneCents.
    tune.mSetter = [](const PropAccessorArgs& args) {
        KeyzoneSettings& keyzone = ElementKeyzone(args);
        const float cents = NewValue<float>(args);
        keyzone.mFineTune = cents;
        keyzone.mFineTuneRatio = exp2f(cents * (1.0f / 1200.0f));
    };
    tune.mHelp = String("The pitch offset of this particular keyzone, in cents.");
    tune.mMin = -100.0f;
    tune.mMax = 100.0f;
    tune.mUnits = Symbol("cents");
    tune.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    BoolMetadata& unpitched = TypeSpecificMetadata(
        members.RegisterProp("unpitched", offsetof(KeyzoneSettings, mUnpitched), kPropertyBool, sizeof(bool)),
        prototype.mUnpitched);
    unpitched.mDefault = prototype.mUnpitched;
    static_cast<void>(Symbol("prop"));
    unpitched.mHelp = String("Ignore Midi note, always play at original pitch.");
    unpitched.mSetter = [](const PropAccessorArgs& args) { ElementKeyzone(args).mUnpitched = NewValue<bool>(args); };
    unpitched.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    BoolMetadata& velocityToVolume = TypeSpecificMetadata(
        members.RegisterProp(
            "velocity_to_volume", offsetof(KeyzoneSettings, mVelocityToVolume), kPropertyBool, sizeof(bool)),
        prototype.mVelocityToVolume);
    velocityToVolume.mDefault = prototype.mVelocityToVolume;
    static_cast<void>(Symbol("prop"));
    velocityToVolume.mHelp = String("Does velocity affect keyzone volume?");
    velocityToVolume.mSetter = [](const PropAccessorArgs& args) {
        ElementKeyzone(args).mVelocityToVolume = NewValue<bool>(args);
    };
    velocityToVolume.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    BoolMetadata& maintainTime = TypeSpecificMetadata(
        members.RegisterProp("maintain_time", offsetof(KeyzoneSettings, mMaintainTime), kPropertyBool, sizeof(bool)),
        prototype.mMaintainTime);
    maintainTime.mDefault = prototype.mMaintainTime;
    static_cast<void>(Symbol("prop"));
    maintainTime.mHelp = String("Time stretch sample to keep length the same while changing pitch.");
    maintainTime.mSetter = [](const PropAccessorArgs& args) {
        ElementKeyzone(args).mMaintainTime = NewValue<bool>(args);
    };
    maintainTime.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    // The formant mode as a flag: 1 keeps the formant, 2 does not.
    const bool formantType = false;
    BoolMetadata& formant =
        TypeSpecificMetadata(members.RegisterProp("maintain_formant", -1, kPropertyBool, 0), formantType);
    static_cast<void>(Symbol("prop"));
    formant.mHelp = String(
        "When changing the pitch of the sample, retain the formant content (to keep voices from "
        "chipmunking/darth vadering)");
    formant.mReadOnly._Set([](const TagArgs& args) { return !ElementKeyzone(args).mMaintainTime; });
    formant.mDefault = false;
    formant.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    formant.mGetter = [](const PropAccessorArgs& args) { return ElementKeyzone(args).mFormantMode == 1; };
    formant.mSetter = [](const PropAccessorArgs& args) {
        ElementKeyzone(args).mFormantMode = NewValue<bool>(args) ? 1 : 2;
    };

    const signed char algorithmType = 0;
    IntMetadata<signed char>& algorithm =
        TypeSpecificMetadata(members.RegisterProp("algorithm", -1, kPropertyInt8, 0), algorithmType);
    static_cast<void>(Symbol("prop"));
    algorithm.mHelp = String("Options will depend on what algorithms are available for the current game.");
    algorithm.mReadOnly._Set([](const TagArgs& args) { return !ElementKeyzone(args).mMaintainTime; });
    algorithm.mDefault = 1;
    // This build offers only the default algorithm.
    algorithm.mAllowedValues._Set([](const TagArgs& args) {
        static_cast<void>(args);
        eastl::vector<AllowedValue<signed char>> values;
        values.push_back(AllowedValue<signed char>{1, String("Default"), String("default")});
        return values;
    });
    algorithm.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    algorithm.mGetter = [](const PropAccessorArgs& args) {
        return static_cast<signed char>(ElementKeyzone(args).mTimeStretchAlgorithm);
    };
    algorithm.mSetter = [](const PropAccessorArgs& args) {
        ElementKeyzone(args).mTimeStretchAlgorithm = NewValue<signed char>(args);
    };

    BoolMetadata& tempoSync = TypeSpecificMetadata(
        members.RegisterProp("tempo_sync", offsetof(KeyzoneSettings, mTempoSync), kPropertyBool, sizeof(bool)),
        prototype.mTempoSync);
    tempoSync.mDefault = prototype.mTempoSync;
    static_cast<void>(Symbol("prop"));
    tempoSync.mHelp = String("Time stretch sample based on current tempo.");
    tempoSync.mSetter = [](const PropAccessorArgs& args) { ElementKeyzone(args).mTempoSync = NewValue<bool>(args); };
    tempoSync.mReadOnly._Set([](const TagArgs& args) { return !ElementKeyzone(args).mMaintainTime; });
    tempoSync.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    IntMetadata<short>& sampledTempo = TypeSpecificMetadata(
        members.RegisterProp("sampled_tempo", offsetof(KeyzoneSettings, mSampledTempo), kPropertyInt16, sizeof(short)),
        prototype.mSampledTempo);
    sampledTempo.mDefault = prototype.mSampledTempo;
    static_cast<void>(Symbol("prop"));
    sampledTempo.mHelp = String("Tempo of the sample as recorded.");
    sampledTempo.mMin = 20;
    sampledTempo.mMax = 300;
    sampledTempo.mSetter = [](const PropAccessorArgs& args) {
        ElementKeyzone(args).mSampledTempo = NewValue<short>(args);
    };
    sampledTempo.mReadOnly._Set([](const TagArgs& args) {
        const KeyzoneSettings& keyzone = ElementKeyzone(args);
        return !keyzone.mMaintainTime || !keyzone.mTempoSync;
    });
    sampledTempo.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    FloatMetadata& adjustment = TypeSpecificMetadata(
        members.RegisterProp(
            "fine_tune_adjustment", offsetof(KeyzoneSettings, mFineTuneRatio), kPropertyFloat, sizeof(float)),
        prototype.mFineTuneRatio);
    adjustment.mDefault = prototype.mFineTuneRatio;
    static_cast<void>(Symbol("prop"));
    adjustment.mReadOnly = true;

    FloatMetadata& weight = TypeSpecificMetadata(
        members.RegisterProp("random_weight", offsetof(KeyzoneSettings, mRandomWeight), kPropertyFloat, sizeof(float)),
        prototype.mRandomWeight);
    weight.mDefault = prototype.mRandomWeight;
    static_cast<void>(Symbol("prop"));
    weight.mHelp = String("The relative chance of picking this sample when using \"Random\" keyzone selection.");
    weight.mMin = 0.0f;
    weight.mMax = 100.0f;
    weight.mSetter = [](const PropAccessorArgs& args) { ElementKeyzone(args).mRandomWeight = NewValue<float>(args); };
    weight.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    weight.mReadOnly._Set([](const TagArgs& args) {
        const int mode = CurrentPreset(args).mKeyzoneSelectMode;
        return mode != 1 && mode != 2;
    });

    PropInfo& trackMap = members.RegisterPropArray(
        "track_map", offsetof(KeyzoneSettings, mTrackMap), sizeof(PropArray<TrackChannelInfo>));
    ArrayMetadata& trackMapMetadata = static_cast<ArrayMetadata&>(*trackMap.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    // Shown for samples of more than two channels.
    trackMapMetadata.mIsHidden._Set([](const TagArgs& args) {
        AudioSampleResource* const sampleResource = ElementKeyzone(args).mSample;
        if (sampleResource == nullptr || sampleResource->GetAudioData() == nullptr) {
            return true;
        }
        return sampleResource->GetAudioData()->GetNumChannels() < 3;
    });
    PropRegistry::RegisterPropArrayItem(trackMap, kPropertyStruct, sizeof(TrackChannelInfo));
    GroupMetadata& track = static_cast<GroupMetadata&>(*trackMapMetadata.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const TrackChannelInfo trackPrototype;
    // Each track follows a MIDI controller from 52; the path ends at the
    // track, after the keyzone and "track_map".
    track.mDisplayName._Set([](const TagArgs& args) {
        const FusionPatchCom& patch = static_cast<const FusionPatchCom&>(args.mComponent);
        const unsigned long index = PathIndex(args.mPath, 1);
        FormatString format("%s (midi cc #%d)");
        format << patch.mKeyzones[PathIndex(args.mPath, 3)].mTrackMap[index].mName << index + 52;
        return String(format.Str());
    });
    PropRegistry& trackMembers = track.mRegistry;

    SymbolMetadata& name = TypeSpecificMetadata(
        trackMembers.RegisterProp("name", offsetof(TrackChannelInfo, mName), kPropertySymbol, sizeof(Symbol)),
        trackPrototype.mName);
    name.mDefault = trackPrototype.mName;
    static_cast<void>(Symbol("prop"));

    PropInfo& channels = trackMembers.RegisterPropArray(
        "channels", offsetof(TrackChannelInfo, mChannels), sizeof(PropArray<TrackChannelInfo::Channel>));
    ArrayMetadata& channelsMetadata = static_cast<ArrayMetadata&>(*channels.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(channels, kPropertyStruct, sizeof(TrackChannelInfo::Channel));
    PropRegistry& channelMembers =
        static_cast<GroupMetadata&>(*channelsMetadata.mItemInfo.mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct_item"));
    const TrackChannelInfo::Channel channelPrototype = {-1, 1.0f, 0.0f};

    IntMetadata<int>& streamIndex = TypeSpecificMetadata(
        channelMembers.RegisterProp(
            "stream_index", offsetof(TrackChannelInfo::Channel, mStreamIndex), kPropertyInt, sizeof(int)),
        channelPrototype.mStreamIndex);
    streamIndex.mDefault = channelPrototype.mStreamIndex;
    static_cast<void>(Symbol("prop"));
    streamIndex.mMin = -1;
    streamIndex.mHelp = String("Index of individual channel in multi-channel mogg (0 based!). (-1 = unassigned)");

    // The volume is stored as a gain and shown in dB, clamped to -96 to 0.
    // The path ends at the channel, after the keyzone and the track.
    FloatMetadata& channelVolume = TypeSpecificMetadata(
        channelMembers.RegisterProp(
            "volume", offsetof(TrackChannelInfo::Channel, mVolume), kPropertyFloat, sizeof(float)),
        channelPrototype.mVolume);
    channelVolume.mDefault = channelPrototype.mVolume;
    static_cast<void>(Symbol("prop"));
    channelVolume.mMin = -96.0f;
    channelVolume.mMax = 0.0f;
    channelVolume.mGetter = [](const PropAccessorArgs& args) {
        const PropPath& path = *static_cast<const PropPath*>(args.mPath);
        const FusionPatchCom* const patch = static_cast<FusionPatchCom*>(args.mComponent);
        const float gain =
            patch->mKeyzones[PathIndex(path, 6)].mTrackMap[PathIndex(path, 4)].mChannels[PathIndex(path, 2)].mVolume;
        const float volumeDb = gain == 0.0f ? -96.0f : 20.0f * log10f(gain);
        if (volumeDb > 0.0f) {
            return 0.0f;
        }
        return volumeDb < -96.0f ? -96.0f : volumeDb;
    };
    channelVolume.mSetter = [](const PropAccessorArgs& args) {
        const PropPath& path = *static_cast<const PropPath*>(args.mPath);
        FusionPatchCom* const patch = static_cast<FusionPatchCom*>(args.mComponent);
        float gain = powf(10.0f, NewValue<float>(args) * 0.05f);
        if (gain < 0.0f) {
            gain = 0.0f;
        }
        if (gain > 1.0f) {
            gain = 1.0f;
        }
        patch->mKeyzones[PathIndex(path, 6)].mTrackMap[PathIndex(path, 4)].mChannels[PathIndex(path, 2)].mVolume =
            gain;
    };
    channelVolume.mUnits._Set([](const TagArgs& args) {
        static_cast<void>(args);
        static Symbol sUnits;
        if (sUnits == Symbol()) {
            sUnits = Symbol("dB");
        }
        return sUnits;
    });
    channelVolume.mHelp = String("Default volume in dB (-96.0 to 0.0) of this channel.");

    FloatMetadata& channelPan = TypeSpecificMetadata(
        channelMembers.RegisterProp("pan", offsetof(TrackChannelInfo::Channel, mPan), kPropertyFloat, sizeof(float)),
        channelPrototype.mPan);
    channelPan.mDefault = channelPrototype.mPan;
    static_cast<void>(Symbol("prop"));
    channelPan.mMin = -1.0f;
    channelPan.mMax = 1.0f;
    channelPan.mHelp = String("Default pan of this channel. -1 = left, 1 = right.");
}

// Reconstructed from eboot.elf at 0x6EA40. The current preset's bit
// crusher.
void RegisterBitCrusherProps(PropRegistry& registry) {
    PropRegistry& members = RegisterAccessorStruct(registry, "bitcrusher");

    const bool enabledType = false;
    BoolMetadata& enabled =
        TypeSpecificMetadata(members.RegisterProp("enabled", -1, kPropertyBool, 0), enabledType);
    static_cast<void>(Symbol("prop"));
    enabled.mHelp = String("Is this patch using bit crushing?");
    enabled.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    enabled.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mBitCrusherEnabled; };
    enabled.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mBitCrusherEnabled = NewValue<bool>(args);
    };

    const float wetType = 0.0f;
    FloatMetadata& wet = TypeSpecificMetadata(members.RegisterProp("wet", -1, kPropertyFloat, 0), wetType);
    static_cast<void>(Symbol("prop"));
    wet.mHelp = String("Depth in percent.");
    wet.mSlider = true;
    wet.mMin = 0.0f;
    wet.mMax = 100.0f;
    wet.mUnits = Symbol("%");
    wet.mDefault = 0.0f;
    wet.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    wet.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mBitCrusherWet; };
    wet.mSetter = [](const PropAccessorArgs& args) { CurrentPreset(args).mBitCrusherWet = NewValue<float>(args); };

    const float crushType = 0.0f;
    FloatMetadata& crush =
        TypeSpecificMetadata(members.RegisterProp("crush_amount", -1, kPropertyFloat, 0), crushType);
    static_cast<void>(Symbol("prop"));
    crush.mHelp = String("Amount of crush.");
    crush.mSlider = true;
    crush.mMin = 0.0f;
    crush.mMax = 15.0f;
    crush.mUnits = Symbol("bits");
    crush.mDefault = 0.0f;
    crush.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    crush.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mBitCrusherCrushAmount; };
    crush.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mBitCrusherCrushAmount = NewValue<float>(args);
    };

    const int holdType = 0;
    IntMetadata<int>& hold =
        TypeSpecificMetadata(members.RegisterProp("sample_and_hold_factor", -1, kPropertyInt, 0), holdType);
    static_cast<void>(Symbol("prop"));
    hold.mSlider = true;
    hold.mMin = 0;
    hold.mMax = 16;
    hold.mDefault = 1;
    hold.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    hold.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mBitCrusherSampleAndHoldFactor; };
    hold.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mBitCrusherSampleAndHoldFactor = NewValue<int>(args);
    };
}

// Reconstructed from eboot.elf at 0x6EE10. The current preset's delay.
void RegisterDelayProps(PropRegistry& registry) {
    PropRegistry& members = RegisterAccessorStruct(registry, "delay");

    const bool enabledType = false;
    BoolMetadata& enabled =
        TypeSpecificMetadata(members.RegisterProp("enabled", -1, kPropertyBool, 0), enabledType);
    static_cast<void>(Symbol("prop"));
    enabled.mHelp = String("Is this patch using delay?");
    enabled.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    enabled.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mDelayEnabled; };
    enabled.mSetter = [](const PropAccessorArgs& args) { CurrentPreset(args).mDelayEnabled = NewValue<bool>(args); };

    const float timeType = 0.0f;
    FloatMetadata& time = TypeSpecificMetadata(members.RegisterProp("time", -1, kPropertyFloat, 0), timeType);
    static_cast<void>(Symbol("prop"));
    time.mHelp = String("Time in seconds or multiple of beat (if BeatSync is on).");
    time.mSlider = true;
    time.mMin = 0.0f;
    time.mMax = 4.0f;
    time.mUnits = Symbol("sec");
    time.mDefault = 0.0f;
    time.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    time.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mDelayTime; };
    time.mSetter = [](const PropAccessorArgs& args) { CurrentPreset(args).mDelayTime = NewValue<float>(args); };

    const bool beatSyncType = false;
    BoolMetadata& beatSync =
        TypeSpecificMetadata(members.RegisterProp("beat_sync", -1, kPropertyBool, 0), beatSyncType);
    static_cast<void>(Symbol("prop"));
    beatSync.mHelp = String("Sync to tempo?");
    beatSync.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    beatSync.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mDelayBeatSync; };
    beatSync.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mDelayBeatSync = NewValue<bool>(args);
    };

    const float tempoType = 0.0f;
    FloatMetadata& tempo =
        TypeSpecificMetadata(members.RegisterProp("default_tempo", -1, kPropertyFloat, 0), tempoType);
    static_cast<void>(Symbol("prop"));
    tempo.mHelp = String("Default tempo if no tempo set by host.");
    tempo.mSlider = true;
    tempo.mMin = 20.0f;
    tempo.mMax = 240.0f;
    tempo.mUnits = Symbol("BPM");
    tempo.mDefault = 120.0f;
    tempo.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    tempo.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mDelayDefaultTempo; };
    tempo.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mDelayDefaultTempo = NewValue<float>(args);
    };

    const float dryType = 0.0f;
    FloatMetadata& dry = TypeSpecificMetadata(members.RegisterProp("dry_gain", -1, kPropertyFloat, 0), dryType);
    static_cast<void>(Symbol("prop"));
    dry.mHelp = String("Gain applied to direct signal.");
    dry.mSlider = true;
    dry.mMin = 0.0f;
    dry.mMax = 1.0f;
    dry.mDefault = 1.0f;
    dry.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    dry.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mDelayDryGain; };
    dry.mSetter = [](const PropAccessorArgs& args) { CurrentPreset(args).mDelayDryGain = NewValue<float>(args); };

    const float wetType = 0.0f;
    FloatMetadata& wet = TypeSpecificMetadata(members.RegisterProp("wet_gain", -1, kPropertyFloat, 0), wetType);
    static_cast<void>(Symbol("prop"));
    wet.mHelp = String("Gain applied to delayed signal.");
    wet.mSlider = true;
    wet.mMin = 0.0f;
    wet.mMax = 1.0f;
    wet.mDefault = 0.5f;
    wet.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    wet.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mDelayWetGain; };
    wet.mSetter = [](const PropAccessorArgs& args) { CurrentPreset(args).mDelayWetGain = NewValue<float>(args); };

    const float feedbackType = 0.0f;
    FloatMetadata& feedback =
        TypeSpecificMetadata(members.RegisterProp("feedback_gain", -1, kPropertyFloat, 0), feedbackType);
    static_cast<void>(Symbol("prop"));
    feedback.mHelp = String("Gain applied to feedback signal.");
    feedback.mSlider = true;
    feedback.mMin = 0.0f;
    feedback.mMax = 1.0f;
    feedback.mDefault = 0.0f;
    feedback.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    feedback.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mDelayFeedbackGain; };
    feedback.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mDelayFeedbackGain = NewValue<float>(args);
    };
}

// The "filter_N_pre_clip" flag of a distortion filter: it runs before the
// clipper. The binary repeats the registration for each filter. Name not
// in the reference map.
template <int Index>
void RegisterPreClipProp(PropRegistry& registry, const char* name, const char* help) {
    const bool preClipType = false;
    BoolMetadata& preClip = TypeSpecificMetadata(registry.RegisterProp(name, -1, kPropertyBool, 0), preClipType);
    static_cast<void>(Symbol("prop"));
    preClip.mHelp = String(help);
    preClip.mDefault = false;
    preClip.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    preClip.mGetter = [](const PropAccessorArgs& args) {
        return CurrentPreset(args).mDistortionFilterPreClip[Index];
    };
    preClip.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mDistortionFilterPreClip[Index] = NewValue<bool>(args);
    };
}

// Reconstructed from eboot.elf at 0x6F480. The current preset's
// distortion: the gains, the clipping curve, oversampling and three
// filters, each before or after the clipper.
void RegisterDistortionProps(PropRegistry& registry) {
    PropRegistry& members = RegisterAccessorStruct(registry, "distortion");

    const bool enabledType = false;
    BoolMetadata& enabled =
        TypeSpecificMetadata(members.RegisterProp("enabled", -1, kPropertyBool, 0), enabledType);
    static_cast<void>(Symbol("prop"));
    enabled.mHelp = String("Is this patch using distortion?");
    enabled.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    enabled.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mDistortionEnabled; };
    enabled.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mDistortionEnabled = NewValue<bool>(args);
    };

    const float inputType = 0.0f;
    FloatMetadata& input =
        TypeSpecificMetadata(members.RegisterProp("input_gain_db", -1, kPropertyFloat, 0), inputType);
    static_cast<void>(Symbol("prop"));
    input.mHelp = String("Input gain in dB");
    input.mSlider = true;
    input.mMin = -50.0f;
    input.mMax = 50.0f;
    input.mUnits = Symbol("dB");
    input.mDefault = 0.0f;
    input.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    input.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mDistortionInputGainDb; };
    input.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mDistortionInputGainDb = NewValue<float>(args);
    };

    const float outputType = 0.0f;
    FloatMetadata& output =
        TypeSpecificMetadata(members.RegisterProp("output_gain_db", -1, kPropertyFloat, 0), outputType);
    static_cast<void>(Symbol("prop"));
    output.mHelp = String("Output gain in dB");
    output.mSlider = true;
    output.mMin = -50.0f;
    output.mMax = 50.0f;
    output.mUnits = Symbol("dB");
    output.mDefault = 0.0f;
    output.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    output.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mDistortionOutputGainDb; };
    output.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mDistortionOutputGainDb = NewValue<float>(args);
    };

    const int typeType = 0;
    IntMetadata<int>& type = TypeSpecificMetadata(members.RegisterProp("type", -1, kPropertyInt, 0), typeType);
    static_cast<void>(Symbol("prop"));
    type.mHelp = String("Type of distortion(0=clean,1=warm,2=dirty,3=soft,4=asymmetric)");
    type.mAllowedValues = DistortionEffect::Settings::GetAllowedTypes();
    type.mDefault = 0;
    type.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    type.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mDistortionType; };
    type.mSetter = [](const PropAccessorArgs& args) { CurrentPreset(args).mDistortionType = NewValue<int>(args); };

    const bool oversampleType = false;
    BoolMetadata& oversample =
        TypeSpecificMetadata(members.RegisterProp("oversample", -1, kPropertyBool, 0), oversampleType);
    static_cast<void>(Symbol("prop"));
    oversample.mHelp = String("Enable 4x oversampling for clipping stage.");
    oversample.mDefault = false;
    oversample.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    oversample.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mDistortionOversample; };
    oversample.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mDistortionOversample = NewValue<bool>(args);
    };

    RegisterPreClipProp<0>(members, "filter_1_pre_clip", "Filter 1 before or after the clipper");
    RegisterFilterMembers<DistortionFilter1>(RegisterAccessorStruct(members, "filter_1"), nullptr);
    RegisterPreClipProp<1>(members, "filter_2_pre_clip", "Filter 2 before or after the clipper");
    RegisterFilterMembers<DistortionFilter2>(RegisterAccessorStruct(members, "filter_2"), nullptr);
    RegisterPreClipProp<2>(members, "filter_3_pre_clip", "Filter 3 before or after the clipper");
    RegisterFilterMembers<DistortionFilter3>(RegisterAccessorStruct(members, "filter_3"), nullptr);
}

// The AmpSimulator parameter `Index` of the current preset's amp model,
// from 0 to `max`; read-only when the model does not use it, unless
// `alwaysEditable`. The binary repeats the registration for each
// parameter. Name not in the reference map.
template <int Index>
void RegisterAmpParameterProp(PropRegistry& registry, const char* name, float max, bool alwaysEditable) {
    const float parameterType = 0.0f;
    FloatMetadata& parameter =
        TypeSpecificMetadata(registry.RegisterProp(name, -1, kPropertyFloat, 0), parameterType);
    static_cast<void>(Symbol("prop"));
    parameter.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    parameter.mSlider = true;
    parameter.mMin = 0.0f;
    parameter.mMax = max;
    if (!alwaysEditable) {
        parameter.mReadOnly._Set([](const TagArgs& args) {
            return !AmpSimulator::IsParameterUsedByModel(CurrentPreset(args).mAmpModel, Index);
        });
    }
    parameter.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mAmpParameters[Index]; };
    parameter.mSetter = [](const PropAccessorArgs& args) {
        CurrentPreset(args).mAmpParameters[Index] = NewValue<float>(args);
    };
}

// Reconstructed from eboot.elf at 0x71760. The current preset's amp
// simulation: the "amp_model" struct of the model and its parameters, then
// the three EQ filters.
void RegisterAmpSimulationProps(PropRegistry& registry) {
    PropRegistry& members = RegisterAccessorStruct(registry, "amp_simulation");
    PropRegistry& model = RegisterAccessorStruct(members, "amp_model");

    const bool enabledType = false;
    BoolMetadata& enabled = TypeSpecificMetadata(model.RegisterProp("enabled", -1, kPropertyBool, 0), enabledType);
    static_cast<void>(Symbol("prop"));
    enabled.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    enabled.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mAmpEnabled; };
    enabled.mSetter = [](const PropAccessorArgs& args) { CurrentPreset(args).mAmpEnabled = NewValue<bool>(args); };

    const int modelType = 0;
    IntMetadata<int>& type = TypeSpecificMetadata(model.RegisterProp("model_type", -1, kPropertyInt, 0), modelType);
    static_cast<void>(Symbol("prop"));
    type.mHelp = String("Which amp model?");
    type.mAllowedValues._Set([](const TagArgs& args) {
        static_cast<void>(args);
        static const char* const kModels[] = {"JCM800", "AC30", "AD30", "ENGL", "Twin"};
        eastl::vector<AllowedValue<int>> values;
        for (int index = 0; index < 5; ++index) {
            values.push_back(AllowedValue<int>{index, String(kModels[index]), String(kModels[index])});
        }
        return values;
    });
    type.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };
    type.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mAmpModel; };
    type.mSetter = [](const PropAccessorArgs& args) { CurrentPreset(args).mAmpModel = NewValue<int>(args); };

    RegisterAmpParameterProp<0>(model, "presence", 10.0f, false);
    RegisterAmpParameterProp<1>(model, "bass", 10.0f, false);
    RegisterAmpParameterProp<2>(model, "middle", 10.0f, false);
    RegisterAmpParameterProp<3>(model, "treble", 10.0f, false);
    RegisterAmpParameterProp<4>(model, "master", 10.0f, false);
    RegisterAmpParameterProp<5>(model, "preamp", 10.0f, false);
    RegisterAmpParameterProp<6>(model, "channel_switch", 1.0f, false);
    RegisterAmpParameterProp<7>(model, "output_gain", 10.0f, true);

    RegisterFilterMembers<AmpFilter1>(RegisterAccessorStruct(members, "filter_1"), nullptr);
    RegisterFilterMembers<AmpFilter2>(RegisterAccessorStruct(members, "filter_2"), nullptr);
    RegisterFilterMembers<AmpFilter3>(RegisterAccessorStruct(members, "filter_3"), nullptr);
}

// The most presets a patch offers to add. Name not in the reference map.
const unsigned int kMaxPresets = 8;

// Reconstructed from eboot.elf at 0x73FB0. The preset selection: choosing
// the value after the last preset appends a copy of the current preset
// named "New Preset <n>". The current preset can be renamed and removed;
// the last one cannot be removed.
void RegisterPresetOptionsProps(const FusionPatchCom& prototype, PropRegistry& registry) {
    StructMetadata& options = static_cast<StructMetadata&>(
        *registry.RegisterProp("preset_options", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    options.mDisplayName = String("Presets");
    PropRegistry& members = static_cast<GroupMetadata&>(options).mRegistry;

    IntMetadata<unsigned long>& current = TypeSpecificMetadata(
        members.RegisterProp(
            "current", offsetof(FusionPatchCom, mCurrentPreset), kPropertyUInt64, sizeof(unsigned long)),
        prototype.mCurrentPreset);
    current.mDefault = prototype.mCurrentPreset;
    static_cast<void>(Symbol("prop"));
    current.mDisplayName = String("Select Preset");
    current.mHelp = String("Currently selected effect preset");
    current.mAllowedValues._Set([](const TagArgs& args) {
        const FusionPatchCom& patch = static_cast<const FusionPatchCom&>(args.mComponent);
        eastl::vector<AllowedValue<unsigned long>> values;
        for (unsigned long index = 0; index < patch.mPresets.size(); ++index) {
            const Symbol name = patch.mPresets[index].mName;
            FormatString format("%i: %s");
            format << index << name;
            const String label(format.Str());
            values.push_back(AllowedValue<unsigned long>{index, String(label.c_str()), String(name.Str())});
        }
        if (patch.mPresets.size() < kMaxPresets) {
            values.push_back(AllowedValue<unsigned long>{
                patch.mPresets.size(), String("(add new)"), String("Add a new preset to the end of the list")});
        }
        return values;
    });
    current.mSetter = [](const PropAccessorArgs& args) {
        FusionPatchCom* const patch = static_cast<FusionPatchCom*>(args.mComponent);
        const unsigned long index = NewValue<unsigned long>(args);
        if (index == patch->mPresets.size()) {
            patch->mPresets._Insert(index, &patch->mPresets[patch->mCurrentPreset]);
            FormatString format("New Preset %i");
            format << index;
            patch->mPresets[patch->mPresets.size() - 1].mName = Symbol(format.Str());
        }
        patch->mCurrentPreset = index <= patch->mPresets.size() ? index : 0;
    };
    current.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    const Symbol nameType;
    SymbolMetadata& name = TypeSpecificMetadata(members.RegisterProp("name", -1, kPropertySymbol, 0), nameType);
    static_cast<void>(Symbol("prop"));
    name.mDisplayName = String("Rename Preset");
    name.mGetter = [](const PropAccessorArgs& args) { return CurrentPreset(args).mName; };
    name.mSetter = [](const PropAccessorArgs& args) { CurrentPreset(args).mName = NewValue<Symbol>(args); };
    name.mOnChanged = [](const PropAccessorArgs& args) { MarkPatchChanged(args); };

    ActionMetadata& remove = static_cast<ActionMetadata&>(*members.RegisterAction("remove").mMetadata.Get());
    static_cast<void>(Symbol("action"));
    remove.mDisplayName = String("Remove Preset");
    remove.mAction = [](const TagArgs& args) {
        FusionPatchCom& patch = const_cast<FusionPatchCom&>(static_cast<const FusionPatchCom&>(args.mComponent));
        const unsigned long index = patch.mCurrentPreset;
        if (patch.mPresets.size() < 2 || index >= patch.mPresets.size()) {
            return;
        }
        RemoveElement(patch.mPresets, index);
        if (index >= patch.mPresets.size()) {
            patch.mCurrentPreset = index - 1;
        }
        if (patch.mResource != nullptr) {
            patch.mResource->mFileChangedOnDisk = true;
        }
    };
}

// Registers a filter struct of "preset_data" with its offset and the
// filter's defaults. Inlined into the builder. Name not in the reference
// map.
void RegisterFilterData(PropRegistry& registry, const char* name, int offset) {
    PropRegistry& members = static_cast<GroupMetadata&>(
                                *registry.RegisterProp(name, offset, kPropertyStruct, sizeof(BiquadFilter::Settings))
                                     .mMetadata.Get())
                                .mRegistry;
    static_cast<void>(Symbol("struct"));
    const BiquadFilter::Settings prototype;
    BoolMetadata& enabled = TypeSpecificMetadata(
        members.RegisterProp("enabled", offsetof(BiquadFilter::Settings, mEnabled), kPropertyBool, sizeof(bool)),
        prototype.mEnabled);
    enabled.mDefault = prototype.mEnabled;
    const signed char typeDefault = static_cast<signed char>(prototype.mType);
    IntMetadata<signed char>& type = TypeSpecificMetadata(
        members.RegisterProp("type", offsetof(BiquadFilter::Settings, mType), kPropertyInt8, 1), typeDefault);
    type.mDefault = typeDefault;
    FloatMetadata& frequency = TypeSpecificMetadata(
        members.RegisterProp("frequency", offsetof(BiquadFilter::Settings, mFrequency), kPropertyFloat, sizeof(float)),
        prototype.mFrequency);
    frequency.mDefault = prototype.mFrequency;
    FloatMetadata& q = TypeSpecificMetadata(
        members.RegisterProp("q", offsetof(BiquadFilter::Settings, mQ), kPropertyFloat, sizeof(float)), prototype.mQ);
    q.mDefault = prototype.mQ;
    FloatMetadata& gain = TypeSpecificMetadata(
        members.RegisterProp("gain", offsetof(BiquadFilter::Settings, mGain), kPropertyFloat, sizeof(float)),
        prototype.mGain);
    gain.mDefault = prototype.mGain;
}

// The member registry of a "preset_data" struct of the given place and
// size. Inlined into the builder. Name not in the reference map.
PropRegistry& RegisterDataStruct(PropRegistry& registry, const char* name, int offset, unsigned long size) {
    PropRegistry& members = static_cast<GroupMetadata&>(
                                *registry.RegisterProp(name, offset, kPropertyStruct, size).mMetadata.Get())
                                .mRegistry;
    static_cast<void>(Symbol("struct"));
    return members;
}

// Registers a float of "preset_data" with its default. Name not in the
// reference map.
void RegisterFloatData(PropRegistry& registry, const char* name, int offset, const float& prototype) {
    FloatMetadata& metadata =
        TypeSpecificMetadata(registry.RegisterProp(name, offset, kPropertyFloat, sizeof(float)), prototype);
    metadata.mDefault = prototype;
}
// Registers a bool of "preset_data" with its default. Name not in the
// reference map.
void RegisterBoolData(PropRegistry& registry, const char* name, int offset, const bool& prototype) {
    BoolMetadata& metadata =
        TypeSpecificMetadata(registry.RegisterProp(name, offset, kPropertyBool, sizeof(bool)), prototype);
    metadata.mDefault = prototype;
}
// Registers an int of "preset_data" with its default. Name not in the
// reference map.
void RegisterIntData(PropRegistry& registry, const char* name, int offset, const int& prototype) {
    IntMetadata<int>& metadata =
        TypeSpecificMetadata(registry.RegisterProp(name, offset, kPropertyInt, sizeof(int)), prototype);
    metadata.mDefault = prototype;
}
// Registers an unsigned byte of "preset_data" with its default. Name not
// in the reference map.
void RegisterByteData(PropRegistry& registry, const char* name, int offset, unsigned char prototype) {
    IntMetadata<unsigned char>& metadata =
        TypeSpecificMetadata(registry.RegisterProp(name, offset, kPropertyUInt8, 1), prototype);
    metadata.mDefault = prototype;
}

// Reconstructed from eboot.elf at 0x74320. The presets themselves, hidden
// from the editor, which edits the current one through the accessor
// properties; each property's default is a default-constructed preset's.
// The registrations do not build the "prop" symbol.
void RegisterPresetDataProps(PropRegistry& registry) {
    PropInfo& array = registry.RegisterPropArray(
        "preset_data", offsetof(FusionPatchCom, mPresets), sizeof(PropArray<PresetSettings>));
    ArrayMetadata& arrayMetadata = static_cast<ArrayMetadata&>(*array.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    arrayMetadata.mIsHidden = true;
    PropRegistry::RegisterPropArrayItem(array, kPropertyStruct, sizeof(PresetSettings));
    PropRegistry& members =
        static_cast<GroupMetadata&>(*arrayMetadata.mItemInfo.mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct_item"));
    const PresetSettings prototype;

    SymbolMetadata& name = TypeSpecificMetadata(
        members.RegisterProp("name", offsetof(PresetSettings, mName), kPropertySymbol, sizeof(Symbol)),
        prototype.mName);
    name.mDefault = prototype.mName;
    RegisterFloatData(members, "volume", offsetof(PresetSettings, mVolume), prototype.mVolume);
    RegisterFloatData(members, "pan", offsetof(PresetSettings, mPan), prototype.mPan);
    RegisterFloatData(
        members, "max_downward_pitch_bend", offsetof(PresetSettings, mMinPitchBendCents), prototype.mMinPitchBendCents);
    RegisterFloatData(
        members, "max_upward_pitch_bend", offsetof(PresetSettings, mMaxPitchBendCents), prototype.mMaxPitchBendCents);
    RegisterFloatData(members, "start_point", offsetof(PresetSettings, mStartPointMs), prototype.mStartPointMs);
    RegisterFloatData(members, "fine_tune_cents", offsetof(PresetSettings, mFineTuneCents), prototype.mFineTuneCents);
    RegisterIntData(members, "max_num_voices", offsetof(PresetSettings, mMaxNumVoices), prototype.mMaxNumVoices);
    RegisterIntData(
        members, "keyzone_select_mode", offsetof(PresetSettings, mKeyzoneSelectMode), prototype.mKeyzoneSelectMode);
    RegisterFilterData(members, "filter", offsetof(PresetSettings, mFilter));

    PropInfo& adsrs = members.RegisterPropArray(
        "adsrs", offsetof(PresetSettings, mADSRs), sizeof(PropArray<ADSR::Settings>));
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(adsrs, kPropertyStruct, sizeof(ADSR::Settings));
    PropRegistry& adsr =
        static_cast<GroupMetadata&>(*static_cast<ArrayMetadata&>(*adsrs.mMetadata.Get()).mItemInfo.mMetadata.Get())
            .mRegistry;
    static_cast<void>(Symbol("struct_item"));
    const ADSR::Settings adsrPrototype;
    RegisterBoolData(adsr, "enabled", offsetof(ADSR::Settings, mEnabled), adsrPrototype.mEnabled);
    RegisterByteData(adsr, "target", offsetof(ADSR::Settings, mTarget), adsrPrototype.mTarget);
    RegisterFloatData(adsr, "depth", offsetof(ADSR::Settings, mDepth), adsrPrototype.mDepth);
    RegisterFloatData(adsr, "attack", offsetof(ADSR::Settings, mAttack), adsrPrototype.mAttack);
    RegisterFloatData(adsr, "decay", offsetof(ADSR::Settings, mDecay), adsrPrototype.mDecay);
    RegisterFloatData(adsr, "sustain", offsetof(ADSR::Settings, mSustain), adsrPrototype.mSustain);
    RegisterFloatData(adsr, "release", offsetof(ADSR::Settings, mRelease), adsrPrototype.mRelease);

    PropInfo& lfos =
        members.RegisterPropArray("lfos", offsetof(PresetSettings, mLFOs), sizeof(PropArray<LFO::Settings>));
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(lfos, kPropertyStruct, sizeof(LFO::Settings));
    PropRegistry& lfo =
        static_cast<GroupMetadata&>(*static_cast<ArrayMetadata&>(*lfos.mMetadata.Get()).mItemInfo.mMetadata.Get())
            .mRegistry;
    static_cast<void>(Symbol("struct_item"));
    const LFO::Settings lfoPrototype;
    RegisterBoolData(lfo, "enabled", offsetof(LFO::Settings, mEnabled), lfoPrototype.mEnabled);
    RegisterByteData(lfo, "target", offsetof(LFO::Settings, mTarget), lfoPrototype.mTarget);
    RegisterFloatData(lfo, "depth", offsetof(LFO::Settings, mDepth), lfoPrototype.mDepth);
    RegisterByteData(lfo, "shape", offsetof(LFO::Settings, mShape), lfoPrototype.mShape);
    RegisterFloatData(lfo, "frequency", offsetof(LFO::Settings, mFrequency), lfoPrototype.mFrequency);
    RegisterBoolData(lfo, "retrigger", offsetof(LFO::Settings, mRetrigger), lfoPrototype.mRetrigger);
    RegisterFloatData(lfo, "initial_phase", offsetof(LFO::Settings, mInitialPhase), lfoPrototype.mInitialPhase);
    RegisterBoolData(lfo, "beat_sync", offsetof(LFO::Settings, mBeatSync), lfoPrototype.mBeatSync);
    RegisterFloatData(lfo, "default_tempo", offsetof(LFO::Settings, mDefaultTempo), lfoPrototype.mDefaultTempo);

    PropInfo& modulators = members.RegisterPropArray(
        "modulators", offsetof(PresetSettings, mModulators), sizeof(PropArray<Modulator::Settings>));
    static_cast<void>(Symbol("array"));
    PropRegistry::RegisterPropArrayItem(modulators, kPropertyStruct, sizeof(Modulator::Settings));
    PropRegistry& modulator = static_cast<GroupMetadata&>(
                                  *static_cast<ArrayMetadata&>(*modulators.mMetadata.Get()).mItemInfo.mMetadata.Get())
                                  .mRegistry;
    static_cast<void>(Symbol("struct_item"));
    const Modulator::Settings modulatorPrototype;
    RegisterByteData(modulator, "target", offsetof(Modulator::Settings, mTarget), modulatorPrototype.mTarget);
    RegisterFloatData(modulator, "range", offsetof(Modulator::Settings, mRange), modulatorPrototype.mRange);
    RegisterFloatData(modulator, "depth", offsetof(Modulator::Settings, mDepth), modulatorPrototype.mDepth);

    PropRegistry& portamento = RegisterDataStruct(
        members, "portamento", offsetof(PresetSettings, mPortamento), sizeof(FusionPatchCom::PortamentoSettings));
    RegisterBoolData(portamento, "enabled", 0, prototype.mPortamento.mEnabled);
    RegisterIntData(portamento, "mode", 4, prototype.mPortamento.mMode);
    RegisterFloatData(portamento, "time", 8, prototype.mPortamento.mTime);

    const int delay = offsetof(PresetSettings, mDelayEnabled);
    PropRegistry& delayMembers = RegisterDataStruct(
        members, "delay", delay, offsetof(PresetSettings, mDistortionEnabled) - delay);
    RegisterBoolData(delayMembers, "enabled", 0, prototype.mDelayEnabled);
    RegisterFloatData(delayMembers, "time", offsetof(PresetSettings, mDelayTime) - delay, prototype.mDelayTime);
    RegisterBoolData(delayMembers, "beat_sync", offsetof(PresetSettings, mDelayBeatSync) - delay, prototype.mDelayBeatSync);
    RegisterFloatData(
        delayMembers, "default_tempo", offsetof(PresetSettings, mDelayDefaultTempo) - delay, prototype.mDelayDefaultTempo);
    RegisterFloatData(delayMembers, "dry_gain", offsetof(PresetSettings, mDelayDryGain) - delay, prototype.mDelayDryGain);
    RegisterFloatData(delayMembers, "wet_gain", offsetof(PresetSettings, mDelayWetGain) - delay, prototype.mDelayWetGain);
    RegisterFloatData(
        delayMembers, "feedback_gain", offsetof(PresetSettings, mDelayFeedbackGain) - delay, prototype.mDelayFeedbackGain);

    const int distortion = offsetof(PresetSettings, mDistortionEnabled);
    PropRegistry& distortionMembers = RegisterDataStruct(
        members, "distortion", distortion, offsetof(PresetSettings, mBitCrusherEnabled) - distortion);
    RegisterBoolData(distortionMembers, "enabled", 0, prototype.mDistortionEnabled);
    RegisterFloatData(
        distortionMembers,
        "input_gain_db",
        offsetof(PresetSettings, mDistortionInputGainDb) - distortion,
        prototype.mDistortionInputGainDb);
    RegisterFloatData(
        distortionMembers,
        "output_gain_db",
        offsetof(PresetSettings, mDistortionOutputGainDb) - distortion,
        prototype.mDistortionOutputGainDb);
    RegisterIntData(
        distortionMembers, "type", offsetof(PresetSettings, mDistortionType) - distortion, prototype.mDistortionType);
    RegisterBoolData(
        distortionMembers,
        "oversample",
        offsetof(PresetSettings, mDistortionOversample) - distortion,
        prototype.mDistortionOversample);
    const int preClip = offsetof(PresetSettings, mDistortionFilterPreClip) - distortion;
    const int filters = offsetof(PresetSettings, mDistortionFilters) - distortion;
    RegisterBoolData(distortionMembers, "filter_1_pre_clip", preClip, prototype.mDistortionFilterPreClip[0]);
    RegisterFilterData(distortionMembers, "filter_1", filters);
    RegisterBoolData(distortionMembers, "filter_2_pre_clip", preClip + 1, prototype.mDistortionFilterPreClip[1]);
    RegisterFilterData(distortionMembers, "filter_2", filters + sizeof(BiquadFilter::Settings));
    RegisterBoolData(distortionMembers, "filter_3_pre_clip", preClip + 2, prototype.mDistortionFilterPreClip[2]);
    RegisterFilterData(distortionMembers, "filter_3", filters + 2 * sizeof(BiquadFilter::Settings));

    const int bitCrusher = offsetof(PresetSettings, mBitCrusherEnabled);
    PropRegistry& bitCrusherMembers = RegisterDataStruct(
        members, "bitcrusher", bitCrusher, offsetof(PresetSettings, mAmpParameters) - bitCrusher);
    RegisterBoolData(bitCrusherMembers, "enabled", 0, prototype.mBitCrusherEnabled);
    RegisterFloatData(bitCrusherMembers, "wet", offsetof(PresetSettings, mBitCrusherWet) - bitCrusher, prototype.mBitCrusherWet);
    RegisterFloatData(
        bitCrusherMembers,
        "crush_amount",
        offsetof(PresetSettings, mBitCrusherCrushAmount) - bitCrusher,
        prototype.mBitCrusherCrushAmount);
    RegisterIntData(
        bitCrusherMembers,
        "sample_and_hold_factor",
        offsetof(PresetSettings, mBitCrusherSampleAndHoldFactor) - bitCrusher,
        prototype.mBitCrusherSampleAndHoldFactor);

    const int amp = offsetof(PresetSettings, mAmpParameters);
    const int ampEnd = offsetof(PresetSettings, mAmpFilters) + 3 * sizeof(BiquadFilter::Settings);
    PropRegistry& ampMembers = RegisterDataStruct(members, "amp_simulation", amp, ampEnd - amp);
    PropRegistry& ampModel =
        RegisterDataStruct(ampMembers, "amp_model", 0, offsetof(PresetSettings, mAmpFilters) - amp);
    RegisterBoolData(ampModel, "enabled", offsetof(PresetSettings, mAmpEnabled) - amp, prototype.mAmpEnabled);
    RegisterIntData(ampModel, "model_type", offsetof(PresetSettings, mAmpModel) - amp, prototype.mAmpModel);
    static const char* const kParameters[8] = {
        "presence", "bass", "middle", "treble", "master", "preamp", "channel_switch", "output_gain"};
    for (int index = 0; index < 8; ++index) {
        RegisterFloatData(ampModel, kParameters[index], index * sizeof(float), prototype.mAmpParameters[index]);
    }
    const int ampFilters = offsetof(PresetSettings, mAmpFilters) - amp;
    RegisterFilterData(ampMembers, "filter_1", ampFilters);
    RegisterFilterData(ampMembers, "filter_2", ampFilters + sizeof(BiquadFilter::Settings));
    RegisterFilterData(ampMembers, "filter_3", ampFilters + 2 * sizeof(BiquadFilter::Settings));
}

}  // namespace

// Reconstructed from eboot.elf at 0x68040. The struct types are named for
// their arrays first. The description precedes the inherited registry;
// the builders then register the current preset's settings, the keyzones,
// the effects and the presets.
void FusionPatchCom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<ADSR::Settings>(Symbol("ADSR::Settings"));
    RegisterPropStruct<LFO::Settings>(Symbol("LFO::Settings"));
    RegisterPropStruct<Modulator::Settings>(Symbol("Modulator::Settings"));
    RegisterPropStruct<KeyzoneSettings>(Symbol("KeyzoneSettings"));
    RegisterPropStruct<PresetSettings>(Symbol("PresetSettings"));
    RegisterPropStruct<TrackChannelInfo>(Symbol("TrackChannelInfo"));
    metadata.mDescription = "Fusion Patch Settings";
    metadata.mAuthor = "Buzz Burrowes";
    metadata.mCategory = ComMetaData::kCategoryAudio;
    metadata.mAllowedResources.push_back(FusionPatchResource::Id());

    FusionPatchCom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }
    registry.SetCurrentRev(1);
    RegisterPresetProps(registry);
    RegisterADSRProps(registry);
    RegisterLFOProps(registry);
    RegisterFilterProps(registry);
    RegisterPortamentoProps(registry);
    RegisterModulatorProps(registry);
    RegisterKeyzoneProps(registry);
    RegisterBitCrusherProps(registry);
    RegisterDelayProps(registry);
    RegisterDistortionProps(registry);
    RegisterAmpSimulationProps(registry);
    RegisterPresetOptionsProps(prototype, registry);
    RegisterPresetDataProps(registry);
}

// Reconstructed from eboot.elf at 0x84320. A keyzone's sample loads from
// its uncached path, copied through a 512-character string. A missing or
// failed sample leaves the zone silent; a note-off zone whose audio data
// reports a nonzero slot 12 only names the component, for a warning the
// build compiles out.
bool FusionPatchCom::_OnResourcesLoaded() {
    const int numKeyzones = static_cast<int>(mKeyzones.size());
    mSampleResources.resize(numKeyzones);
    for (int index = 0; index < numKeyzones; ++index) {
        KeyzoneSettings& keyzone = mKeyzones[index];
        if (keyzone.mSamplePath == ResourcePath()) {
            continue;
        }
        const StackString<512> file(GetUncachedResourcePath(keyzone.mSamplePath));
        ResourcePath path;
        path = file.c_str();
        const ResourcePtr<AudioSampleResource> sample = _LoadSample(path);
        if (sample == nullptr || sample->Fail()) {
            static_cast<void>(MakeErrorName());
            keyzone.mSample = nullptr;
            continue;
        }
        mSampleResources[index] = sample.Get();
        keyzone.mSample = mSampleResources[index];
        if (keyzone.mTriggerOnNoteOff && sample->GetAudioData()->GetSampleFormat() != 0) {
            static_cast<void>(MakeErrorName());
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x84600.
ResourcePtr<AudioSampleResource> FusionPatchCom::_LoadSample(ResourcePath path) const {
    static Symbol sMogg;
    if (sMogg == Symbol()) {
        sMogg = Symbol("mogg");
    }
    if (Symbol(FileGetExt(path.Str(), false)) == sMogg) {
        const ResourcePtr<MoggSampleResource> sample = MoggSampleResource::GetOrCreate(path);
        return ResourcePtr<AudioSampleResource>(sample.Get());
    }
    const ResourcePtr<WaveResource> sample = Resource::GetOrLoad<WaveResource>(path, false);
    return ResourcePtr<AudioSampleResource>(sample.Get());
}

// Reconstructed from eboot.elf at 0x848C0.
void FusionPatchCom::_KeyZonePropertyChanged(unsigned long index) {
    if (mResource == nullptr) {
        return;
    }
    static Symbol sChanged;
    if (sChanged == Symbol()) {
        sChanged = Symbol("keyzone_property_changed");
    }
    Message<1> msg(sChanged);
    msg.mNodes[2] = DataNode(static_cast<int>(index));
    mResource->mEntity->Export(msg.mData);
    mResource->mFileChangedOnDisk = true;
}

// Reconstructed from eboot.elf at 0x84A80.
eastl::vector<AllowedValue<int>> FusionPatchCom::GetAllowedKeyzoneSelectModeValues() {
    eastl::vector<AllowedValue<int>> values;
    values.push_back(AllowedValue<int>{0, String("Layers"), String("All overlapping zones will play.")});
    values.push_back(AllowedValue<int>{
        1,
        String("Random"),
        String("Where there are overlapping zones, ONLY ONE will be randomly chosen to play.")});
    values.push_back(AllowedValue<int>{
        2,
        String("Random with Repetition"),
        String("Where there are overlapping zones, ONLY ONE will be randomly chosen to play. The last "
               "played keyzone will repeat sometimes.")});
    values.push_back(AllowedValue<int>{
        3,
        String("Cycle"),
        String("Where there are overlapping zones, ONLY ONE will play... each will play in turn.")});
    return values;
}

// Reconstructed from eboot.elf at 0x84D10.
eastl::vector<AllowedValue<int>> FusionPatchCom::PortamentoSettings::GetAllowedModeValues() {
    eastl::vector<AllowedValue<int>> values;
    values.push_back(AllowedValue<int>{
        kModeLegato, String("Legato"), String("Portamento pitch is reset when all notes are released.")});
    values.push_back(AllowedValue<int>{
        kModePersistent,
        String("Persistent"),
        String("Portamento pitch is maintained even when no notes are playing.")});
    return values;
}

// Reconstructed from eboot.elf at 0x84E80.
FusionPatchCom::KeyzoneSettings* FusionPatchCom::AddKeyZone(
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
    short sampledTempo) {
    static_cast<void>(unused);
    KeyzoneSettings* const keyzone = AddKeyZone(path);
    if (keyzone != nullptr) {
        keyzone->mMinNote = minNote;
        keyzone->mMaxNote = maxNote;
        keyzone->mRootNote = rootNote;
        keyzone->mMinVelocity = minVelocity;
        keyzone->mMaxVelocity = maxVelocity;
        keyzone->mUnpitched = unpitched;
        keyzone->mVelocityToVolume = velocityToVolume;
        keyzone->mMaintainTime = maintainTime;
        keyzone->mTimeStretchAlgorithm = timeStretchAlgorithm;
        keyzone->mFormantMode = formantMode;
        keyzone->mTempoSync = tempoSync;
        keyzone->mSampledTempo = sampledTempo;
    }
    return keyzone;
}

// Reconstructed from eboot.elf at 0x84EF0. The sample loads only when its
// file exists on disk.
FusionPatchCom::KeyzoneSettings* FusionPatchCom::AddKeyZone(const char* path) {
    if (mResource != nullptr) {
        static Symbol sPreCreate;
        if (sPreCreate == Symbol()) {
            sPreCreate = Symbol("pre_create_keyzone");
        }
        Message<1> msg(sPreCreate);
        msg.mNodes[2] = DataNode(static_cast<int>(mKeyzones.size()));
        mResource->mEntity->Export(msg.mData);
    }
    const KeyzoneSettings newKeyzone;
    mKeyzones._Insert(mKeyzones.size(), &newKeyzone);
    KeyzoneSettings& keyzone = mKeyzones[mKeyzones.size() - 1];
    if (path != nullptr && std::strcmp(path, Symbol().Str()) != 0) {
        keyzone.mSamplePath = path;
        if (FileExists(path, kReadNoBuffer)) {
            ResourcePath samplePath;
            samplePath = path;
            const ResourcePtr<AudioSampleResource> sample = _LoadSample(samplePath);
            if (sample != nullptr && !sample->Fail()) {
                keyzone.mSample = sample;
                mSampleResources.push_back(sample);
            }
        }
    }
    if (mResource != nullptr) {
        static Symbol sPostCreate;
        if (sPostCreate == Symbol()) {
            sPostCreate = Symbol("post_create_keyzone");
        }
        Message<0> msg(sPostCreate);
        mResource->mEntity->Export(msg.mData);
    }
    return &keyzone;
}

// Reconstructed from eboot.elf at 0x853B0. The copy takes the source's
// settings, track map, sample and path.
void FusionPatchCom::CloneKeyZone(unsigned long index) {
    AddKeyZone(mKeyzones[index].mSamplePath.Str());
    mKeyzones[mKeyzones.size() - 1] = mKeyzones[index];
}

// Reconstructed from eboot.elf at 0x85420. The zone's sample stays loaded.
void FusionPatchCom::DeleteKeyZone(unsigned long index) {
    if (mResource != nullptr) {
        static Symbol sPreDestroy;
        if (sPreDestroy == Symbol()) {
            sPreDestroy = Symbol("pre_destroy_keyzone");
        }
        Message<1> msg(sPreDestroy);
        msg.mNodes[2] = DataNode(static_cast<int>(index));
        mResource->mEntity->Export(msg.mData);
    }
    gAudioRenderTargets.mDefault->GetVoicePool()->KillVoices(&mKeyzones[index]);
    RemoveElement(mKeyzones, index);
    if (mResource != nullptr) {
        static Symbol sPostDestroy;
        if (sPostDestroy == Symbol()) {
            sPostDestroy = Symbol("post_destroy_keyzone");
        }
        Message<0> msg(sPostDestroy);
        mResource->mEntity->Export(msg.mData);
    }
}

// Reconstructed from eboot.elf at 0x76710. Both paths pass through an
// empty path helper at 0x245850, which this build compiles to nothing.
String FusionPatchCom::_GetPatchRelativePath(ResourcePath path) const {
    if (mResource == nullptr) {
        return String("");
    }
    char file[512];
    std::strcpy(file, path.Str());
    char folder[512];
    FileGetPath(mResource->mPath.Str(), folder);
    char relative[512] = {};
    return String(FileRelativePath(folder, file, relative));
}

// Reconstructed from eboot.elf at 0x76860. An empty path without a
// resource.
ResourcePath FusionPatchCom::_GetFullPathFromSample(const String& sample) const {
    ResourcePath path;
    if (mResource == nullptr) {
        path = "";
        return path;
    }
    char folderBuffer[512] = {};
    const String folder(FileGetPath(mResource->mPath.Str(), folderBuffer));
    char full[512] = {};
    path = FileMakePath(folder.c_str(), sample.c_str(), full);
    return path;
}

// Reconstructed from eboot.elf at 0x76B20.
bool FusionPatchCom::_LoadFromDTAFile(ResourcePath path) {
    const DataArrayPtr data = DataReadFile(path.Str(), false);
    return _LoadFromDataArray(data, path);
}

// Reconstructed from eboot.elf at 0x85710.
Symbol FusionPatchCom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x85720.
Symbol FusionPatchCom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x85730.
int FusionPatchCom::CurrentRev() const {
    return const_cast<FusionPatchCom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x85750.
bool FusionPatchCom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x85780.
Component* FusionPatchCom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x85790. The copy goes at the buffer's
// next 8-byte boundary, without the resource or the loaded samples, and its
// properties follow it.
char* FusionPatchCom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<FusionPatchCom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) FusionPatchCom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x859F0.
PropRegistry& FusionPatchCom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x85A00.
ComMetaData& FusionPatchCom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0xD880. The binary emits the factory in
// audio/SoundManager.o with the class's Init.
Component* FusionPatchCom::_Create() {
    return new FusionPatchCom();
}

// Reconstructed from eboot.elf at 0x5350. InitComponentClass's shape, but
// the superclass flag is left as it is; the binary emits the registration
// in audio/SoundManager.o, which calls it.
void FusionPatchCom::Init() {
    static const long sHeap = MemFindHeap("metadata");
    MemPushHeap(sHeap);
    const Symbol name = sClassName;
    sMetaData.mCreatable = true;
    sMetaData.mAllowMultiple = false;
    _Init(sPropRegistry, sMetaData);
    RegisterPropTypeSize<FusionPatchCom>(name);
    FusionPatchCom prototype;
    Component::sFactory[name] = _Create;
    Component::sVtables[name] = [](void* storage) -> Component* {
        return new (storage) FusionPatchCom();
    };
    const bool hasPostPoll = prototype._HasPostPoll();
    sMetaData.Init(name, name, sPropRegistry, sizeof(FusionPatchCom), hasPostPoll, false);
    MemPopHeap();
}
