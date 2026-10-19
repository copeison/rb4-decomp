// stategraph/StateNodeAnimate.o (0x70AD80 to 0x7178F7). The class's
// registration and factory are emitted in stategraph/StateGraph.o. The
// object also holds the copies of DrivenProp<GameObjectId>::Register
// (0x70D9F0), DrivenProp<ResourcePath>::Register (0x70E190), the three
// DrivenProp<T>::GetValue it uses (0x70EBF0 ResourcePath, 0x70F6C0
// GameObjectId, 0x70FB90 float), DrivenProp<float>::CopyFromProperty
// (0x716F10) and StateNode::_InitAsSuperclass (0x70E940).
#include "stategraph/nodes/StateNodeAnimate.h"

#include <cmath>
#include <cstring>
#include <new>

#include "anim/char/CharClipCom.h"
#include "anim/char/CharClipResource.h"
#include "anim/core/AnimResource.h"
#include "anim/drivers/AnimDriver.h"
#include "anim/drivers/AnimDriverCom.h"
#include "anim/props/PropAnimResource.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropUtl.h"
#include "entity/resources/ResourceMetaData.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "stategraph/transitions/StateTransition.h"
#include "utl/files/FileUtl.h"
#include "utl/text/MakeString.h"
#include "utl/time/TimeMgr.h"

namespace {

// The id of no object, at 0x1AB2F28, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The units the display names of the time properties give, by timeline
// (0x193B850). Name not in the reference map.
const char* const gTimelineUnits[] = {"seconds", "beat", "UI seconds", "tutorial seconds", "units"};

// The unit of the node's times: frames, or its timeline's. Inlined into
// the display-name tag functions. Name not in the reference map.
const char* TimeUnitLabel(const StateNodeAnimate& node) {
    if (node.mOptions.mUseFrameTime) {
        return "frame";
    }
    const unsigned int timeline = node.mOptions.mTimeline;
    return timeline < 5 ? gTimelineUnits[timeline] : "";
}

// The display name of a time property: the format with the unit. Inlined
// into the display-name tag functions. Name not in the reference map.
String TimeDisplayName(const char* format, const TagArgs& args) {
    FormatString name(format);
    name << TimeUnitLabel(static_cast<const StateNodeAnimate&>(args.mComponent));
    return String(name.Str());
}

// The property callbacks _Init installs (0x710CB0 to 0x710D40): a changed
// property redraws the node, and "loop_mode" reads as the resolved mode.
// Names not in the reference map.
// Reconstructed from eboot.elf at 0x710CB0.
void OnClipValueChanged(const PropAccessorArgs& args) {
    static_cast<StateNodeAnimate*>(args.mComponent)->_SendDisplayTextUpdated();
}

// Reconstructed from eboot.elf at 0x710CC0.
void OnClipPropChanged(const PropAccessorArgs& args) {
    static_cast<StateNodeAnimate*>(args.mComponent)->_SendDisplayTextUpdated();
}

// Reconstructed from eboot.elf at 0x710CD0.
void OnClipDrivenChanged(const PropAccessorArgs& args) {
    static_cast<StateNodeAnimate*>(args.mComponent)->_SendDisplayTextUpdated();
}

// Reconstructed from eboot.elf at 0x710CE0.
void OnAnimValueChanged(const PropAccessorArgs& args) {
    static_cast<StateNodeAnimate*>(args.mComponent)->_SendDisplayTextUpdated();
}

// Reconstructed from eboot.elf at 0x710CF0.
void OnAnimPropChanged(const PropAccessorArgs& args) {
    static_cast<StateNodeAnimate*>(args.mComponent)->_SendDisplayTextUpdated();
}

// Reconstructed from eboot.elf at 0x710D00.
void OnAnimDrivenChanged(const PropAccessorArgs& args) {
    static_cast<StateNodeAnimate*>(args.mComponent)->_SendDisplayTextUpdated();
}

// Reconstructed from eboot.elf at 0x710D10.
void OnTimelineChanged(const PropAccessorArgs& args) {
    static_cast<StateNodeAnimate*>(args.mComponent)->_SendDisplayTextUpdated();
}

// Reconstructed from eboot.elf at 0x710D20.
int GetLoopModeAccessor(const PropAccessorArgs& args) {
    return static_cast<StateNodeAnimate*>(args.mComponent)->GetLoopMode(nullptr);
}

// Reconstructed from eboot.elf at 0x710D40.
void OnLoopModeChanged(const PropAccessorArgs& args) {
    static_cast<StateNodeAnimate*>(args.mComponent)->_SendDisplayTextUpdated();
}

}  // namespace

// The object's statics, in the order of its static initializer (0x717840).
Symbol StateNodeAnimate::sId("StateNodeAnimate");
Symbol StateNodeAnimate::sClassName("StateNodeAnimate");
PropRegistry StateNodeAnimate::sPropRegistry;
ComMetaData StateNodeAnimate::sMetaData;

// Reconstructed from eboot.elf at 0x70AD80.
StateNodeAnimate::StateNodeAnimate()
    : mLoopMode(kLoopModeDefault),
      mAnimType(PropAnimResource::Id()),
      mClip(GameObjectId{0xFFFFFFFFU}),
      mAnim(),
      mOptions(),
      mCurrentTimeEnabled(false),
      mUseNormalizedTime(false),
      mTimeRangeBeginEnabled(false),
      mTimeRangeEndEnabled(false),
      mPeriodEnabled(false),
      mStartTimeEnabled(false),
      mStopOnExit(true),
      mCurrentTime(0.0F),
      mTimeRangeBegin(0.0F),
      mTimeRangeEnd(0.0F),
      mPeriod(1.0F),
      mStartTime(0.0F),
      mWeight(1.0F),
      mScale(1.0F),
      mCharAnim() {}

