// stategraph/StateNodeModifySound.o (0x780B20 to 0x785086). The class's
// registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/nodes/StateNodeModifySound.h"

#include <math.h>
#include <new>

#include "audio/core/generators/AudioGenerator.h"
#include "audio/core/system/SoundManager.h"
#include "entity/core/Entity.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "stategraph/conditions/StateConditionTimeElapsed.h"
#include "stategraph/drivers/StateGraphDriverCom.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "utl/time/TimeMgr.h"

// The object's statics, in the order of its static initializer (0x784FD0).
// The id of no object it first sets (0x1AB8CA0) comes from a shared header
// and is not used here.
Symbol StateNodeModifySound::sId("StateNodeModifySound");
Symbol StateNodeModifySound::sClassName("StateNodeModifySound");
PropRegistry StateNodeModifySound::sPropRegistry;
ComMetaData StateNodeModifySound::sMetaData;

// Reconstructed from eboot.elf at 0x780B20.
StateNodeModifySound::StateNodeModifySound()
    : mInstanceName(), mInstanceNameIsGlobal(false), mAction(kActionNone), mParameter(), mMute(), mVolume() {}

// Reconstructed from eboot.elf at 0x780BB0. The tag functions are lambdas
// (0x783900 to 0x784870): the properties are read-only without an instance
// name or while their struct is disabled, and the parameter's units and
// curve are hidden for an immediate change. Revision 1 made the values
// driven; an older one holds plain floats.
void StateNodeModifySound::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeModifySound prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    SymbolMetadata& instanceName = TypeSpecificMetadata(
        registry.RegisterProp(
            "instance_name", offsetof(StateNodeModifySound, mInstanceName), kPropertySymbol, sizeof(Symbol)),
        prototype.mInstanceName);
    instanceName.mDefault = prototype.mInstanceName;
    static_cast<void>(Symbol("prop"));
    instanceName.mHelp = String("The name by which this particular instance of the sound is interacted with.");
    // At 0x783650.
    instanceName.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<StateBase*>(args.mComponent)->_SendDisplayTextUpdated();
    };

    BoolMetadata& isGlobal = TypeSpecificMetadata(
        registry.RegisterProp(
            "instance_name_is_global", offsetof(StateNodeModifySound, mInstanceNameIsGlobal), kPropertyBool, 1),
        prototype.mInstanceNameIsGlobal);
    isGlobal.mDefault = prototype.mInstanceNameIsGlobal;
    static_cast<void>(Symbol("prop"));
    isGlobal.mHelp = String(
        "If true, the \"instance name\" is searched for on the list of \"global\" sounds. Otherwise the local list of "
        "sounds is searched.");
    // At 0x783900.
    isGlobal.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const StateNodeModifySound&>(args.mComponent).mInstanceName == Symbol();
    });

    IntMetadata<int>& action = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("action", offsetof(StateNodeModifySound, mAction), kPropertyInt, 4).mMetadata.Get());
    action.mDefault = prototype.mAction;
    static_cast<void>(Symbol("prop"));
    action.mHelp = String("Action (pause, continue, stop) to perform on the sound.");
    action.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{kActionNone, String("Do nothing"), String("Don't pause/continue/stop the sound.")},
        AllowedValue<int>{kActionPause, String("Pause"), String("Pause the sound if it's running.")},
        AllowedValue<int>{kActionContinue, String("Continue"), String("Continue the sound if it's paused.")},
        AllowedValue<int>{kActionStop, String("Stop"), String("Stop the sound if it's running.")}};
    // At 0x783A10.
    action.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const StateNodeModifySound&>(args.mComponent).mInstanceName == Symbol();
    });

    GroupMetadata& parameter = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "parameter", offsetof(StateNodeModifySound, mParameter), kPropertyStruct, sizeof(ParamSettings))
                                                                 .mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    PropRegistry& parameterRegistry = parameter.mRegistry;
    ParamSettings parameterType;
    // At 0x783B20.
    parameter.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const StateNodeModifySound&>(args.mComponent).mInstanceName == Symbol();
    });
    parameter.mHelp = String("Parameter to set on the sound (if it's still valid).");

    BoolMetadata& parameterEnable = TypeSpecificMetadata(
        parameterRegistry.RegisterProp("enable", offsetof(ParamSettings, mEnable), kPropertyBool, 1),
        parameterType.mEnable);
    parameterEnable.mDefault = parameterType.mEnable;
    static_cast<void>(Symbol("prop"));
    parameterEnable.mHelp = String("Enable or disable setting of this parameter.");

    SymbolMetadata& parameterName = TypeSpecificMetadata(
        parameterRegistry.RegisterProp("name", offsetof(ParamSettings, mName), kPropertySymbol, sizeof(Symbol)),
        parameterType.mName);
    parameterName.mDefault = parameterType.mName;
    static_cast<void>(Symbol("prop"));
    parameterName.mHelp = String("Name of the parameter.");
    // At 0x783C30.
    parameterName.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeModifySound&>(args.mComponent).mParameter.mEnable;
    });

    GroupMetadata& parameterValue = parameterType.mValue.Register(
        parameterRegistry, Symbol("value"), offsetof(ParamSettings, mValue), StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    parameterValue.mHelp = String("Value of the parameter.");
    // At 0x783D30.
    parameterValue.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeModifySound&>(args.mComponent).mParameter.mEnable;
    });

    BoolMetadata& persistent = TypeSpecificMetadata(
        parameterRegistry.RegisterProp("persistent", offsetof(ParamSettings, mPersistent), kPropertyBool, 1),
        parameterType.mPersistent);
    persistent.mDefault = parameterType.mPersistent;
    static_cast<void>(Symbol("prop"));
    persistent.mHelp = String("If checked, this node will continually enforce the specified value each frame.");
    // At 0x783E30.
    persistent.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeModifySound&>(args.mComponent).mParameter.mEnable;
    });

    FloatMetadata& duration = TypeSpecificMetadata(
        parameterRegistry.RegisterProp("duration", offsetof(ParamSettings, mDuration), kPropertyFloat, 4),
        parameterType.mDuration);
    duration.mDefault = parameterType.mDuration;
    static_cast<void>(Symbol("prop"));
    duration.mHelp = String("How long it should take to get from the current value to the new value.");
    // At 0x783F30.
    duration.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeModifySound&>(args.mComponent).mParameter.mEnable;
    });
    duration.mMin = 0.0F;

    IntMetadata<int>& units = static_cast<IntMetadata<int>&>(
        *parameterRegistry.RegisterProp("units", offsetof(ParamSettings, mUnits), kPropertyInt, 4).mMetadata.Get());
    units.mDefault = parameterType.mUnits;
    static_cast<void>(Symbol("prop"));
    units.mHelp = String("The time units used to measure the duration");
    // At 0x784030.
    units.mIsHidden._Set([](const TagArgs& args) {
        const ParamSettings& settings = static_cast<const StateNodeModifySound&>(args.mComponent).mParameter;
        return !settings.mEnable || 0.0F >= settings.mDuration;
    });
    units.mAllowedValues.mMode = TagBase<eastl::vector<AllowedValue<int>>>::kModeFunction;
    units.mAllowedValues.mFunction = gTimeUnitsAllowedValues;

    FloatWaveformMetadata& blendCurve = static_cast<FloatWaveformMetadata&>(TypeSpecificMetadata(
        parameterRegistry.RegisterProp(
            "blend_curve", offsetof(ParamSettings, mBlendCurve), kPropertyWaveformFloat, sizeof(Waveform<float>)),
        parameterType.mBlendCurve));
    blendCurve.mDefault = Waveform<float>(parameterType.mBlendCurve);
    static_cast<void>(Symbol("prop"));
    blendCurve.mHelp = String("A curve defining the blend ratio over time");
    // At 0x784140.
    blendCurve.mIsHidden._Set([](const TagArgs& args) {
        const ParamSettings& settings = static_cast<const StateNodeModifySound&>(args.mComponent).mParameter;
        return !settings.mEnable || 0.0F >= settings.mDuration;
    });
    blendCurve.mMin = 0.0F;
    blendCurve.mMax = 1.0F;
    blendCurve.mAllowedTypes = {2};

    GroupMetadata& mute = static_cast<GroupMetadata&>(
        *registry.RegisterProp("mute", offsetof(StateNodeModifySound, mMute), kPropertyStruct, sizeof(MuteSettings))
             .mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    MuteSettings muteType;
    PropRegistry& muteRegistry = mute.mRegistry;
    // At 0x784250.
    mute.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const StateNodeModifySound&>(args.mComponent).mInstanceName == Symbol();
    });
    mute.mHelp = String("Mute or unmute the sound so it cannot be heard; NOTE: that this is different from pausing!");

    BoolMetadata& muteEnable = TypeSpecificMetadata(
        muteRegistry.RegisterProp("enable", offsetof(MuteSettings, mEnable), kPropertyBool, 1), muteType.mEnable);
    muteEnable.mDefault = muteType.mEnable;
    static_cast<void>(Symbol("prop"));
    muteEnable.mHelp = String("Enable or disable setting of mute.");

    BoolMetadata& muteSet =
        TypeSpecificMetadata(muteRegistry.RegisterProp("set", offsetof(MuteSettings, mSet), kPropertyBool, 1),
                             muteType.mSet);
    muteSet.mDefault = muteType.mSet;
    static_cast<void>(Symbol("prop"));
    muteSet.mHelp = String("Mute or unmute the sound.");
    // At 0x784360.
    muteSet.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeModifySound&>(args.mComponent).mMute.mEnable;
    });

    GroupMetadata& volume = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "volume", offsetof(StateNodeModifySound, mVolume), kPropertyStruct, sizeof(VolumeSettings))
                                                              .mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    PropRegistry& volumeRegistry = volume.mRegistry;
    VolumeSettings volumeType;
    // At 0x784460.
    volume.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const StateNodeModifySound&>(args.mComponent).mInstanceName == Symbol();
    });
    volume.mHelp = String("Volume to set on the sound (if it's still valid).");

    BoolMetadata& volumeEnable = TypeSpecificMetadata(
        volumeRegistry.RegisterProp("enable", offsetof(VolumeSettings, mEnable), kPropertyBool, 1), volumeType.mEnable);
    volumeEnable.mDefault = volumeType.mEnable;
    static_cast<void>(Symbol("prop"));
    volumeEnable.mHelp = String("Enable or disable setting of volume.");

    GroupMetadata& volumeValue = volumeType.mValue.Register(
        volumeRegistry, Symbol("value"), offsetof(VolumeSettings, mValue), StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    volumeValue.mHelp = String("New volume if dbFS (-96.0 == off, 0.0 == normal).");
    // At 0x784570.
    volumeValue.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeModifySound&>(args.mComponent).mVolume.mEnable;
    });

    FloatMetadata& seconds = TypeSpecificMetadata(
        volumeRegistry.RegisterProp("seconds", offsetof(VolumeSettings, mSeconds), kPropertyFloat, 4),
        volumeType.mSeconds);
    seconds.mDefault = volumeType.mSeconds;
    static_cast<void>(Symbol("prop"));
    seconds.mHelp = String("How long it should take to get from the current volume to the new volume.");
    // At 0x784670.
    seconds.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeModifySound&>(args.mComponent).mVolume.mEnable;
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
    // At 0x784770.
    option.mReadOnly._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeModifySound&>(args.mComponent).mVolume.mEnable;
    });

    // At 0x784870: the values saved before revision 1 were plain floats.
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev > 0) {
                return;
            }
            StateNodeModifySound* const node = static_cast<StateNodeModifySound*>(args.mComponent);
            PropPath::Node parameterNode;
            PropPath::Node valueNode;
            const float parameterValue = GetPropValue<float>(
                *args.mSavedProps,
                PropPath(parameterNode = Symbol("parameter"), valueNode = Symbol("value")));
            node->mParameter.mValue = DrivenProp<float>(parameterValue);
            PropPath::Node volumeNode;
            const float volumeValue = GetPropValue<float>(
                *args.mSavedProps, PropPath(volumeNode = Symbol("volume"), valueNode = Symbol("value")));
            node->mVolume.mValue = DrivenProp<float>(volumeValue);
        };
    if (&registry == &own) {
        own.SetCurrentRev(1);
        own.mRevisionFunc = onRevision;
    }

    StateNode::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "A state that modifies a currently playing sound.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("m");
}

