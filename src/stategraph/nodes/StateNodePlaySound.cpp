// stategraph/StateNodePlaySound.o (0x785090 to 0x78CF86). The class's
// registration and factory are emitted in stategraph/StateGraph.o, with the
// DrivenAudioParamSettings constructor. The object also holds this file's
// copies of DrivenProp<Symbol>::Register (0x7879D0) and
// StateBase::_FixDrivenProp<Symbol> (0x788180), and the std::function
// wrappers of the tag functions.
#include "stategraph/nodes/StateNodePlaySound.h"

#include <math.h>
#include <new>

#include "audio/core/components/AudioEmitterCom.h"
#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/generators/DialogGenerator.h"
#include "audio/core/music/MusicGenerator.h"
#include "audio/core/system/SoundManager.h"
#include "entity/core/Entity.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "stategraph/conditions/StateConditionTimeElapsed.h"
#include "stategraph/drivers/StateGraphDriverCom.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "utl/time/TimeMgr.h"

namespace {

// The id of no object, at 0x1AB8F08, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The "params" element the path's array index names. Inlined into the tag
// functions and the accessors. Name not in the reference map.
DrivenAudioParamSettings& ParamAt(const StateNodePlaySound& node, const PropPath& path) {
    return node.mParams[path[1].mValue];
}

// Clamps the element's value to the range of the event's parameter of its
// name, as a plain value. Inlined into the accessors. Name not in the
// reference map.
void ClampParamToEvent(const StateNodePlaySound& node, DrivenAudioParamSettings& param) {
    EventParameterInfo info;
    const Symbol event(node.GetSoundEventName());
    if (GetEventParameter(event.Str(), param.mName.Str(), &info)) {
        const float value = param.mValue.mValue;
        param.mValue = DrivenProp<float>(
            info.mMaximum < value ? info.mMaximum : (value > info.mMinimum ? value : info.mMinimum));
    }
}

// "use_defined_param" switches to the event's parameter of the name and
// clamps the value to its range. Name not in the reference map.
// Reconstructed from eboot.elf at 0x789F10.
void SetUseDefinedParamAccessor(const PropAccessorArgs& args) {
    StateNodePlaySound* const node = static_cast<StateNodePlaySound*>(args.mComponent);
    DrivenAudioParamSettings& param = ParamAt(*node, *static_cast<const PropPath*>(args.mPath));
    const bool useDefined = *static_cast<const bool*>(args.mValue);
    param.mUseDefinedParam = useDefined;
    if (useDefined) {
        ClampParamToEvent(*node, param);
    }
}

// "defined_name" names the parameter and clamps the value to its range.
// Name not in the reference map.
// Reconstructed from eboot.elf at 0x78A0B0.
void SetDefinedNameAccessor(const PropAccessorArgs& args) {
    StateNodePlaySound* const node = static_cast<StateNodePlaySound*>(args.mComponent);
    DrivenAudioParamSettings& param = ParamAt(*node, *static_cast<const PropPath*>(args.mPath));
    param.mName = *static_cast<const Symbol*>(args.mValue);
    ClampParamToEvent(*node, param);
}

// The parameters the sound starts with: each one whose event has it, set
// at once or already at the event's default. The others blend in _Poll.
// Name not in the reference map.
// Reconstructed from eboot.elf at 0x7894D0.
void GetInitialParams(
    const char* event,
    const PropArray<DrivenAudioParamSettings>& params,
    PlayArgs::ParameterList& list,
    Entity* entity) {
    for (unsigned int i = 0; i < params.size(); ++i) {
        const float value = params[i].mValue.GetValue(entity);
        float defaultValue = 0.0F;
        if (GetEventParameterDefault(event, params[i].mName.Str(), &defaultValue)) {
            if (params[i].mDuration == 0.0F || defaultValue == value) {
                list.push_back(PlayArgs::ParameterValue{params[i].mName, value});
            }
        }
    }
}

// Advances the sound's parameters: an immediate persistent one is set every
// frame; one with a duration blends to the driven value along its curve,
// starting again when the value changes, and a persistent one keeps
// following it once the blend is done. Name not in the reference map.
// Reconstructed from eboot.elf at 0x7897D0.
void PollParams(
    const PropArray<DrivenAudioParamSettings>& params,
    eastl::vector<StateNodePlaySound::ParamState>& states,
    unsigned int handle,
    StateGraphInstance* graph) {
    AudioGenerator* const generator = theSoundManager.LockIfOwned(handle);
    if (generator == nullptr) {
        return;
    }
    Entity* const entity = graph->mEntity;
    for (unsigned int i = 0; i < params.size(); ++i) {
        StateNodePlaySound::ParamState& state = states[i];
        if (!state.mValid) {
            continue;
        }
        const DrivenAudioParamSettings& param = params[i];
        if (0.0F >= param.mDuration) {
            if (param.mPersistent) {
                generator->SetParameter(param.mName, param.mValue.GetValue(entity));
            }
            continue;
        }
        if (!param.mPersistent && state.mDone) {
            continue;
        }
        const float target = param.mValue.GetValue(entity);
        if (target == state.mTarget) {
            if (!state.mBlending) {
                continue;
            }
        } else {
            state.mElapsed = 0.0F;
            state.mTarget = target;
            if (target == state.mCurrent) {
                state.mBlending = false;
                continue;
            }
            state.mStartValue = state.mCurrent;
            state.mBlending = true;
        }
        const int units = graph->ResolveTimeUnits(param.mUnits);
        state.mElapsed += graph->mClock->DeltaTime(static_cast<TimeUnits>(units));
        const float fraction = state.mElapsed / param.mDuration;
        const float clamped = 1.0F < fraction ? 1.0F : (fraction > 0.0F ? fraction : 0.0F);
        const float blend = param.mBlendCurve.Evaluate(clamped);
        state.mCurrent = (state.mTarget - state.mStartValue) * blend + state.mStartValue;
        if (state.mElapsed < param.mDuration) {
            generator->SetParameter(param.mName, state.mCurrent);
            state.mBlending = true;
        } else {
            state.mCurrent = state.mTarget;
            generator->SetParameter(param.mName, state.mTarget);
            state.mBlending = false;
            state.mDone = true;
        }
    }
    --generator->mRefCount;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x78CED0).
Symbol StateNodePlaySound::sId("StateNodePlaySound");
Symbol StateNodePlaySound::sClassName("StateNodePlaySound");
PropRegistry StateNodePlaySound::sPropRegistry;
ComMetaData StateNodePlaySound::sMetaData;

// Reconstructed from eboot.elf at 0x785090. The sound stops on exit and a
// dialog line may be interrupted by default.
StateNodePlaySound::StateNodePlaySound()
    : mUsePreloadedBank(false),
      mPreloadedSound(),
      mSound(),
      mAudioEmitter(gNullObjectId),
      mInstanceName(),
      mInstanceNameIsGlobal(false),
      mIsMasterMusic(false),
      mIsDialog(false),
      mIsInterruptible(true),
      mStopOnExit(true),
      mParams(),
      mStartMuted(false),
      mVolume() {}

// Reconstructed from eboot.elf at 0x785190. The tag functions are lambdas
// (0x78B140 to 0x78CA30). The sound is driven from revision 1 on; an older
// one holds a plain symbol.
void StateNodePlaySound::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodePlaySound prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    BoolMetadata& usePreloaded = TypeSpecificMetadata(
        registry.RegisterProp("use_preloaded_bank", offsetof(StateNodePlaySound, mUsePreloadedBank), kPropertyBool, 1),
        prototype.mUsePreloadedBank);
    usePreloaded.mDefault = prototype.mUsePreloadedBank;
    static_cast<void>(Symbol("prop"));
    usePreloaded.mHelp = String("Enable this if you want to use a bank that is preloaded via PreloaderCom.");