// Reconstructed from eboot.elf at 0x70AF90. "clip" applies to a
// CharClipResource and "anim" to the other types; "current_time" hides the
// range, period, scale, looping and start options it overrides. Revisions 1
// to 6 move older layouts of the period, weight, scale, ranges and
// animation into the driven properties.
void StateNodeAnimate::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeAnimate prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&own != &registry) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    SymbolMetadata& animType = TypeSpecificMetadata(
        registry.RegisterProp("anim_type", offsetof(StateNodeAnimate, mAnimType), kPropertySymbol, 8),
        prototype.mAnimType);
    animType.mDefault = prototype.mAnimType;
    static_cast<void>(Symbol("prop"));
    animType.mHelp = String("The type of animation this node plays.");
    animType.mAllowedValues = GetAnimTypeAllowedValues();

    // "clip": an object with a CharClipCom.
    GroupMetadata& clip = prototype.mClip.Register(
        registry, Symbol("clip"), offsetof(StateNodeAnimate, mClip), StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    clip.mHelp = String("The animation this node plays.");
    // At 0x711C30.
    clip.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeAnimate&>(args.mComponent).mAnimType != CharClipResource::Id();
    });
    GameObjectIdMetadata& clipValue = TypeSpecificMetadata(
        const_cast<PropInfo&>(*clip.mRegistry.FindProp(PropPath(Symbol("value")), nullptr)), prototype.mClip.mValue);
    static_cast<void>(Symbol("driven_prop_value"));
    clipValue.mRequiredComs = {CharClipCom::sId};
    clipValue.mEntity.mMode = TagBase<Entity*>::kModeFunction;
    clipValue.mEntity.mFunction = StateBase::_GetContextEntityTagFunc();
    clipValue.mOnChanged = OnClipValueChanged;
    PropRefMetadata& clipProp = TypeSpecificMetadata(
        const_cast<PropInfo&>(*clip.mRegistry.FindProp(PropPath(Symbol("prop")), nullptr)), prototype.mClip.mProp);
    static_cast<void>(Symbol("driven_prop_prop"));
    clipProp.mRefOnChanged = OnClipPropChanged;
    BoolMetadata& clipDriven = TypeSpecificMetadata(
        const_cast<PropInfo&>(*clip.mRegistry.FindProp(PropPath(Symbol("is_driven")), nullptr)),
        prototype.mClip.mDriven);
    static_cast<void>(Symbol("driven_prop_is_driven"));
    clipDriven.mOnChanged = OnClipDrivenChanged;

    // "anim": a resource of the class "anim_type" names.
    GroupMetadata& anim = prototype.mAnim.Register(
        registry, Symbol("anim"), offsetof(StateNodeAnimate, mAnim), StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    anim.mHelp = String("The animation this node plays.");
    // At 0x712C40.
    anim.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeAnimate&>(args.mComponent).mAnimType == CharClipResource::Id();
    });
    ResourcePathMetadata& animValue = TypeSpecificMetadata(
        const_cast<PropInfo&>(*anim.mRegistry.FindProp(PropPath(Symbol("value")), nullptr)), prototype.mAnim.mValue);
    static_cast<void>(Symbol("driven_prop_value"));
    // At 0x712DE0.
    animValue.mResourceId._Set(
        [](const TagArgs& args) { return static_cast<const StateNodeAnimate&>(args.mComponent).mAnimType; });
    animValue.mEntity.mMode = TagBase<Entity*>::kModeFunction;
    animValue.mEntity.mFunction = StateBase::_GetContextEntityTagFunc();
    animValue.mOnChanged = OnAnimValueChanged;
    PropRefMetadata& animProp = TypeSpecificMetadata(
        const_cast<PropInfo&>(*anim.mRegistry.FindProp(PropPath(Symbol("prop")), nullptr)), prototype.mAnim.mProp);
    static_cast<void>(Symbol("driven_prop_prop"));
    animProp.mRefOnChanged = OnAnimPropChanged;
    BoolMetadata& animDriven = TypeSpecificMetadata(
        const_cast<PropInfo&>(*anim.mRegistry.FindProp(PropPath(Symbol("is_driven")), nullptr)),
        prototype.mAnim.mDriven);
    static_cast<void>(Symbol("driven_prop_is_driven"));
    animDriven.mOnChanged = OnAnimDrivenChanged;

    BoolMetadata& useFrameTime = TypeSpecificMetadata(
        registry.RegisterProp(
            "use_frame_time",
            offsetof(StateNodeAnimate, mOptions) + offsetof(AnimOptions, mUseFrameTime),
            kPropertyBool,
            1),
        prototype.mOptions.mUseFrameTime);
    useFrameTime.mDefault = prototype.mOptions.mUseFrameTime;
    static_cast<void>(Symbol("prop"));
    useFrameTime.mHelp = String(
        "If true, time_range_begin, time_range_end, and start_time will be specified in frames (and will be "
        "renamed as such). If false, these values will be specified in the chosen time units.");

    IntMetadata<int>& timeline = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "timeline", offsetof(StateNodeAnimate, mOptions) + offsetof(AnimOptions, mTimeline), kPropertyInt, 4)
                                                                        .mMetadata.Get());
    timeline.mDefault = prototype.mOptions.mTimeline;
    static_cast<void>(Symbol("prop"));
    timeline.mHelp = String("The timeline to use when playing the animation.");
    // At 0x712EE0: "Defer to anim" (4), with the animation's own timeline,
    // then the timelines.
    timeline.mAllowedValues._Set([](const TagArgs& args) {
        const eastl::vector<AllowedValue<int>>& units = GetTimeUnitsAllowedValues();
        eastl::vector<AllowedValue<int>> values;
        values.reserve(units.size() + 1);
        const AnimResource* const resource =
            static_cast<const StateNodeAnimate&>(args.mComponent).GetAnimResource();
        const char* label;
        if (resource != nullptr) {
            FormatString text("Defer to anim (%s)");
            text << GetTimeUnitsName(resource->GetUnits(), true);
            label = text.Str();
        } else {
            label = "Defer to anim";
        }
        values.push_back(AllowedValue<int>{
            kNumTimeUnits, String(label), String("Use the native timeline specified by the animation itself.")});
        // The binary inserts the range at once (0x32BF20).
        for (const AllowedValue<int>& value : units) {
            values.push_back(value);
        }
        return values;
    });
    // At 0x713170.
    timeline.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeAnimate&>(args.mComponent).mOptions.mUseFrameTime;
    });
    timeline.mOnChanged = OnTimelineChanged;

    GroupMetadata& currentTime = static_cast<GroupMetadata&>(
        *registry.RegisterProp("current_time", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    currentTime.mKind = 1;
    currentTime.mHelp = String(
        "Optionally force a particular frame of the animation to be applied once per frame. Note that in this "
        "case, almost all other options are irrelevant and will be hidden. This is similar to a SetAnimFrame "
        "node, except that when blending between Animate nodes we blend the animation content instead of "
        "interpolating the frame to be applied as we do in SetAnimFrame.");
    // At 0x713270.
    currentTime.mDisplayName._Set([](const TagArgs& args) { return TimeDisplayName("current %s", args); });
    BoolMetadata& currentTimeEnabled = TypeSpecificMetadata(
        currentTime.mRegistry.RegisterProp(
            "enabled", offsetof(StateNodeAnimate, mCurrentTimeEnabled), kPropertyBool, 1),
        prototype.mCurrentTimeEnabled);
    currentTimeEnabled.mDefault = prototype.mCurrentTimeEnabled;
    GroupMetadata& currentTimeValue = prototype.mCurrentTime.Register(
        currentTime.mRegistry,
        Symbol("time"),
        offsetof(StateNodeAnimate, mCurrentTime),
        StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    // At 0x713420.
    currentTimeValue.mIsHidden._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeAnimate&>(args.mComponent).mCurrentTimeEnabled;
    });

    BoolMetadata& useNormalizedTime = TypeSpecificMetadata(
        registry.RegisterProp(
            "use_normalized_time", offsetof(StateNodeAnimate, mUseNormalizedTime), kPropertyBool, 1),
        prototype.mUseNormalizedTime);
    useNormalizedTime.mDefault = prototype.mUseNormalizedTime;
    static_cast<void>(Symbol("prop"));
    useNormalizedTime.mHelp =
        String("If true, use 0 to 1 values to progress through entire animation regardless of its length");
    // At 0x713520.
    useNormalizedTime.mIsHidden._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeAnimate&>(args.mComponent).mCurrentTimeEnabled;
    });

    GroupMetadata& rangeBegin = static_cast<GroupMetadata&>(
        *registry.RegisterProp("time_range_begin", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    rangeBegin.mKind = 1;
    rangeBegin.mHelp = String(
        "Optionally specify the beginning of the frame range to animate within; if not specified, the first "
        "frame of the AnimResource itself is used.");
    // At 0x713620.
    rangeBegin.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeAnimate&>(args.mComponent).mCurrentTimeEnabled;
    });
    // At 0x713720.
    rangeBegin.mDisplayName._Set([](const TagArgs& args) { return TimeDisplayName("%s range begin", args); });
    BoolMetadata& rangeBeginEnabled = TypeSpecificMetadata(
        rangeBegin.mRegistry.RegisterProp(
            "enabled", offsetof(StateNodeAnimate, mTimeRangeBeginEnabled), kPropertyBool, 1),
        prototype.mTimeRangeBeginEnabled);
    rangeBeginEnabled.mDefault = prototype.mTimeRangeBeginEnabled;
    GroupMetadata& rangeBeginValue = prototype.mTimeRangeBegin.Register(
        rangeBegin.mRegistry,
        Symbol("time"),
        offsetof(StateNodeAnimate, mTimeRangeBegin),
        StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    // At 0x7138D0.
    rangeBeginValue.mIsHidden._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeAnimate&>(args.mComponent).mTimeRangeBeginEnabled;
    });

    GroupMetadata& rangeEnd = static_cast<GroupMetadata&>(
        *registry.RegisterProp("time_range_end", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    rangeEnd.mKind = 1;
    rangeEnd.mHelp = String(
        "Optionally specify the frame to animate to; if not specified, the last frame of the AnimResource "
        "itself is used.");
    // At 0x7139D0.
    rangeEnd.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeAnimate&>(args.mComponent).mCurrentTimeEnabled;
    });
    // At 0x713AD0.
    rangeEnd.mDisplayName._Set([](const TagArgs& args) { return TimeDisplayName("%s range end", args); });
    BoolMetadata& rangeEndEnabled = TypeSpecificMetadata(
        rangeEnd.mRegistry.RegisterProp(
            "enabled", offsetof(StateNodeAnimate, mTimeRangeEndEnabled), kPropertyBool, 1),
        prototype.mTimeRangeEndEnabled);
    rangeEndEnabled.mDefault = prototype.mTimeRangeEndEnabled;
    GroupMetadata& rangeEndValue = prototype.mTimeRangeEnd.Register(
        rangeEnd.mRegistry,
        Symbol("time"),
        offsetof(StateNodeAnimate, mTimeRangeEnd),
        StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    // At 0x713C80.
    rangeEndValue.mIsHidden._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeAnimate&>(args.mComponent).mTimeRangeEndEnabled;
    });

    GroupMetadata& period = static_cast<GroupMetadata&>(
        *registry.RegisterProp("period", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    period.mKind = 1;
    period.mHelp = String(
        "Optionally specify a period of time over which this animation will play (assuming scale = 1.0). To "
        "toggle between specifying the period as a value and as a property reference, click '>'.");
    // At 0x713D80.
    period.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeAnimate&>(args.mComponent).mCurrentTimeEnabled;
    });
    BoolMetadata& periodEnabled = TypeSpecificMetadata(
        period.mRegistry.RegisterProp("enabled", offsetof(StateNodeAnimate, mPeriodEnabled), kPropertyBool, 1),
        prototype.mPeriodEnabled);
    periodEnabled.mDefault = prototype.mPeriodEnabled;
    GroupMetadata& periodValue = prototype.mPeriod.Register(
        period.mRegistry, Symbol("period"), offsetof(StateNodeAnimate, mPeriod), StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    // At 0x713E80.
    periodValue.mIsHidden._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeAnimate&>(args.mComponent).mPeriodEnabled;
    });

    GroupMetadata& weight = prototype.mWeight.Register(
        registry, Symbol("weight"), offsetof(StateNodeAnimate, mWeight), StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    weight.mHelp = String("The weight to use when playing the animation.");

    GroupMetadata& scale = prototype.mScale.Register(
        registry, Symbol("scale"), offsetof(StateNodeAnimate, mScale), StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    scale.mHelp =
        String("The scale to use when playing the animation. Use negative numbers to play the animation backwards.");
    // At 0x713F80.
    scale.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeAnimate&>(args.mComponent).mCurrentTimeEnabled;
    });

    IntMetadata<int>& loopMode = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("loop_mode", offsetof(StateNodeAnimate, mLoopMode), kPropertyInt, 4).mMetadata.Get());
    loopMode.mDefault = prototype.mLoopMode;
    static_cast<void>(Symbol("prop"));
    loopMode.mHelp = String(
        "Controls whether an animation loops or not. \"Default\" means the looping state authored directly into "
        "the animation is used. \"Loop\" and \"Do not loop\" override this default behavior.\n\nIf any "
        "transitions out of this node are animation style blends that require the animation to loop in order "
        "to work properly, looping will be forced.");
    // At 0x714080.
    loopMode.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeAnimate&>(args.mComponent).mCurrentTimeEnabled;
    });
    // At 0x714180: read-only while a transition forces looping.
    loopMode.mReadOnly._Set([](const TagArgs& args) {
        bool forced = false;
        static_cast<const StateNodeAnimate&>(args.mComponent).GetLoopMode(&forced);
        return forced;
    });
    loopMode.mGetter = GetLoopModeAccessor;
    // At 0x7142C0: "Defer to anim", with the animation's own looping, then
    // the overrides.
    loopMode.mAllowedValues._Set([](const TagArgs& args) {
        eastl::vector<AllowedValue<int>> values;
        values.reserve(3);
        const AnimResource* const resource =
            static_cast<const StateNodeAnimate&>(args.mComponent).GetAnimResource();
        const char* label;
        if (resource != nullptr) {
            FormatString text("Defer to anim (%s)");
            text << (resource->IsLooping() ? "Loop" : "Do not loop");
            label = text.Str();
        } else {
            label = "Defer to anim";
        }
        values.push_back(AllowedValue<int>{
            kLoopModeDefault, String(label), String("Uses the looping mode specified by the the animation itself.")});
        values.push_back(AllowedValue<int>{kLoopModeLoop, String("Loop"), String("The animation will loop.")});
        values.push_back(
            AllowedValue<int>{kLoopModeDoNotLoop, String("Do not loop"), String("The animation will not loop.")});
        return values;
    });
    loopMode.mOnChanged = OnLoopModeChanged;

    BoolMetadata& startFromCurrent = TypeSpecificMetadata(
        registry.RegisterProp(
            "start_from_current_frame",
            offsetof(StateNodeAnimate, mOptions) + offsetof(AnimOptions, mStartFromCurrentFrame),
            kPropertyBool,
            1),
        prototype.mOptions.mStartFromCurrentFrame);
    startFromCurrent.mDefault = prototype.mOptions.mStartFromCurrentFrame;
    static_cast<void>(Symbol("prop"));
    startFromCurrent.mHelp = String("If checked, this animation will begin playing from its currently active frame.");
    // At 0x714650.
    startFromCurrent.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeAnimate&>(args.mComponent).mCurrentTimeEnabled;
    });
    // At 0x714750.
    startFromCurrent.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const StateNodeAnimate&>(args.mComponent).mStartTimeEnabled;
    });

    GroupMetadata& startTime = static_cast<GroupMetadata&>(
        *registry.RegisterProp("start_time", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    startTime.mKind = 1;
    startTime.mHelp = String(
        "Optionally specify the frame to animate from; if not specified, then the beginning of the frame range "
        "is used. Note that these are distinct concepts; for example you might loop over the entire specified "
        "range, but start from a midpoint which is specified by this property.");
    // At 0x714850.
    startTime.mDisplayName._Set([](const TagArgs& args) { return TimeDisplayName("start %s", args); });
    // At 0x714A00.
    startTime.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateNodeAnimate&>(args.mComponent).mCurrentTimeEnabled;
    });
    // At 0x714B00.
    startTime.mReadOnly._Set([](const TagArgs& args) {
        return static_cast<const StateNodeAnimate&>(args.mComponent).mOptions.mStartFromCurrentFrame;
    });
    BoolMetadata& startTimeEnabled = TypeSpecificMetadata(
        startTime.mRegistry.RegisterProp(
            "enabled", offsetof(StateNodeAnimate, mStartTimeEnabled), kPropertyBool, 1),
        prototype.mStartTimeEnabled);
    startTimeEnabled.mDefault = prototype.mStartTimeEnabled;
    GroupMetadata& startTimeValue = prototype.mStartTime.Register(
        startTime.mRegistry,
        Symbol("time"),
        offsetof(StateNodeAnimate, mStartTime),
        StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    // At 0x714C00.
    startTimeValue.mIsHidden._Set([](const TagArgs& args) {
        return !static_cast<const StateNodeAnimate&>(args.mComponent).mStartTimeEnabled;
    });

    SymbolMetadata& instanceName = TypeSpecificMetadata(
        registry.RegisterProp(
            "anim_instance_name",
            offsetof(StateNodeAnimate, mOptions) + offsetof(AnimOptions, mName),
            kPropertySymbol,
            8),
        prototype.mOptions.mName);
    instanceName.mDefault = prototype.mOptions.mName;
    static_cast<void>(Symbol("prop"));
    instanceName.mHelp = String(
        "An optional name by which this particular instance of the animation can be interacted with later on, "
        "by other animation nodes. If not set, this node will spawn a new instance of the animation even if "
        "other instances are already playing.");

    BoolMetadata& stopOnExit = TypeSpecificMetadata(
        registry.RegisterProp("stop_on_exit", offsetof(StateNodeAnimate, mStopOnExit), kPropertyBool, 1),
        prototype.mStopOnExit);
    stopOnExit.mDefault = prototype.mStopOnExit;
    static_cast<void>(Symbol("prop"));
    stopOnExit.mHelp = String(
        "If true, this animation will be stopped when this node exits. Otherwise, the animation will keep "
        "playing. Remember to name the instance if you want to modify it later with another Animate node.");

    // At 0x714D00.
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            StateNodeAnimate* const node = static_cast<StateNodeAnimate*>(args.mComponent);
            const Component& saved = *args.mSavedProps;
            PropPath::Node first;
            PropPath::Node second;
            // Before revision 2 the period was a plain value (as "frame",
            // then "period"), with its reference and flag beside it.
            if (args.mSavedRev <= 0) {
                node->mPeriod.mValue =
                    GetPropValue<float>(saved, PropPath(first = Symbol("period"), second = Symbol("frame")));
            } else if (args.mSavedRev == 1) {
                node->mPeriod.mValue =
                    GetPropValue<float>(saved, PropPath(first = Symbol("period"), second = Symbol("period")));
                node->mPeriod.mProp =
                    GetPropValue<PropRef>(saved, PropPath(first = Symbol("period"), second = Symbol("period_prop")));
                node->mPeriod.mDriven =
                    GetPropValue<bool>(saved, PropPath(first = Symbol("period"), second = Symbol("driven")));
            }
            // The weight, scale, ranges and start were plain values, then
            // driven properties under older names.
            if (args.mSavedRev <= 1) {
                node->mWeight.mValue = GetPropValue<float>(saved, PropPath(Symbol("weight")));
                node->mScale.mValue = GetPropValue<float>(saved, PropPath(Symbol("scale")));
                node->mTimeRangeBegin.mValue = GetPropValue<float>(
                    saved, PropPath(first = Symbol("frame_range_begin"), second = Symbol("frame")));
                node->mTimeRangeEnd.mValue = GetPropValue<float>(
                    saved, PropPath(first = Symbol("frame_range_end"), second = Symbol("frame")));
                node->mStartTime.mValue =
                    GetPropValue<float>(saved, PropPath(first = Symbol("start_frame"), second = Symbol("frame")));
            } else if (args.mSavedRev == 2) {
                node->mTimeRangeBegin.CopyFromProperty(
                    saved, PropPath(first = Symbol("frame_range_begin"), second = Symbol("frame")));
                node->mTimeRangeEnd.CopyFromProperty(
                    saved, PropPath(first = Symbol("frame_range_end"), second = Symbol("frame")));
                node->mStartTime.CopyFromProperty(
                    saved, PropPath(first = Symbol("start_frame"), second = Symbol("frame")));
            }
            // The times were frames before revision 3.
            if (args.mSavedRev <= 2) {
                node->mOptions.mUseFrameTime = true;
            }
            // Revision 4 added the type, taken from the animation's path.
            if (args.mSavedRev <= 3) {
                node->mAnimType =
                    ResourceMetaData::GetIdFromPath(GetPropValue<ResourcePath>(saved, PropPath(Symbol("anim"))));
            }
            if (args.mSavedRev <= 4 && strcmp(node->mAnimType.Str(), "CharBtClipResource") == 0) {
                node->mAnimType = CharClipResource::Id();
            }
            // Revision 6 made "clip" and "anim" driven properties.
            if (args.mSavedRev <= 5) {
                Component& com = const_cast<Component&>(saved);
                static Symbol clip;
                if (clip == Symbol()) {
                    clip = Symbol("clip");
                }
                {
                    const PropPath path(clip);
                    void* storage = &com;
                    const PropInfo* const info = com._GetPropRegistry().FindProp(path, &storage);
                    if (info != nullptr) {
                        _GetProp(com, path, *info, storage, &node->mClip.mValue, kPropertyGameObjectId);
                    }
                }
                static Symbol anim;
                if (anim == Symbol()) {
                    anim = Symbol("anim");
                }
                {
                    const PropPath path(clip);
                    void* storage = &com;
                    const PropInfo* const info = com._GetPropRegistry().FindProp(path, &storage);
                    if (info != nullptr) {
                        _GetProp(com, path, *info, storage, &node->mClip.mValue, kPropertyGameObjectId);
                    }
                }
                {
                    const PropPath path(anim);
                    void* storage = &com;
                    const PropInfo* const info = com._GetPropRegistry().FindProp(path, &storage);
                    if (info != nullptr) {
                        _GetProp(com, path, *info, storage, &node->mAnim.mValue, kPropertyResourcePath);
                    }
                }
            }
        };
    registry.SetCurrentRev(6);
    registry.mRevisionFunc = onRevision;

    StateNode::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "A state that plays an animation.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("a");
}