// Reconstructed from eboot.elf at 0x782CF0.
const char* StateNodeModifySound::GetEditorText() const {
    return mInstanceName.Str();
}

// Reconstructed from eboot.elf at 0x782D00. The parameter is disabled and
// set at once; a duration blends it in seconds.
StateNodeModifySound::ParamSettings::ParamSettings()
    : mEnable(false),
      mName(""),
      mValue(),
      mPersistent(false),
      mDuration(0.0F),
      mUnits(kTimeUnitsSeconds),
      mBlendCurve() {}

// Reconstructed from eboot.elf at 0x782E10.
StateNodeModifySound::Instance::Instance(const StateNodeModifySound& node, StateGraphInstance& graph)
    : StateNodeInstance(node, graph),
      mModifySound(&node),
      mHandle(0),
      mBlending(false),
      mPersistent(false),
      mStartValue(0.0F),
      mElapsed(0.0F) {}

// Reconstructed from eboot.elf at 0x782E50. A parameter with a duration
// blends from its current value in _Poll; a persistent one is set there
// every frame; any other is set here. The volume is given in dB. Stopping
// the sound forgets its name in the local table.
void StateNodeModifySound::Instance::_Enter() {
    mHandle = 0;
    mBlending = false;
    mPersistent = false;
    const StateNodeModifySound* node = mModifySound;
    if (node->mInstanceName == Symbol()) {
        return;
    }
    StateGraphDriverCom* const driver = mGraph->mEntity->GetRoot()->GetCom<StateGraphDriverCom>();
    if (!driver->HasSoundHandle(
            node->mInstanceName, mHandle, static_cast<SoundHandleScope>(node->mInstanceNameIsGlobal))) {
        return;
    }

    node = mModifySound;
    if (node->mParameter.mEnable && node->mParameter.mName != Symbol()) {
        if (!(0.0F >= node->mParameter.mDuration)) {
            mElapsed = 0.0F;
            const Symbol name = node->mParameter.mName;
            AudioGenerator* const generator = theSoundManager.LockIfOwned(mHandle);
            bool found = false;
            if (generator != nullptr) {
                found = generator->GetParameter(name, mStartValue);
                --generator->mRefCount;
            }
            mBlending = found;
            node = mModifySound;
        }
        if (node->mParameter.mPersistent) {
            mPersistent = true;
        }
        if (!mBlending && !mPersistent) {
            const Symbol name = node->mParameter.mName;
            const unsigned int handle = mHandle;
            const float value = node->mParameter.mValue.GetValue(_GetContextEntity());
            AudioGenerator* const generator = theSoundManager.LockIfOwned(handle);
            if (generator != nullptr) {
                generator->SetParameter(name, value);
                --generator->mRefCount;
            }
        }
    }

    if (mModifySound->mMute.mEnable) {
        const bool mute = mModifySound->mMute.mSet;
        AudioGenerator* const generator = theSoundManager.LockIfOwned(mHandle);
        if (generator != nullptr) {
            generator->SetMute(mute, false);
            --generator->mRefCount;
        }
    }

    node = mModifySound;
    if (node->mVolume.mEnable) {
        const unsigned int handle = mHandle;
        const float db = node->mVolume.mValue.GetValue(_GetContextEntity());
        const float seconds = mModifySound->mVolume.mSeconds;
        const int option = mModifySound->mVolume.mOption;
        AudioGenerator* const generator = theSoundManager.LockIfOwned(handle);
        if (generator != nullptr) {
            generator->SetGain(powf(10.0F, db * 0.05F), seconds, static_cast<AudioGenerator::PostFadeOption>(option));
            --generator->mRefCount;
        }
    }

    switch (mModifySound->mAction) {
    case kActionPause: {
        AudioGenerator* const generator = theSoundManager.LockIfOwned(mHandle);
        if (generator != nullptr) {
            generator->Pause();
            --generator->mRefCount;
        }
        break;
    }
    case kActionContinue: {
        AudioGenerator* const generator = theSoundManager.LockIfOwned(mHandle);
        if (generator != nullptr) {
            generator->Continue();
            --generator->mRefCount;
        }
        break;
    }
    case kActionStop: {
        AudioGenerator* const generator = theSoundManager.LockIfOwned(mHandle);
        if (generator != nullptr) {
            generator->Stop();
            --generator->mRefCount;
        }
        driver->RemoveSoundHandle(mHandle, kSoundHandleLocal);
        break;
    }
    default:
        break;
    }
}