    GroupMetadata& sound = prototype.mSound.Register(
        registry, Symbol("sound"), offsetof(StateNodePlaySound, mSound), StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    sound.mDisplayName = String("sound event");
    sound.mHelp = String("The sound event to play.");
    // At 0x78B140.
    sound.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodePlaySound&>(args.mComponent).mUsePreloadedBank;
    });

    SymbolMetadata& preloadedSound = TypeSpecificMetadata(
        registry.RegisterProp(
            "preloaded_sound", offsetof(StateNodePlaySound, mPreloadedSound), kPropertySymbol, sizeof(Symbol)),
        prototype.mPreloadedSound);
    preloadedSound.mDefault = prototype.mPreloadedSound;
    static_cast<void>(Symbol("prop"));
    preloadedSound.mDisplayName = String("sound event");
    preloadedSound.mHelp =
        String("The sound event to play from a bank listed in a PreloaderCom on your current entity.");
    // At 0x78B240.
    preloadedSound.mIsHidden._Set([](const TagArgs& args) {
        return !static_cast<const StateNodePlaySound&>(args.mComponent).mUsePreloadedBank;
    });
    // At 0x789F00.
    preloadedSound.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<StateBase*>(args.mComponent)->_SendDisplayTextUpdated();
    };
    // At 0x78B340: the events of the banks the entity has loaded.
    preloadedSound.mAllowedValues._Set([](const TagArgs& args) {
        eastl::vector<Symbol> events;
        theSoundManager.GetLoadedEvents(
            static_cast<const StateNodePlaySound&>(args.mComponent)._GetContextEntityPrototype(), &events);
        eastl::vector<AllowedValue<Symbol>> values;
        for (unsigned long i = 0; i < events.size(); ++i) {
            values.push_back(AllowedValue<Symbol>{events[i], String(events[i].Str()), String("")});
        }
        return values;
    });

    GameObjectIdMetadata& audioEmitter = TypeSpecificMetadata(
        registry.RegisterProp(
            "audio_emitter", offsetof(StateNodePlaySound, mAudioEmitter), kPropertyGameObjectId, sizeof(GameObjectId)),
        prototype.mAudioEmitter);
    audioEmitter.mDefault = prototype.mAudioEmitter;
    static_cast<void>(Symbol("prop"));
    audioEmitter.mHelp = String("The audio emitter object to use; if not defined will use the default 2D emitter.");
    audioEmitter.mRequiredComs = {AudioEmitterCom::sId};
    audioEmitter.mEntity.mMode = TagBase<Entity*>::kModeFunction;
    audioEmitter.mEntity.mFunction = StateBase::_GetContextEntityTagFunc();

    SymbolMetadata& instanceName = TypeSpecificMetadata(
        registry.RegisterProp(
            "instance_name", offsetof(StateNodePlaySound, mInstanceName), kPropertySymbol, sizeof(Symbol)),
        prototype.mInstanceName);
    instanceName.mDefault = prototype.mInstanceName;
    static_cast<void>(Symbol("prop"));
    instanceName.mHelp = String(
        "An optional name by which this particular instance of the sound can be interacted with later on, by a "
        "modify sound node.");

    BoolMetadata& isGlobal = TypeSpecificMetadata(
        registry.RegisterProp(
            "instance_name_is_global", offsetof(StateNodePlaySound, mInstanceNameIsGlobal), kPropertyBool, 1),
        prototype.mInstanceNameIsGlobal);
    isGlobal.mDefault = prototype.mInstanceNameIsGlobal;
    static_cast<void>(Symbol("prop"));
    isGlobal.mHelp = String(
        "If true, the \"instance name\" will be placed in the global list of sounds instead of the local list of "
        "sounds.");
    // At 0x78B5C0.
    isGlobal.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const StateNodePlaySound&>(args.mComponent).mInstanceName == Symbol();
    });

    BoolMetadata& stopOnExit = TypeSpecificMetadata(
        registry.RegisterProp("stop_on_exit", offsetof(StateNodePlaySound, mStopOnExit), kPropertyBool, 1),
        prototype.mStopOnExit);
    stopOnExit.mDefault = prototype.mStopOnExit;
    static_cast<void>(Symbol("prop"));
    stopOnExit.mHelp = String(
        "If true, this sound will be stopped when this node exits. Otherwise, the sound will keep playing. Remember "
        "to name the instance if you want to modify it later with a ModifySound node.");

    BoolMetadata& isMasterMusic = TypeSpecificMetadata(
        registry.RegisterProp("is_master_music", offsetof(StateNodePlaySound, mIsMasterMusic), kPropertyBool, 1),
        prototype.mIsMasterMusic);
    isMasterMusic.mDefault = prototype.mIsMasterMusic;
    static_cast<void>(Symbol("prop"));
    isMasterMusic.mHelp = String("The sound will be played as the 'Master Music' on the specified emitter.");
    // At 0x78B6D0.
    isMasterMusic.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const StateNodePlaySound&>(args.mComponent).mIsDialog;
    });

    // The binary repeats the master music help text here.
    BoolMetadata& isDialog = TypeSpecificMetadata(
        registry.RegisterProp("is_dialog", offsetof(StateNodePlaySound, mIsDialog), kPropertyBool, 1),
        prototype.mIsDialog);
    isDialog.mDefault = prototype.mIsDialog;
    static_cast<void>(Symbol("prop"));
    isDialog.mHelp = String("The sound will be played as the 'Master Music' on the specified emitter.");
    // At 0x78B7D0.
    isDialog.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const StateNodePlaySound&>(args.mComponent).mIsMasterMusic;
    });

    BoolMetadata& isInterruptible = TypeSpecificMetadata(
        registry.RegisterProp("is_interruptible", offsetof(StateNodePlaySound, mIsInterruptible), kPropertyBool, 1),
        prototype.mIsInterruptible);
    isInterruptible.mDefault = prototype.mIsInterruptible;
    static_cast<void>(Symbol("prop"));
    isInterruptible.mHelp = String(
        "APPLIES ONLY TO DIALOG! NOTE: Will be overridden if emitter is setup to allow multiple dialog lines at a "
        "time.");
    // At 0x78B8D0.
    isInterruptible.mIsHidden._Set([](const TagArgs& args) {
        return !static_cast<const StateNodePlaySound&>(args.mComponent).mIsDialog;
    });

    PropInfo& paramsInfo = registry.RegisterPropArray(
        "params", offsetof(StateNodePlaySound, mParams), sizeof(PropArray<DrivenAudioParamSettings>));
    ArrayMetadata& params = static_cast<ArrayMetadata&>(*paramsInfo.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    params.mHelp = String("A way to set the initial value of Fmod Params built into the event.");
    PropRegistry::RegisterPropArrayItem(paramsInfo, kPropertyStruct, sizeof(DrivenAudioParamSettings));
    PropRegistry& item = static_cast<GroupMetadata&>(*params.mItemInfo.mMetadata.Get()).mRegistry;
    static_cast<void>(Symbol("struct_item"));
    DrivenAudioParamSettings itemType;

    // The binary repeats the preloaded bank help text here.
    BoolMetadata& useDefined = TypeSpecificMetadata(
        item.RegisterProp(
            "use_defined_param", offsetof(DrivenAudioParamSettings, mUseDefinedParam), kPropertyBool, 1),
        itemType.mUseDefinedParam);
    useDefined.mDefault = itemType.mUseDefinedParam;
    static_cast<void>(Symbol("prop"));
    useDefined.mHelp = String("Enable this if you want to use a bank that is preloaded via PreloaderCom.");
    useDefined.mSetter = SetUseDefinedParamAccessor;

    SymbolMetadata& name = TypeSpecificMetadata(
        item.RegisterProp("name", offsetof(DrivenAudioParamSettings, mName), kPropertySymbol, sizeof(Symbol)),
        itemType.mName);
    name.mDefault = itemType.mName;
    static_cast<void>(Symbol("prop"));
    name.mDisplayName = String("name");
    name.mHelp = String("The parameter to adjust.");
    // At 0x78B9D0.
    name.mIsHidden._Set([](const TagArgs& args) {
        return ParamAt(static_cast<const StateNodePlaySound&>(args.mComponent), args.mPath).mUseDefinedParam;
    });

    SymbolMetadata& definedName = TypeSpecificMetadata(
        item.RegisterProp("defined_name", offsetof(DrivenAudioParamSettings, mName), kPropertySymbol, sizeof(Symbol)),
        itemType.mName);
    definedName.mDefault = itemType.mName;
    static_cast<void>(Symbol("prop"));
    definedName.mDisplayName = String("name");
    definedName.mHelp = String("The parameter to adjust.");
    // At 0x78BAE0.
    definedName.mIsHidden._Set([](const TagArgs& args) {
        return !ParamAt(static_cast<const StateNodePlaySound&>(args.mComponent), args.mPath).mUseDefinedParam;
    });
    // At 0x78BC00: the parameters of the sound event. The binary also
    // builds an empty list it does not use.
    definedName.mAllowedValues._Set([](const TagArgs& args) {
        eastl::vector<Symbol> unused;
        const Symbol event(static_cast<const StateNodePlaySound&>(args.mComponent).GetSoundEventName());
        const int count = GetEventParameterCount(event.Str());
        eastl::vector<AllowedValue<Symbol>> values;
        EventParameterInfo info;
        for (int i = 0; i < count; ++i) {
            if (!GetEventParameterByIndex(event.Str(), i, &info)) {
                break;
            }
            values.push_back(AllowedValue<Symbol>{info.mName, String(info.mName.Str()), String("")});
        }
        return values;
    });
    definedName.mSetter = SetDefinedNameAccessor;

    GroupMetadata& value = itemType.mValue.Register(
        item, Symbol("value"), offsetof(DrivenAudioParamSettings, mValue), StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    PropPath::Node valueNode;
    PropPath valuePath;
    valuePath.PushNode(valueNode = Symbol("value"));
    FloatMetadata& valueValue = TypeSpecificMetadata(
        const_cast<PropInfo&>(*value.mRegistry.FindProp(valuePath, nullptr)), itemType.mValue.mValue);
    static_cast<void>(Symbol("driven_prop_value"));
    // At 0x78BEF0 and 0x78C0C0: the range of the event's parameter.
    valueValue.mMin._Set([](const TagArgs& args) {
        const StateNodePlaySound& node = static_cast<const StateNodePlaySound&>(args.mComponent);
        const Symbol event(node.GetSoundEventName());
        EventParameterInfo info;
        const DrivenAudioParamSettings& param = ParamAt(node, args.mPath);
        if (param.mUseDefinedParam && GetEventParameter(event.Str(), param.mName.Str(), &info)) {
            return info.mMinimum;
        }
        return -1000000.0F;
    });
    valueValue.mMax._Set([](const TagArgs& args) {
        const StateNodePlaySound& node = static_cast<const StateNodePlaySound&>(args.mComponent);
        const Symbol event(node.GetSoundEventName());
        EventParameterInfo info;
        const DrivenAudioParamSettings& param = ParamAt(node, args.mPath);
        if (param.mUseDefinedParam && GetEventParameter(event.Str(), param.mName.Str(), &info)) {
            return info.mMaximum;
        }
        return 1000000.0F;
    });

    BoolMetadata& persistent = TypeSpecificMetadata(
        item.RegisterProp("persistent", offsetof(DrivenAudioParamSettings, mPersistent), kPropertyBool, 1),
        itemType.mPersistent);
    persistent.mDefault = itemType.mPersistent;
    static_cast<void>(Symbol("prop"));
    persistent.mHelp = String("If checked, this node will continually enforce the specified value each frame.");
    // At 0x78C290.
    persistent.mReadOnly._Set([](const TagArgs& args) {
        return !ParamAt(static_cast<const StateNodePlaySound&>(args.mComponent), args.mPath).mValue.mDriven;
    });

    FloatMetadata& duration = TypeSpecificMetadata(
        item.RegisterProp("duration", offsetof(DrivenAudioParamSettings, mDuration), kPropertyFloat, 4),
        itemType.mDuration);
    duration.mDefault = itemType.mDuration;
    static_cast<void>(Symbol("prop"));
    duration.mHelp = String("How long it should take to get from the current value to the new value.");
    duration.mMin = 0.0F;

    IntMetadata<int>& units = static_cast<IntMetadata<int>&>(
        *item.RegisterProp("units", offsetof(DrivenAudioParamSettings, mUnits), kPropertyInt, 4).mMetadata.Get());
    units.mDefault = itemType.mUnits;
    static_cast<void>(Symbol("prop"));
    units.mHelp = String("The time units used to measure the duration");
    units.mAllowedValues.mMode = TagBase<eastl::vector<AllowedValue<int>>>::kModeFunction;
    units.mAllowedValues.mFunction = gTimeUnitsAllowedValues;
    // At 0x78C3B0.
    units.mIsHidden._Set([](const TagArgs& args) {
        return 0.0F >= ParamAt(static_cast<const StateNodePlaySound&>(args.mComponent), args.mPath).mDuration;
    });

    FloatWaveformMetadata& blendCurve = static_cast<FloatWaveformMetadata&>(TypeSpecificMetadata(
        item.RegisterProp(
            "blend_curve",
            offsetof(DrivenAudioParamSettings, mBlendCurve),
            kPropertyWaveformFloat,
            sizeof(Waveform<float>)),
        itemType.mBlendCurve));
    blendCurve.mDefault = Waveform<float>(itemType.mBlendCurve);
    static_cast<void>(Symbol("prop"));
    blendCurve.mHelp = String("A curve defining the blend ratio over time");
    blendCurve.mMin = 0.0F;
    blendCurve.mMax = 1.0F;
    blendCurve.mAllowedTypes = {2};
    // At 0x78C4D0.
    blendCurve.mIsHidden._Set([](const TagArgs& args) {
        return 0.0F >= ParamAt(static_cast<const StateNodePlaySound&>(args.mComponent), args.mPath).mDuration;
    });

    BoolMetadata& startMuted = TypeSpecificMetadata(
        registry.RegisterProp("start_muted", offsetof(StateNodePlaySound, mStartMuted), kPropertyBool, 1),
        prototype.mStartMuted);
    startMuted.mDefault = prototype.mStartMuted;
    static_cast<void>(Symbol("prop"));
    startMuted.mHelp = String("Initially muted?");

    GroupMetadata& volume = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "volume", offsetof(StateNodePlaySound, mVolume), kPropertyStruct, sizeof(VolumeSettings))
                                                              .mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    VolumeSettings volumeType;
    volume.mHelp = String("Volume to set on the sound (if it's still valid).");
    PropRegistry& volumeRegistry = volume.mRegistry;

    BoolMetadata& volumeEnable = TypeSpecificMetadata(
        volumeRegistry.RegisterProp("enable", offsetof(VolumeSettings, mEnable), kPropertyBool, 1), volumeType.mEnable);
    volumeEnable.mDefault = volumeType.mEnable;
    static_cast<void>(Symbol("prop"));
    volumeEnable.mHelp = String("Enable or disable setting of volume.");

    GroupMetadata& volumeValue = volumeType.mValue.Register(
        volumeRegistry, Symbol("value"), offsetof(VolumeSettings, mValue), StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    volumeValue.mHelp = String("New volume if dbFS (-96.0 == off, 0.0 == normal).");
    // At 0x78C5F0.
    volumeValue.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const StateNodePlaySound&>(args.mComponent).mVolume.mEnable;
    });

    BoolMetadata& volumePersistent = TypeSpecificMetadata(
        volumeRegistry.RegisterProp("persistent", offsetof(VolumeSettings, mPersistent), kPropertyBool, 1),
        volumeType.mPersistent);
    volumePersistent.mDefault = volumeType.mPersistent;
    static_cast<void>(Symbol("prop"));
    volumePersistent.mHelp = String("If checked, this node will continually enforce the specified value each frame.");
    // At 0x78C6F0.
    volumePersistent.mReadOnly._Set([](const TagArgs& args) {
        const VolumeSettings& settings = static_cast<const StateNodePlaySound&>(args.mComponent).mVolume;
        return !settings.mEnable || !settings.mValue.mDriven;
    });

    FloatMetadata& seconds = TypeSpecificMetadata(
        volumeRegistry.RegisterProp("seconds", offsetof(VolumeSettings, mSeconds), kPropertyFloat, 4),
        volumeType.mSeconds);
    seconds.mDefault = volumeType.mSeconds;
    static_cast<void>(Symbol("prop"));
    seconds.mHelp = String("How long it should take to get from the current volume to the new volume.");
    // At 0x78C800.
    seconds.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const StateNodePlaySound&>(args.mComponent).mVolume.mEnable;
    });
    seconds.mMin = 0.0F;

    IntMetadata<int>& option = static_cast<IntMetadata<int>&>(
        *volumeRegistry.RegisterProp("option", offsetof(VolumeSettings, mOption), kPropertyInt, 4).mMetadata.Get());
    option.mDefault = volumeType.mOption;
    static_cast<void>(Symbol("prop"));
    option.mHelp = String("What should happen to the sound when the adjustment is complete.");
    option.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            AudioGenerator::kPostFadeNone, String("Keep Playing"), String("Just keep playing the sound.")},
        AllowedValue<int>{AudioGenerator::kPostFadeStop, String("Stop"), String("Stop the sound.")}};
    // At 0x78C900.
    option.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const StateNodePlaySound&>(args.mComponent).mVolume.mEnable;
    });

    StateBase::_FixDrivenProp<Symbol>(registry, PropPath(Symbol("sound")));

    // At 0x78CA30: the sound saved before revision 1 was a plain symbol.
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev > 0) {
                return;
            }
            StateNodePlaySound* const node = static_cast<StateNodePlaySound*>(args.mComponent);
            node->mSound.mValue = GetPropValue<Symbol>(*args.mSavedProps, PropPath(Symbol("sound")));
        };
    if (&registry == &own) {
        own.SetCurrentRev(1);
        own.mRevisionFunc = onRevision;
    }

    StateNode::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "A state that plays a sound.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("p");
}