// Reconstructed from eboot.elf at 0x70EAB0. A CharAnimResource is loaded
// with the node's resources; the other types are looked up when needed.
bool StateNodeAnimate::_OnResourcesLoaded() {
    StateBase::_OnResourcesLoaded();
    if (mAnimType == CharAnimResource::Id()) {
        mCharAnim = Resource::GetOrLoad<CharAnimResource>(mAnim.GetValue(_GetContextEntityPrototype()), false);
    } else {
        mCharAnim = static_cast<CharAnimResource*>(nullptr);
    }
    return true;
}

// Reconstructed from eboot.elf at 0x70EEB0.
void StateNodeAnimate::SetAnim(ResourcePath path) {
    mAnim = DrivenProp<ResourcePath>(path);
    mAnimType = ResourceMetaData::GetIdFromPath(path);
    _SendDisplayTextUpdated();
}

// Reconstructed from eboot.elf at 0x70EFA0.
void StateNodeAnimate::SetOptions(const AnimOptions& options) {
    mOptions = options;
    mTimeRangeBeginEnabled = options.mTimeRangeBegin != AnimOptions::kInvalidTime;
    mTimeRangeEndEnabled = options.mTimeRangeEnd != AnimOptions::kInvalidTime;
    mPeriodEnabled = options.mPeriod != AnimOptions::kInvalidPeriod;
    mStartTimeEnabled = options.mStartTime != AnimOptions::kInvalidTime;
}