// Reconstructed from eboot.elf at 0x783180. The blend moves from the start
// value to the driven value along the curve at the clamped fraction of the
// duration.
bool StateNodeModifySound::Instance::_Poll() {
    if (mBlending) {
        const int units = mGraph->ResolveTimeUnits(mModifySound->mParameter.mUnits);
        mElapsed += mGraph->mClock->DeltaTime(static_cast<TimeUnits>(units));
        const float fraction = mElapsed / mModifySound->mParameter.mDuration;
        const float clamped = 1.0F < fraction ? 1.0F : (0.0F > fraction ? 0.0F : fraction);
        const float blend = mModifySound->mParameter.mBlendCurve.Evaluate(clamped);
        const float target = mModifySound->mParameter.mValue.GetValue(_GetContextEntity());
        const float start = mStartValue;
        const Symbol name = mModifySound->mParameter.mName;
        AudioGenerator* const generator = theSoundManager.LockIfOwned(mHandle);
        if (generator != nullptr) {
            generator->SetParameter(name, (target - start) * blend + start);
            --generator->mRefCount;
        }
        mBlending = mElapsed < mModifySound->mParameter.mDuration;
        return mBlending;
    }
    if (mPersistent) {
        const StateNodeModifySound* const node = mModifySound;
        const unsigned int handle = mHandle;
        const Symbol name = node->mParameter.mName;
        const float value = node->mParameter.mValue.GetValue(_GetContextEntity());
        AudioGenerator* const generator = theSoundManager.LockIfOwned(handle);
        if (generator != nullptr) {
            generator->SetParameter(name, value);
            --generator->mRefCount;
        }
        return true;
    }
    return false;
}

// Reconstructed from eboot.elf at 0x7833F0.
Symbol StateNodeModifySound::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x783400.
Symbol StateNodeModifySound::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x783410.
int StateNodeModifySound::CurrentRev() const {
    return const_cast<StateNodeModifySound*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x783430.
bool StateNodeModifySound::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x783460.
Component* StateNodeModifySound::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x783470. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateNodeModifySound::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodeModifySound*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeModifySound(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x783580.
PropRegistry& StateNodeModifySound::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x783590.
ComMetaData& StateNodeModifySound::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x7835A0.
StateNodeInstance* StateNodeModifySound::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x783660.
StateNodeModifySound::StateNodeModifySound(const StateNodeModifySound& other)
    : StateNode(other),
      mInstanceName(other.mInstanceName),
      mInstanceNameIsGlobal(other.mInstanceNameIsGlobal),
      mAction(other.mAction),
      mParameter(other.mParameter),
      mMute(other.mMute),
      mVolume(other.mVolume) {}

// Reconstructed from eboot.elf at 0x700610. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodeModifySound::_Create() {
    return new StateNodeModifySound();
}

// Reconstructed from eboot.elf at 0x6FB2B0. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodeModifySound::Init() {
    InitComponentClass<StateNodeModifySound>();
}