// Reconstructed from eboot.elf at 0x788790.
const char* StateNodePlaySound::GetEditorText() const {
    return GetSoundEventName();
}

// Reconstructed from eboot.elf at 0x7887E0.
StateNodePlaySound::Instance::Instance(const StateNodePlaySound& node, StateGraphInstance& graph)
    : StateNodeInstance(node, graph), mPlaySound(&node), mHandle(0), mParamStates(), mVolumeDb(0.0F) {
    mParamStates.resize(node.mParams.size());
}

// Reconstructed from eboot.elf at 0x788880. The deleting destructor at
// 0x788960 returns the instance to the pool.
StateNodePlaySound::Instance::~Instance() {
    if (mPlaySound->mStopOnExit) {
        AudioGenerator* const generator = theSoundManager.LockIfOwned(mHandle);
        if (generator != nullptr) {
            generator->Stop();
            --generator->mRefCount;
        }
        mGraph->mEntity->GetRoot()->GetCom<StateGraphDriverCom>()->RemoveSoundHandle(mHandle, kSoundHandleLocal);
    }
}

// Reconstructed from eboot.elf at 0x788990. The sound plays on the
// emitter object's AudioEmitterCom, or the default 2D emitter, as master
// music, dialog or a plain sound. The parameters with a duration read their
// start values from the playing sound. A sound the driver refuses to name
// is stopped.
void StateNodePlaySound::Instance::_Enter() {
    const StateNodePlaySound* node = mPlaySound;
    const Symbol sound = node->mUsePreloadedBank ? node->mPreloadedSound : node->mSound.GetValue(_GetContextEntity());
    if (sound == Symbol()) {
        return;
    }

    Entity* const entity = _GetContextEntity();
    AudioEmitterCom* emitter = static_cast<AudioEmitterCom*>(theSoundManager.GetDefault2DEmitterComponent());
    if (entity != nullptr) {
        const GameObjectId id = mPlaySound->mAudioEmitter;
        if (id.mId != gNullObjectId.mId) {
            const GameObject* const object = entity->GetObject(id);
            if (object != nullptr) {
                AudioEmitterCom* const com = object->GetCom<AudioEmitterCom>();
                if (com != nullptr) {
                    emitter = com;
                } else {
                    // The report is compiled out; its arguments remain.
                    static_cast<void>(object->GetSafeName());
                }
            }
        }
    }

    PlayArgs::ParameterList parameters;
    if (mPlaySound->mParams.size() != 0) {
        GetInitialParams(sound.Str(), mPlaySound->mParams, parameters, entity);
    }
    PlayArgs soundArgs;
    PlayMusicArgs musicArgs;
    DialogPlayArgs dialogArgs;
    node = mPlaySound;
    PlayArgs* const args = node->mIsMasterMusic ? &musicArgs : (node->mIsDialog ? &dialogArgs : &soundArgs);
    args->mName = sound;
    if (!parameters.empty()) {
        args->mParameters = &parameters;
    }
    args->mStartMuted = node->mStartMuted;
    if (node->mVolume.mEnable) {
        args->mHasInitialGain = true;
        args->mInitialGainDb = node->mVolume.mValue.GetValue(entity);
        mVolumeDb = mPlaySound->mVolume.mValue.GetValue(entity);
        args->mInitialGainDb = mVolumeDb;
        node = mPlaySound;
        args->mInitialGainFadeSecs = node->mVolume.mSeconds;
        args->mInitialGainPostFade = node->mVolume.mOption;
    }

    unsigned int handle;
    if (node->mIsMasterMusic) {
        musicArgs.mOptions.mSync = kMaster;
        handle = emitter->PlayMusic(musicArgs);
    } else if (node->mIsDialog) {
        dialogArgs.mInterruptible = node->mIsInterruptible;
        handle = emitter->PlayDialog(dialogArgs);
    } else {
        handle = emitter->PlaySound(soundArgs);
    }
    mHandle = handle;
    if (handle == 0) {
        return;
    }

    node = mPlaySound;
    for (unsigned int i = 0; i < node->mParams.size(); ++i) {
        ParamState& state = mParamStates[i];
        const Symbol name = node->mParams[i].mName;
        AudioGenerator* const generator = theSoundManager.LockIfOwned(handle);
        if (generator == nullptr) {
            state.mValid = false;
            continue;
        }
        const bool valid = generator->GetParameter(name, state.mStartValue);
        --generator->mRefCount;
        state.mValid = valid;
        if (valid && !(0.0F >= node->mParams[i].mDuration)) {
            state.mElapsed = 0.0F;
            state.mTarget = node->mParams[i].mValue.GetValue(entity);
            state.mCurrent = state.mStartValue;
            state.mBlending = state.mTarget != state.mStartValue;
        }
    }

    StateGraphDriverCom* const driver = entity->GetRoot()->GetCom<StateGraphDriverCom>();
    if (!driver->AddSoundHandle(
            mHandle, mPlaySound->mInstanceName, static_cast<SoundHandleScope>(mPlaySound->mInstanceNameIsGlobal))) {
        // The report is compiled out; its arguments remain.
        static_cast<void>(mPlaySound->GetName());
        AudioGenerator* const generator = theSoundManager.LockIfOwned(mHandle);
        if (generator != nullptr) {
            generator->Stop();
            --generator->mRefCount;
        }
        mHandle = 0;
    }
}