// Reconstructed from eboot.elf at 0x70F010.
const char* StateNodeAnimate::GetEditorText() const {
    int loopMode = GetLoopMode(nullptr);
    if (loopMode == kLoopModeDefault) {
        const AnimResource* const resource = GetAnimResource();
        if (resource != nullptr) {
            loopMode = resource->IsLooping() ? kLoopModeLoop : kLoopModeDoNotLoop;
        } else {
            loopMode = kLoopModeDoNotLoop;
        }
    }
    const char* const name = GetName();
    FormatString text("%s%s");
    text << name << (loopMode == kLoopModeLoop ? "\n(looping)" : "");
    return text.Str();
}

// Reconstructed from eboot.elf at 0x70F0F0. A CharAnimResource is the one
// _OnResourcesLoaded loaded; a PropAnimResource must be inlined into the
// context entity's resource; a clip is inlined or loaded.
AnimResource* StateNodeAnimate::GetAnimResource() const {
    if (mAnimType == CharAnimResource::Id()) {
        CharAnimResource* const resource = mCharAnim.Get();
        if (resource == nullptr || resource->Fail()) {
            return nullptr;
        }
        return resource;
    }
    Entity* const prototype = _GetContextEntityPrototype();
    const EntityResource* const entityResource = prototype->mResource;
    const ResourcePath path = GetAnimPath(prototype);
    if (path == ResourcePath()) {
        return nullptr;
    }
    if (mAnimType != PropAnimResource::Id()) {
        // The clip's class id is made but not used.
        static_cast<void>(CharClipResource::Id());
        ResourcePtr<CharClipResource> clip = entityResource->TryGetInline<CharClipResource>(path);
        if (clip == nullptr) {
            clip = Resource::GetOrLoad<CharClipResource>(path, false);
            if (clip == nullptr) {
                return nullptr;
            }
        }
        return clip->Fail() ? nullptr : clip.Get();
    }
    ResourcePtr<PropAnimResource> anim = entityResource->TryGetInline<PropAnimResource>(path);
    if (anim == nullptr || anim->Fail()) {
        return nullptr;
    }
    return anim.Get();
}

// Reconstructed from eboot.elf at 0x70F490.
bool StateNodeAnimate::IsValid() const {
    return !mCurrentTimeEnabled;
}

// Reconstructed from eboot.elf at 0x70F4B0.
const char* StateNodeAnimate::GetName() const {
    Entity* const prototype = _GetContextEntityPrototype();
    if (mAnimType == CharClipResource::Id()) {
        const GameObjectId clip = mClip.GetValue(prototype);
        if (clip.mId != gNullObjectId.mId) {
            const GameObject* const object = prototype->GetObject(clip);
            if (object != nullptr) {
                return object->GetSafeName();
            }
        }
        return "<None>";
    }
    const ResourcePath path = mAnim.GetValue(prototype);
    char base[512] = {};
    FormatString name("%s");
    name << FileGetBase(path.Str(), base);
    return name.Str();
}

// Reconstructed from eboot.elf at 0x70F990. A beat is a sixtieth of the
// rate.
float StateNodeAnimate::GetScale(int units, float rate) const {
    const AnimResource* const resource = GetAnimResource();
    if (resource == nullptr) {
        return 0.0F;
    }
    Entity* const prototype = _GetContextEntityPrototype();
    const float scale = mScale.GetValue(prototype);
    if (scale == 0.0F) {
        return 0.0F;
    }
    if (mPeriodEnabled) {
        return mPeriod.GetValue(prototype) / scale;
    }
    const int ownUnits = GetUnits();
    float begin;
    float end;
    if (mOptions.mUseFrameTime) {
        begin = resource->GetTimeFromFrame(
            mTimeRangeBeginEnabled ? mTimeRangeBegin.GetValue(prototype) : resource->GetStartFrame(),
            static_cast<TimeUnits>(ownUnits));
        end = resource->GetTimeFromFrame(
            mTimeRangeEndEnabled ? mTimeRangeEnd.GetValue(prototype) : resource->GetEndFrame(),
            static_cast<TimeUnits>(ownUnits));
    } else {
        begin = mTimeRangeBeginEnabled
            ? mTimeRangeBegin.GetValue(prototype)
            : resource->GetTimeFromFrame(resource->GetStartFrame(), mOptions.mTimeline);
        end = mTimeRangeEndEnabled ? mTimeRangeEnd.GetValue(prototype)
                                   : resource->GetTimeFromFrame(resource->GetEndFrame(), mOptions.mTimeline);
    }
    float length = fabsf((end - begin) / scale);
    if (units != kNumTimeUnits && (units == kTimeUnitsBeats) != (ownUnits == kTimeUnitsBeats)) {
        const float beatsPerSecond = rate * (1.0F / 60.0F);
        if (ownUnits == kTimeUnitsBeats) {
            length /= beatsPerSecond;
        } else {
            length *= beatsPerSecond;
        }
    }
    return length;
}