// Reconstructed from eboot.elf at 0x7896A0. A changed persistent driven
// volume fades to its new level.
bool StateNodePlaySound::Instance::_Poll() {
    if (mHandle == 0) {
        return false;
    }
    AudioGenerator* generator = theSoundManager.LockIfOwned(mHandle);
    if (generator == nullptr) {
        return false;
    }
    --generator->mRefCount;
    const StateNodePlaySound* node = mPlaySound;
    if (node->mParams.size() != 0) {
        PollParams(node->mParams, mParamStates, mHandle, mGraph);
        node = mPlaySound;
    }
    if (node->mVolume.mEnable && node->mVolume.mPersistent && node->mVolume.mValue.mDriven) {
        const float db = node->mVolume.mValue.GetValue(_GetContextEntity());
        if (db != mVolumeDb) {
            mVolumeDb = db;
            const float seconds = mPlaySound->mVolume.mSeconds;
            const int option = mPlaySound->mVolume.mOption;
            generator = theSoundManager.LockIfOwned(mHandle);
            if (generator != nullptr) {
                generator->SetGain(
                    powf(10.0F, db * 0.05F), seconds, static_cast<AudioGenerator::PostFadeOption>(option));
                --generator->mRefCount;
            }
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x789B70.
Symbol StateNodePlaySound::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x789B80.
Symbol StateNodePlaySound::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x789B90.
int StateNodePlaySound::CurrentRev() const {
    return const_cast<StateNodePlaySound*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x789BB0.
bool StateNodePlaySound::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x789BE0.
Component* StateNodePlaySound::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x789BF0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateNodePlaySound::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodePlaySound*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodePlaySound(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x789E10.
PropRegistry& StateNodePlaySound::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x789E20.
ComMetaData& StateNodePlaySound::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x789E30.
StateNodeInstance* StateNodePlaySound::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x700660. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodePlaySound::_Create() {
    return new StateNodePlaySound();
}

// Reconstructed from eboot.elf at 0x6FB760. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodePlaySound::Init() {
    InitComponentClass<StateNodePlaySound>();
}