// Reconstructed from eboot.elf at 0x70FE60.
bool StateNodeAnimate::GetLength(float& length) const {
    const AnimResource* const resource = GetAnimResource();
    if (resource == nullptr) {
        length = 0.0F;
        return true;
    }
    Entity* const prototype = _GetContextEntityPrototype();
    float begin;
    float end;
    if (mOptions.mUseFrameTime) {
        begin = mTimeRangeBeginEnabled ? mTimeRangeBegin.GetValue(prototype) : resource->GetStartFrame();
        end = mTimeRangeEndEnabled ? mTimeRangeEnd.GetValue(prototype) : resource->GetEndFrame();
    } else {
        const TimeUnits units = static_cast<TimeUnits>(GetUnits());
        begin = resource->GetFrameFromTime(
            mTimeRangeBeginEnabled ? mTimeRangeBegin.GetValue(prototype)
                                   : resource->GetTimeFromFrame(resource->GetStartFrame(), mOptions.mTimeline),
            units);
        end = resource->GetFrameFromTime(
            mTimeRangeEndEnabled ? mTimeRangeEnd.GetValue(prototype)
                                 : resource->GetTimeFromFrame(resource->GetEndFrame(), mOptions.mTimeline),
            units);
    }
    length = fabsf(end - begin);
    return true;
}

// Reconstructed from eboot.elf at 0x70FFE0.
int StateNodeAnimate::GetUnits() const {
    if (mOptions.mTimeline != kNumTimeUnits) {
        return mOptions.mTimeline;
    }
    const AnimResource* const resource = GetAnimResource();
    return resource != nullptr ? resource->GetUnits() : kTimeUnitsSeconds;
}

// Reconstructed from eboot.elf at 0x710040.
void StateNodeAnimate::Refresh() const {
    if (mAnimType != CharClipResource::Id()) {
        return;
    }
    Entity* const prototype = _GetContextEntityPrototype();
    const GameObjectId clip = mClip.GetValue(prototype);
    if (clip.mId == gNullObjectId.mId) {
        return;
    }
    const GameObject* const object = prototype->GetObject(clip);
    if (object == nullptr) {
        return;
    }
    CharClipCom* const com = object->GetCom<CharClipCom>();
    if (com != nullptr) {
        com->Refresh();
    }
}

// Reconstructed from eboot.elf at 0x7101A0. The blend of a synchronized
// transition reaches past the end of the node's timing when its start and
// duration add up to more than one pass.
int StateNodeAnimate::GetLoopMode(bool* forced) const {
    int loopMode = mLoopMode;
    bool isForced = false;
    for (unsigned int index = 0; index < mTransitions.mSize; ++index) {
        const StateTransition* const transition = mRuntime.mTransitions[index];
        if (transition->IsSynchronized() &&
            transition->mAnimationBlendDuration + transition->mAnimationBlendStartTime > 1.0F) {
            loopMode = kLoopModeLoop;
            isForced = true;
            break;
        }
    }
    if (forced != nullptr) {
        *forced = isForced;
    }
    return loopMode;
}

// Reconstructed from eboot.elf at 0x710240.
ResourcePath StateNodeAnimate::GetAnimPath(Entity* entity) const {
    if (mAnimType != CharClipResource::Id()) {
        return mAnim.GetValue(entity);
    }
    const GameObjectId clip = mClip.GetValue(entity);
    if (clip.mId != gNullObjectId.mId) {
        const GameObject* const object = entity->GetObject(clip);
        if (object != nullptr) {
            const CharClipCom* const com = object->GetCom<CharClipCom>();
            if (com != nullptr) {
                const CharClipResource* const resource = com->mResource.Get();
                if (resource != nullptr && !resource->Fail()) {
                    return resource->mPath;
                }
            }
        }
    }
    return ResourcePath();
}

// Reconstructed from eboot.elf at 0x7103D0. _CreateInstance inlines it.
StateNodeAnimate::Instance::Instance(const StateNodeAnimate& node, StateGraphInstance& graph)
    : StateNodeInstance(node, graph), mAnimateNode(&node), mAnimId(), mWeight(0.0F), mTimingPosition(-1.0F) {}

// Reconstructed from eboot.elf at 0x710410. A forced current time plays
// the whole animation, which _Poll then positions; otherwise the enabled
// range, start and period replace the options' unset ones. A position set
// before the start places the playback on its range.
void StateNodeAnimate::Instance::_Enter() {
    const ResourcePath path = mAnimateNode->GetAnimPath(_GetContextEntity());
    AnimDriverCom* const driver = mGraph->mAnimDriver;
    if (driver == nullptr || path == ResourcePath()) {
        return;
    }
    const StateNodeAnimate* node = mAnimateNode;
    AnimOptions options = node->mOptions;
    if (node->mCurrentTimeEnabled) {
        options.mTimeRangeBegin = -9e29F;
        options.mTimeRangeEnd = 9e29F;
        options.mStartTime = AnimOptions::kInvalidTime;
        options.mPeriod = AnimOptions::kInvalidPeriod;
        options.mScale = 0.0F;
    } else {
        if (node->mTimeRangeBeginEnabled) {
            options.mTimeRangeBegin = node->mTimeRangeBegin.GetValue(_GetContextEntity());
            node = mAnimateNode;
        } else {
            options.mTimeRangeBegin = AnimOptions::kInvalidTime;
        }
        if (node->mTimeRangeEndEnabled) {
            options.mTimeRangeEnd = node->mTimeRangeEnd.GetValue(_GetContextEntity());
            node = mAnimateNode;
        } else {
            options.mTimeRangeEnd = AnimOptions::kInvalidTime;
        }
        if (node->mStartTimeEnabled) {
            options.mStartTime = node->mStartTime.GetValue(_GetContextEntity());
            node = mAnimateNode;
        } else {
            options.mStartTime = AnimOptions::kInvalidTime;
        }
        if (node->mPeriodEnabled) {
            options.mPeriod = node->mPeriod.GetValue(_GetContextEntity());
            node = mAnimateNode;
        } else {
            options.mPeriod = AnimOptions::kInvalidPeriod;
        }
        options.mScale = node->mScale.GetValue(_GetContextEntity());
    }
    mAnimId = driver->StartAnim(path, options);
    PlaybackInfo* const playback = driver->GetPlaybackInfo(mAnimId);
    if (playback == nullptr) {
        return;
    }
    playback->mAutoDestruct = options.mName == Symbol();
    const int loopMode = mAnimateNode->GetLoopMode(nullptr);
    if (loopMode != kLoopModeDefault) {
        playback->mLooping = loopMode == kLoopModeLoop;
    }
    if (mTimingPosition != -1.0F) {
        playback->mCurrentTime =
            playback->mBeginTime + (playback->mEndTime - playback->mBeginTime) * mTimingPosition;
    }
}

// Reconstructed from eboot.elf at 0x710690. A normalized current time
// spans the animation's frames; otherwise it is a frame or a time on the
// playback's timeline.
bool StateNodeAnimate::Instance::_Poll() {
    if (mAnimId.mDriver == nullptr) {
        return false;
    }
    AnimDriverCom* const driver = mGraph->mAnimDriver;
    Entity* const entity = _GetContextEntity();
    PlaybackInfo* const playback = driver->GetPlaybackInfo(mAnimId);
    if (playback == nullptr) {
        mAnimId = AnimInstID::NullID();
        return false;
    }
    playback->mWeight = mAnimateNode->mWeight.GetValue(entity) * mWeight;
    if (!mAnimateNode->mCurrentTimeEnabled) {
        playback->mScale = mAnimateNode->mScale.GetValue(entity);
    } else {
        const float time = mAnimateNode->mCurrentTime.GetValue(entity);
        if (mAnimateNode->mUseNormalizedTime) {
            const float start = playback->mAnim->GetStartFrame();
            const float end = playback->mAnim->GetEndFrame();
            const float clamped = 1.0F < time ? 1.0F : (time > 0.0F ? time : 0.0F);
            playback->SetCurrentFrame((end - start) * clamped + start);
        } else if (mAnimateNode->mOptions.mUseFrameTime) {
            playback->SetCurrentFrame(time);
        } else {
            playback->mCurrentTime = time;
        }
    }
    return !playback->mPaused;
}

// Reconstructed from eboot.elf at 0x7107F0. A named instance is paused, so
// a later node can resume it; an unnamed one is stopped.
void StateNodeAnimate::Instance::_Exit() {
    if (mAnimId.mDriver == nullptr || !mAnimateNode->mStopOnExit) {
        return;
    }
    AnimDriverCom* const driver = mGraph->mAnimDriver;
    if (mAnimateNode->mOptions.mName != Symbol()) {
        driver->PauseAnim(mAnimId, true);
    } else {
        driver->StopAnim(mAnimId);
    }
}

// Reconstructed from eboot.elf at 0x710850. One once the playback is gone.
float StateNodeAnimate::Instance::_GetTimingPosition() {
    if (mAnimId.mDriver == nullptr) {
        return 0.0F;
    }
    const PlaybackInfo* const playback = mGraph->mAnimDriver->GetPlaybackInfo(mAnimId);
    if (playback == nullptr) {
        return 1.0F;
    }
    return playback->GetTimingPosition();
}

// Reconstructed from eboot.elf at 0x710890.
void StateNodeAnimate::Instance::_SetTimingPosition(float position) {
    mTimingPosition = position;
}

// Reconstructed from eboot.elf at 0x710960.
Symbol StateNodeAnimate::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x710970.
Symbol StateNodeAnimate::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x710980.
int StateNodeAnimate::CurrentRev() const {
    return const_cast<StateNodeAnimate*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x7109A0.
bool StateNodeAnimate::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x7109D0.
Component* StateNodeAnimate::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x7109E0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateNodeAnimate::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodeAnimate*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeAnimate(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x710BB0.
PropRegistry& StateNodeAnimate::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x710BC0.
ComMetaData& StateNodeAnimate::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x710BD0.
const StateNodeTiming* StateNodeAnimate::GetTiming() const {
    return this;
}

// Reconstructed from eboot.elf at 0x710BE0.
StateNodeInstance* StateNodeAnimate::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x710C50.
bool StateNodeAnimate::CanBlend() const {
    return true;
}

// Reconstructed from eboot.elf at 0x710CA0.
void StateNodeAnimate::Instance::_SetWeight(float weight) {
    mWeight = weight;
}

// Reconstructed from eboot.elf at 0x6FEF30. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodeAnimate::_Create() {
    return new StateNodeAnimate();
}

// Reconstructed from eboot.elf at 0x6F73C0. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodeAnimate::Init() {
    InitComponentClass<StateNodeAnimate>();
}
