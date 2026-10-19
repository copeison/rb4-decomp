// stategraph/StateTransition.o (0x7F5220 to 0x7FB537). The class's
// registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/transitions/StateTransition.h"

#include <math.h>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropArray.h"
#include "stategraph/conditions/StateCondition.h"
#include "stategraph/conditions/StateConditionTimeElapsed.h"
#include "stategraph/nodes/StateNode.h"
#include "stategraph/nodes/StateNodeFolder.h"
#include "stategraph/runtime/StateGraphInstance.h"

namespace {

// The property accessors and tag functions of _Init. Names not in the
// reference map.

// "condition" sets the condition through the transition, which redraws it.
// Reconstructed from eboot.elf at 0x7F8E50.
void SetConditionAccessor(const PropAccessorArgs& args) {
    static_cast<StateTransition*>(args.mComponent)->SetCondition(*static_cast<const GameObjectId*>(args.mValue));
}

// Reconstructed from eboot.elf at 0x7F8E90.
void SetOriginAccessor(const PropAccessorArgs& args) {
    static_cast<StateTransition*>(args.mComponent)->SetOrigin(*static_cast<const GameObjectId*>(args.mValue));
}

// Reconstructed from eboot.elf at 0x7F8EC0.
void SetTargetAccessor(const PropAccessorArgs& args) {
    static_cast<StateTransition*>(args.mComponent)->SetTarget(*static_cast<const GameObjectId*>(args.mValue));
}

// The editor views of the synchronized blend's start time, duration and
// offset show each timing position as a percentage, as frames of the node's
// timing, and as seconds or beats. The start time and the duration are
// measured on the origin, the offset on the target.

StateTransition& Transition(const PropAccessorArgs& args) {
    return *static_cast<StateTransition*>(args.mComponent);
}

const StateTransition& Transition(const TagArgs& args) {
    return static_cast<const StateTransition&>(args.mComponent);
}

float Value(const PropAccessorArgs& args) {
    return *static_cast<const float*>(args.mValue);
}

// Sets a position on the node's timing from frames: nothing without a
// timing, zero for a timing without a length. Inlined into the frame
// setters; the getters use ScaleByNodeLength.
void SetFrames(const StateNode& node, float* position, float frames) {
    const StateNodeTiming* const timing = node.GetTiming();
    if (timing == nullptr) {
        return;
    }
    float length = 0.0F;
    timing->GetLength(length);
    *position = length != 0.0F ? frames / length : 0.0F;
}

// Reconstructed from eboot.elf at 0x7F8EF0.
float GetStartTimePercent(const PropAccessorArgs& args) {
    return Transition(args).mAnimationBlendStartTime * 100.0F;
}

// Reconstructed from eboot.elf at 0x7F8F10.
void SetStartTimePercent(const PropAccessorArgs& args) {
    Transition(args).mAnimationBlendStartTime = Value(args) * 0.01F;
}

// Reconstructed from eboot.elf at 0x7F8F30.
float GetStartTimeFrame(const PropAccessorArgs& args) {
    const StateTransition& transition = Transition(args);
    return transition.ScaleByNodeLength(*transition.mRuntime.mOriginNode, transition.mAnimationBlendStartTime);
}

// Reconstructed from eboot.elf at 0x7F8FB0.
void SetStartTimeFrame(const PropAccessorArgs& args) {
    StateTransition& transition = Transition(args);
    SetFrames(*transition.mRuntime.mOriginNode, &transition.mAnimationBlendStartTime, Value(args));
}

// Reconstructed from eboot.elf at 0x7F9040.
float GetStartTimeTime(const PropAccessorArgs& args) {
    const StateTransition& transition = Transition(args);
    return transition.ConvertToNodeTiming(*transition.mRuntime.mOriginNode, transition.mAnimationBlendStartTime);
}

// Reconstructed from eboot.elf at 0x7F90C0.
void SetStartTimeTime(const PropAccessorArgs& args) {
    StateTransition& transition = Transition(args);
    transition.ConvertFromNodeTiming(
        *transition.mRuntime.mOriginNode, &transition.mAnimationBlendStartTime, Value(args));
}

// Reconstructed from eboot.elf at 0x7F9150.
float GetDurationPercent(const PropAccessorArgs& args) {
    return Transition(args).mAnimationBlendDuration * 100.0F;
}

// Reconstructed from eboot.elf at 0x7F9170.
void SetDurationPercent(const PropAccessorArgs& args) {
    Transition(args).mAnimationBlendDuration = Value(args) * 0.01F;
}

// Reconstructed from eboot.elf at 0x7F9190.
float GetDurationFrame(const PropAccessorArgs& args) {
    const StateTransition& transition = Transition(args);
    return transition.ScaleByNodeLength(*transition.mRuntime.mOriginNode, transition.mAnimationBlendDuration);
}

// Reconstructed from eboot.elf at 0x7F9210.
void SetDurationFrame(const PropAccessorArgs& args) {
    StateTransition& transition = Transition(args);
    SetFrames(*transition.mRuntime.mOriginNode, &transition.mAnimationBlendDuration, Value(args));
}

// Reconstructed from eboot.elf at 0x7F92A0.
float GetDurationTime(const PropAccessorArgs& args) {
    const StateTransition& transition = Transition(args);
    return transition.ConvertToNodeTiming(*transition.mRuntime.mOriginNode, transition.mAnimationBlendDuration);
}

// Reconstructed from eboot.elf at 0x7F9320.
void SetDurationTime(const PropAccessorArgs& args) {
    StateTransition& transition = Transition(args);
    transition.ConvertFromNodeTiming(
        *transition.mRuntime.mOriginNode, &transition.mAnimationBlendDuration, Value(args));
}

// Reconstructed from eboot.elf at 0x7F93B0.
float GetOffsetPercent(const PropAccessorArgs& args) {
    return Transition(args).mAnimationBlendOffset * 100.0F;
}

// Reconstructed from eboot.elf at 0x7F93D0.
void SetOffsetPercent(const PropAccessorArgs& args) {
    Transition(args).mAnimationBlendOffset = Value(args) * 0.01F;
}

// Reconstructed from eboot.elf at 0x7F93F0.
float GetOffsetFrame(const PropAccessorArgs& args) {
    const StateTransition& transition = Transition(args);
    return transition.ScaleByNodeLength(*transition.mRuntime.mTargetNode, transition.mAnimationBlendOffset);
}

// Reconstructed from eboot.elf at 0x7F9470.
void SetOffsetFrame(const PropAccessorArgs& args) {
    StateTransition& transition = Transition(args);
    SetFrames(*transition.mRuntime.mTargetNode, &transition.mAnimationBlendOffset, Value(args));
}

// Reconstructed from eboot.elf at 0x7F9500.
float GetOffsetTime(const PropAccessorArgs& args) {
    const StateTransition& transition = Transition(args);
    return transition.ConvertToNodeTiming(*transition.mRuntime.mTargetNode, transition.mAnimationBlendOffset);
}

// Reconstructed from eboot.elf at 0x7F9580.
void SetOffsetTime(const PropAccessorArgs& args) {
    StateTransition& transition = Transition(args);
    transition.ConvertFromNodeTiming(
        *transition.mRuntime.mTargetNode, &transition.mAnimationBlendOffset, Value(args));
}

// The editor hides the views of a blend that is not synchronized.
bool IsUnsynchronized(const TagArgs& args) {
    return !Transition(args).IsSynchronized();
}

// A view's frames are hidden when the node's timing has no length.
bool HasNoLength(const StateNode& node) {
    const StateNodeTiming* const timing = node.GetTimingSource();
    float length = 0.0F;
    return timing == nullptr || !timing->GetLength(length);
}

// The units of a view's time: the node's timing counts in beats (1) or in
// seconds.
Symbol TimeUnitsLabel(const StateNode& node) {
    const StateNodeTiming* const timing = node.GetTimingSource();
    return Symbol(timing != nullptr && timing->GetUnits() == 1 ? "beats" : "seconds");
}

// Whether the blend takes time: it is synchronized or its duration is
// driven. The units, the blend curve and the interruption and blending
// settings follow it. The meaning is inferred and the evidence is weak.
bool BlendsOverTime(const StateTransition& transition) {
    return transition.IsSynchronized() || transition.mDuration.mDriven;
}

}  // namespace

// The object's statics, in the order of its static initializer (0x7FB480).
Symbol StateTransition::sId("StateTransition");
Symbol StateTransition::sClassName("StateTransition");
PropRegistry StateTransition::sPropRegistry;
ComMetaData StateTransition::sMetaData;

// Reconstructed from eboot.elf at 0x7F5220. The blend curve takes the
// default float waveform, drawing its evaluation state twice: for the
// temporary and for the copy.
StateTransition::StateTransition()
    : mOriginState{0xFFFFFFFFU},
      mTargetState{0xFFFFFFFFU},
      mCondition{0xFFFFFFFFU},
      mDuration(),
      mUnits(4),
      mBlendCurve(Waveform<float>(GetDefaultFloatWaveformResource().Get())),
      mCanInterrupt(true),
      mCanBeInterrupted(false),
      mHsvColorBlending(false),
      mBlendMode(kBlendAnimation),
      mRotationBlending(0),
      mAnimationBlendStartTime(0.9F),
      mAnimationBlendDuration(0.1F),
      mAnimationBlendOffset(0.0F),
      mRuntime{nullptr, nullptr, nullptr} {}

// Reconstructed from eboot.elf at 0x7F5420. Revision 5.
void StateTransition::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateTransition prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    GameObjectIdMetadata& condition = TypeSpecificMetadata(
        registry.RegisterProp("condition", offsetof(StateTransition, mCondition), kPropertyGameObjectId, 4),
        prototype.mCondition);
    condition.mDefault = prototype.mCondition;
    static_cast<void>(Symbol("prop"));
    condition.mIsHidden = true;
    condition.mSetter = SetConditionAccessor;

    GameObjectIdMetadata& origin = TypeSpecificMetadata(
        registry.RegisterProp("origin_state", offsetof(StateTransition, mOriginState), kPropertyGameObjectId, 4),
        prototype.mOriginState);
    origin.mDefault = prototype.mOriginState;
    static_cast<void>(Symbol("prop"));
    origin.mIsHidden = true;
    origin.mSetter = SetOriginAccessor;

    GameObjectIdMetadata& target = TypeSpecificMetadata(
        registry.RegisterProp("target_state", offsetof(StateTransition, mTargetState), kPropertyGameObjectId, 4),
        prototype.mTargetState);
    target.mDefault = prototype.mTargetState;
    static_cast<void>(Symbol("prop"));
    target.mIsHidden = true;
    target.mSetter = SetTargetAccessor;

    IntMetadata<int>& blendMode = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "blend_mode", offsetof(StateTransition, mBlendMode), kPropertyInt, 4).mMetadata.Get());
    blendMode.mDefault = prototype.mBlendMode;
    static_cast<void>(Symbol("prop"));
    blendMode.mHelp = String(
        "What kind of blending should be performed for this transition? Note that certain node types may not "
        "support blending of any kind.");
    blendMode.mIsHidden._Set([](const TagArgs& args) {
        const StateTransition& transition = Transition(args);
        return !transition.mRuntime.mOriginNode->HasValidTiming() ||
               !transition.mRuntime.mTargetNode->HasValidTiming();
    });
    blendMode.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            kBlendAnimation,
            String("Animation"),
            String("The 'from' animation and 'to' animation node will be crossfaded over a specified duration, "
                   "beginning at specified points in the 'from' and 'to' animations. This allows the user to pick "
                   "the most compatible portions of the animations involved and avoid a jerky transition.")},
        AllowedValue<int>{
            kBlendNormal,
            String("Normal"),
            String("The 'from' node and 'to' node will be crossfaded over a specified duration, beginning at the "
                   "instant this transition is activated.")}};

    // A transition out of a folder's start node takes no time.
    GroupMetadata& duration = prototype.mDuration.Register(
        registry, Symbol("duration"), offsetof(StateTransition, mDuration), StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    duration.mHelp = String("The length of time this transition takes to complete.");
    duration.mIsHidden._Set([](const TagArgs& args) {
        const StateTransition& transition = Transition(args);
        return transition.IsSynchronized() || transition.IsFromFolderStart();
    });
    PropPath::Node node;
    PropPath durationValue;
    durationValue.PushNode(node = Symbol("value"));
    FloatMetadata& durationValueMetadata = TypeSpecificMetadata(
        const_cast<PropInfo&>(*duration.mRegistry.FindProp(durationValue, nullptr)), prototype.mDuration.mValue);
    static_cast<void>(Symbol("driven_prop_value"));
    durationValueMetadata.mMin = 0.0F;

    FloatMetadata& startTime = TypeSpecificMetadata(
        registry.RegisterProp(
            "animation_blend_start_time", offsetof(StateTransition, mAnimationBlendStartTime), kPropertyFloat, 4),
        prototype.mAnimationBlendStartTime);
    startTime.mDefault = prototype.mAnimationBlendStartTime;
    static_cast<void>(Symbol("prop"));
    startTime.mIsHidden = true;

    GroupMetadata& startView = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "animation_blend_start_time_editor_view", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    PropRegistry& startMembers = startView.mRegistry;
    startView.mDisplayName = String("blend start");
    startView.mHelp = String(
        "The point in the 'from' animation at which the blending should begin.\n"
        "\n"
        "This value is represented three ways: as a percentage of the duration of the 'from' animation, as a frame "
        "within the 'from' animation, and as a real time relative to the start of the 'from' animation in seconds "
        "or beats.\n"
        "\n"
        "Note that for compound animations like blend trees, etc., the real time value will be approximate, and "
        "frame values will be unavailable.\n"
        "\n"
        "If a value greater than the length of the animation is set, then the animation will loop one or more "
        "times as necessary before the blend begins. This allows the user to delay the blend if desired.");
    startView.mKind = 1;
    startView.mIsHidden._Set(IsUnsynchronized);

    float view = 0.0F;
    FloatMetadata& startPercent =
        TypeSpecificMetadata(startMembers.RegisterProp("percent", -1, kPropertyFloat, 0), view);
    static_cast<void>(Symbol("prop"));
    startPercent.mHelp =
        String("The blend start point, expressed as a percentage of the of the 'from' animation's duration.");
    startPercent.mMin = 0.0F;
    startPercent.mIncrement = 0.01F;
    startPercent.mUnits = Symbol("%");
    startPercent.mGetter = GetStartTimePercent;
    startPercent.mSetter = SetStartTimePercent;

    view = 0.0F;
    FloatMetadata& startFrame = TypeSpecificMetadata(startMembers.RegisterProp("frame", -1, kPropertyFloat, 0), view);
    static_cast<void>(Symbol("prop"));
    startFrame.mHelp = String(
        "The blend start point, expressed as frames relative to the beginning of the 'from' animation.\n"
        "\n"
        "Note that these are 'frames' as authored in the animation itself, not necessarily 30FPS frames depending "
        "on playback scale, etc.");
    startFrame.mMin = 0.0F;
    startFrame.mIncrement = 0.01F;
    startFrame.mUnits = Symbol("frames");
    startFrame.mIsHidden._Set(
        [](const TagArgs& args) { return HasNoLength(*Transition(args).mRuntime.mOriginNode); });
    startFrame.mGetter = GetStartTimeFrame;
    startFrame.mSetter = SetStartTimeFrame;

    view = 0.0F;
    FloatMetadata& startSeconds = TypeSpecificMetadata(startMembers.RegisterProp("time", -1, kPropertyFloat, 0), view);
    static_cast<void>(Symbol("prop"));
    startSeconds.mHelp = String(
        "The blend start point, expressed as seconds or beats relative to the beginning of the 'from' animation.\n"
        "\n"
        "For compound animations like blend trees, this value will be an approximation.");
    startSeconds.mMin = 0.0F;
    startSeconds.mIncrement = 0.01F;
    startSeconds.mUnits._Set(
        [](const TagArgs& args) { return TimeUnitsLabel(*Transition(args).mRuntime.mOriginNode); });
    startSeconds.mGetter = GetStartTimeTime;
    startSeconds.mSetter = SetStartTimeTime;

    FloatMetadata& blendDuration = TypeSpecificMetadata(
        registry.RegisterProp(
            "animation_blend_duration", offsetof(StateTransition, mAnimationBlendDuration), kPropertyFloat, 4),
        prototype.mAnimationBlendDuration);
    blendDuration.mDefault = prototype.mAnimationBlendDuration;
    static_cast<void>(Symbol("prop"));
    blendDuration.mIsHidden = true;

    GroupMetadata& durationView = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "animation_blend_duration_editor_view", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    PropRegistry& durationMembers = durationView.mRegistry;
    durationView.mDisplayName = String("blend duration");
    // The binary repeats the start time's first sentence.
    durationView.mHelp = String(
        "The point in the 'from' animation at which the blending should begin.\n"
        "\n"
        "This value is represented three ways: as a percentage of the 'from' animation's duration, as a number of "
        "frames worth of the 'from' animation, and as an amount of real time in seconds or beats.\n"
        "\n"
        "Note that for compound animations like blend trees, etc., the real time transition duration will be "
        "approximate, and frame values will be unavailable.\n"
        "\n"
        "If a value greater than the length of the animation is set, then the animation will loop one or more "
        "times as necessary during the blend.");
    durationView.mKind = 1;
    durationView.mIsHidden._Set(IsUnsynchronized);

    view = 0.0F;
    FloatMetadata& durationPercent =
        TypeSpecificMetadata(durationMembers.RegisterProp("percent", -1, kPropertyFloat, 0), view);
    static_cast<void>(Symbol("prop"));
    durationPercent.mHelp =
        String("The blend duration, expressed as a percentage of the of the 'from' animation's duration.");
    durationPercent.mMin = 0.0F;
    durationPercent.mIncrement = 0.01F;
    durationPercent.mUnits = Symbol("%");
    durationPercent.mGetter = GetDurationPercent;
    durationPercent.mSetter = SetDurationPercent;

    view = 0.0F;
    FloatMetadata& durationFrame =
        TypeSpecificMetadata(durationMembers.RegisterProp("frame", -1, kPropertyFloat, 0), view);
    static_cast<void>(Symbol("prop"));
    durationFrame.mHelp = String(
        "The blend duration, expressed as the number of frames of the 'from' animation that will be played during "
        "the transition.\n"
        "\n"
        "Note that these are 'frames' as authored in the animation itself, not necessarily 30FPS frames depending "
        "on playback scale, etc.");
    durationFrame.mMin = 0.0F;
    durationFrame.mIncrement = 0.01F;
    durationFrame.mUnits = Symbol("frames");
    durationFrame.mIsHidden._Set(
        [](const TagArgs& args) { return HasNoLength(*Transition(args).mRuntime.mOriginNode); });
    durationFrame.mGetter = GetDurationFrame;
    durationFrame.mSetter = SetDurationFrame;

    view = 0.0F;
    FloatMetadata& durationSeconds =
        TypeSpecificMetadata(durationMembers.RegisterProp("time", -1, kPropertyFloat, 0), view);
    static_cast<void>(Symbol("prop"));
    durationSeconds.mHelp = String(
        "The blend duration, expressed as seconds or beats.\n"
        "\n"
        "For compound animations like blend trees, this value will be an approximation.");
    durationSeconds.mMin = 0.0F;
    durationSeconds.mIncrement = 0.01F;
    durationSeconds.mUnits._Set(
        [](const TagArgs& args) { return TimeUnitsLabel(*Transition(args).mRuntime.mOriginNode); });
    durationSeconds.mGetter = GetDurationTime;
    durationSeconds.mSetter = SetDurationTime;

    FloatMetadata& offset = TypeSpecificMetadata(
        registry.RegisterProp(
            "animation_blend_offset", offsetof(StateTransition, mAnimationBlendOffset), kPropertyFloat, 4),
        prototype.mAnimationBlendOffset);
    offset.mDefault = prototype.mAnimationBlendOffset;
    static_cast<void>(Symbol("prop"));
    offset.mIsHidden = true;

    GroupMetadata& offsetView = static_cast<GroupMetadata&>(*registry.RegisterProp(
        "animation_blend_offset_editor_view", 0, kPropertyStruct, 0).mMetadata.Get());
    static_cast<void>(Symbol("struct"));
    PropRegistry& offsetMembers = offsetView.mRegistry;
    offsetView.mDisplayName = String("blend offset");
    offsetView.mHelp = String(
        "The point at which the 'to' animation should start when blending begins.\n"
        "\n"
        "This value is represented three ways: as a percentage of the duration of the 'to' animation, as a frame "
        "within the 'to' animation, and as a real time relative to the start of the 'to' animation in seconds or "
        "beats.\n"
        "\n"
        "Note that for compound animations like blend trees, etc., the real time value will be approximate, and "
        "frame values will be unavailable.");
    offsetView.mKind = 1;
    offsetView.mIsHidden._Set(IsUnsynchronized);

    view = 0.0F;
    FloatMetadata& offsetPercent =
        TypeSpecificMetadata(offsetMembers.RegisterProp("percent", -1, kPropertyFloat, 0), view);
    static_cast<void>(Symbol("prop"));
    offsetPercent.mHelp = String(
        "The point at which the 'to' animation should start when blending begins, expressed as a percentage of the "
        "of the 'to' animation's duration.");
    offsetPercent.mMin = 0.0F;
    offsetPercent.mMax = 100.0F;
    offsetPercent.mIncrement = 0.01F;
    offsetPercent.mUnits = Symbol("%");
    offsetPercent.mGetter = GetOffsetPercent;
    offsetPercent.mSetter = SetOffsetPercent;

    view = 0.0F;
    FloatMetadata& offsetFrame =
        TypeSpecificMetadata(offsetMembers.RegisterProp("frame", -1, kPropertyFloat, 0), view);
    static_cast<void>(Symbol("prop"));
    offsetFrame.mHelp = String(
        "The point at which the 'to' animation should start when blending begins, expressed as frames relative to "
        "the beginning of the 'to' animation.\n"
        "\n"
        "Note that these are 'frames' as authored in the animation itself, not necessarily 30FPS frames depending "
        "on playback scale, etc.");
    offsetFrame.mMin = 0.0F;
    // At most the length of the target's timing.
    offsetFrame.mMax._Set([](const TagArgs& args) {
        float length = 0.0F;
        const StateNodeTiming* const timing = Transition(args).mRuntime.mTargetNode->GetTimingSource();
        if (timing == nullptr) {
            return 0.0F;
        }
        timing->GetLength(length);
        return length;
    });
    offsetFrame.mIncrement = 0.01F;
    offsetFrame.mUnits = Symbol("frames");
    offsetFrame.mIsHidden._Set(
        [](const TagArgs& args) { return HasNoLength(*Transition(args).mRuntime.mTargetNode); });
    offsetFrame.mGetter = GetOffsetFrame;
    offsetFrame.mSetter = SetOffsetFrame;

    view = 0.0F;
    FloatMetadata& offsetSeconds =
        TypeSpecificMetadata(offsetMembers.RegisterProp("time", -1, kPropertyFloat, 0), view);
    static_cast<void>(Symbol("prop"));
    offsetSeconds.mHelp = String(
        "The point at which the 'to' animation should start when blending begins, expressed as seconds or beats "
        "relative to the beginning of the 'to' animation.\n"
        "\n"
        "For compound animations like blend trees, this value will be an approximation.");
    offsetSeconds.mMin = 0.0F;
    // At most the target timing's scale in the driver's default units (4).
    offsetSeconds.mMax._Set([](const TagArgs& args) {
        const StateNodeTiming* const timing = Transition(args).mRuntime.mTargetNode->GetTimingSource();
        return timing != nullptr ? timing->GetScale(4, 120.0F) : 0.0F;
    });
    offsetSeconds.mIncrement = 0.01F;
    offsetSeconds.mUnits._Set(
        [](const TagArgs& args) { return TimeUnitsLabel(*Transition(args).mRuntime.mTargetNode); });
    offsetSeconds.mGetter = GetOffsetTime;
    offsetSeconds.mSetter = SetOffsetTime;

    IntMetadata<int>& units = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("units", offsetof(StateTransition, mUnits), kPropertyInt, 4).mMetadata.Get());
    units.mDefault = prototype.mUnits;
    static_cast<void>(Symbol("prop"));
    units.mHelp = String("The time units used to measure the duration of this transition.");
    units.mIsHidden._Set([](const TagArgs& args) {
        const StateTransition& transition = Transition(args);
        return transition.IsSynchronized() || !BlendsOverTime(transition);
    });
    units.mAllowedValues.mMode = TagBase<eastl::vector<AllowedValue<int>>>::kModeFunction;
    units.mAllowedValues.mFunction = gTimeUnitsAllowedValues;

    GroupMetadata& blendCurve = prototype.mBlendCurve.Register(
        registry, Symbol("blend_curve"), offsetof(StateTransition, mBlendCurve), StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    blendCurve.mHelp = String("A curve defining the blend ratio over time between the connected states.");
    blendCurve.mIsHidden._Set([](const TagArgs& args) { return !BlendsOverTime(Transition(args)); });
    PropPath blendCurveValue;
    blendCurveValue.PushNode(node = Symbol("value"));
    WaveformMetadata<float>& blendCurveValueMetadata = TypeSpecificMetadata(
        const_cast<PropInfo&>(*blendCurve.mRegistry.FindProp(blendCurveValue, nullptr)),
        prototype.mBlendCurve.mValue);
    static_cast<void>(Symbol("driven_prop_value"));
    // The binary assigns the list through a shared helper (0x2D58C0).
    blendCurveValueMetadata.mAllowedTypes = {2};

    BoolMetadata& canInterrupt = TypeSpecificMetadata(
        registry.RegisterProp("can_interrupt", offsetof(StateTransition, mCanInterrupt), kPropertyBool, 1),
        prototype.mCanInterrupt);
    canInterrupt.mDefault = prototype.mCanInterrupt;
    static_cast<void>(Symbol("prop"));
    canInterrupt.mHelp = String(
        "If true, this transition can interrupt a transition into the node this transition comes from, if that "
        "transition is marked \"can be interrupted\". This is ignored for transitions with no condition, as an "
        "unconditional interrupt would be pointless (and should just be refactored).");
    canInterrupt.mIsHidden._Set([](const TagArgs& args) {
        const StateTransition& transition = Transition(args);
        return transition.mRuntime.mCondition == nullptr || transition.IsFromFolderStart();
    });

    BoolMetadata& canBeInterrupted = TypeSpecificMetadata(
        registry.RegisterProp(
            "can_be_interrupted", offsetof(StateTransition, mCanBeInterrupted), kPropertyBool, 1),
        prototype.mCanBeInterrupted);
    canBeInterrupted.mDefault = prototype.mCanBeInterrupted;
    static_cast<void>(Symbol("prop"));
    canBeInterrupted.mHelp = String(
        "If true, transitions away from the node this transition goes to, if marked \"can interrupt\", can be "
        "activated before this transition has finished, leaving this blend incomplete. It is up to the following "
        "nodes of the graph to correct for the interrupted blend.");
    canBeInterrupted.mIsHidden._Set([](const TagArgs& args) { return !BlendsOverTime(Transition(args)); });

    BoolMetadata& hsvColorBlending = TypeSpecificMetadata(
        registry.RegisterProp(
            "hsv_color_blending", offsetof(StateTransition, mHsvColorBlending), kPropertyBool, 1),
        prototype.mHsvColorBlending);
    hsvColorBlending.mDefault = prototype.mHsvColorBlending;
    static_cast<void>(Symbol("prop"));
    hsvColorBlending.mHelp = String(
        "If true, any SetProperty nodes targeting color properties will blend in the HSV format, preserving their "
        "brightness and saturation but altering their hue more dramatically. If false, colors will blend in RGB "
        "space, which provides a more direct hue interpolation while altering brightness and saturation more "
        "dramatically. This setting has no effect on non-color SetProperty nodes.");
    hsvColorBlending.mIsHidden._Set([](const TagArgs& args) {
        const StateTransition& transition = Transition(args);
        return !BlendsOverTime(transition) || !transition.mRuntime.mTargetNode->UsesColorBlending();
    });

    IntMetadata<int>& rotationBlending = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "rotation_blending", offsetof(StateTransition, mRotationBlending), kPropertyInt, 4).mMetadata.Get());
    rotationBlending.mDefault = prototype.mRotationBlending;
    static_cast<void>(Symbol("prop"));
    rotationBlending.mHelp = String(
        "Any SetProperty nodes targeting rotation properties will blend in the specified format. Euler blending "
        "will interpolate pitch, roll, and yaw directly, which may result in a strange path if multiple angles "
        "change by a large amount simultaneously. However, using Euler blending it is possible to interpolate "
        "beyond a 180 degree arc on each axis. Quaternion blending will result in a direct interpolation, which "
        "may be more intuitive when multiple axes of rotation are involved. However, quaternion blending always "
        "follows the shortest path between the blended orientations. This means the path will always fall within "
        "a 180 degree arc on each axis. This setting has no effect on non-rotation SetProperty nodes.");
    rotationBlending.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{0, String("Euler"), String("Blend rotations using Euler interpolation.")},
        AllowedValue<int>{1, String("Quaternion"), String("Blend rotations using quaternion interpolation.")}};
    rotationBlending.mIsHidden._Set([](const TagArgs& args) {
        const StateTransition& transition = Transition(args);
        return !BlendsOverTime(transition) || !transition.mRuntime.mTargetNode->UsesRotationBlending();
    });

    StateBase::_FixDrivenProp<float>(registry, PropPath(Symbol("duration")));

    // Before revision 3 the transitions blended normally; before revision 4
    // the blend took the duration; before revision 5 the start time and the
    // offset were saved as "start_time" and "offset".
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision = [](const PropRegistry::RevisionArgs& args) {
        StateTransition* const transition = static_cast<StateTransition*>(args.mComponent);
        if (args.mSavedRev > 4) {
            return;
        }
        if (args.mSavedRev <= 2) {
            transition->mBlendMode = kBlendNormal;
        }
        if (args.mSavedRev <= 3) {
            transition->mAnimationBlendDuration = transition->mDuration.mValue;
        }
        static Symbol start_time;
        if (start_time == Symbol()) {
            start_time = Symbol("start_time");
        }
        Component* saved = args.mSavedProps;
        const PropPath startPath(start_time);
        void* storage = saved;
        const PropInfo* info = saved->_GetPropRegistry().FindProp(startPath, &storage);
        if (info != nullptr) {
            _GetProp(*saved, startPath, *info, storage, &transition->mAnimationBlendStartTime, kPropertyFloat);
        }
        static Symbol offset;
        if (offset == Symbol()) {
            offset = Symbol("offset");
        }
        saved = args.mSavedProps;
        const PropPath offsetPath(offset);
        storage = saved;
        info = saved->_GetPropRegistry().FindProp(offsetPath, &storage);
        if (info != nullptr) {
            _GetProp(*saved, offsetPath, *info, storage, &transition->mAnimationBlendOffset, kPropertyFloat);
        }
    };
    if (&own == &registry) {
        registry.SetCurrentRev(5);
        registry.mRevisionFunc = onRevision;
    }

    StateBase::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "A transition line within a StateGraph.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mInterface = sClassName;
}

// Reconstructed from eboot.elf at 0x7F7C40. The deleting destructor is at
// 0x7F8A30; the blend curve releases its resources.
StateTransition::~StateTransition() {}

// Reconstructed from eboot.elf at 0x7F7C90.
void StateTransition::_Fixup() {
    mRuntime.mOriginNode = _GetObjectBaseCom<StateNode>(mOriginState, true);
    mRuntime.mTargetNode = _GetObjectBaseCom<StateNode>(mTargetState, true);
    mRuntime.mCondition = _GetObjectBaseCom<StateCondition>(mCondition, true);
}

// Reconstructed from eboot.elf at 0x7F7CF0.
void StateTransition::SetOrigin(GameObjectId origin) {
    mOriginState = origin;
    mRuntime.mOriginNode = _GetObjectBaseCom<StateNode>(origin, true);
}

// Reconstructed from eboot.elf at 0x7F7D20.
void StateTransition::SetTarget(GameObjectId target) {
    mTargetState = target;
    mRuntime.mTargetNode = _GetObjectBaseCom<StateNode>(target, true);
}

// Reconstructed from eboot.elf at 0x7F7D50.
void StateTransition::SetCondition(GameObjectId condition) {
    if (mCondition.mId != condition.mId) {
        mCondition = condition;
        mRuntime.mCondition = _GetObjectBaseCom<StateCondition>(condition, true);
        _SendDisplayTextUpdated();
    }
}

// Reconstructed from eboot.elf at 0x7F7D90.
void StateTransition::_LoadBlendCurve() {
    static Symbol blendCurve;
    if (blendCurve == Symbol()) {
        blendCurve = Symbol("blend_curve");
    }
    const ResourcePath path = GetInlineWaveformPath(*this, PropPath(blendCurve));
    const ResourcePtr<WaveformResource<float>> resource =
        mObject->mEntity->mResource->TryGetInline<WaveformResource<float>>(path);
    if (resource != nullptr) {
        mBlendCurve.mValue.SetResource(resource.Get());
    }
}

// Reconstructed from eboot.elf at 0x7F7FD0.
bool StateTransition::TargetUsesColorBlending() const {
    return mRuntime.mTargetNode->UsesColorBlending();
}

// Reconstructed from eboot.elf at 0x7F7FE0.
bool StateTransition::TargetUsesRotationBlending() const {
    return mRuntime.mTargetNode->UsesRotationBlending();
}

// Reconstructed from eboot.elf at 0x7F7FF0.
bool StateTransition::TakesTime() const {
    return IsSynchronized() || mDuration.mDriven || 0.0F < mDuration.mValue;
}

// Reconstructed from eboot.elf at 0x7F8050.
bool StateTransition::IsSynchronized() const {
    return mBlendMode == kBlendAnimation && mRuntime.mOriginNode->HasValidTiming() &&
           mRuntime.mTargetNode->HasValidTiming();
}

// Reconstructed from eboot.elf at 0x7F8090.
float StateTransition::GetStartWindow() const {
    return static_cast<float>(static_cast<int>(mAnimationBlendStartTime)) + 1.0F;
}

// Reconstructed from eboot.elf at 0x7F80B0.
float StateTransition::WrapToStartWindow(float position) const {
    const float window = static_cast<float>(static_cast<int>(mAnimationBlendStartTime)) + 1.0F;
    float wrapped = 0.0F;
    if (window != 0.0F) {
        const float remainder = fmodf(position, window);
        wrapped = remainder < 0.0F ? remainder + window : remainder;
    }
    return wrapped;
}

// Reconstructed from eboot.elf at 0x7F8100.
bool StateTransition::IsFromFolderStart() const {
    const StateNode* const origin = mRuntime.mOriginNode;
    const StateNodeFolder* const folder = origin->mRuntime.mFolderNode;
    return folder != nullptr && folder->mStart.mId == origin->mObject->mId.mId;
}

// Reconstructed from eboot.elf at 0x7F8130.
const StateNodeTiming* StateTransition::GetOriginTiming() const {
    return mRuntime.mOriginNode->GetTimingSource();
}

// Reconstructed from eboot.elf at 0x7F8140.
const StateNodeTiming* StateTransition::GetTargetTiming() const {
    return mRuntime.mTargetNode->GetTimingSource();
}

// Reconstructed from eboot.elf at 0x7F8150.
float StateTransition::ConvertToNodeTiming(const StateNode& node, float position) const {
    const StateNodeTiming* const origin = mRuntime.mOriginNode->GetTimingSource();
    const StateNodeTiming* const timing = node.GetTiming();
    if (origin == nullptr || timing == nullptr) {
        return 0.0F;
    }
    return timing->GetScale(origin->GetUnits(), 120.0F) * position;
}

// Reconstructed from eboot.elf at 0x7F81C0. Either timing missing leaves
// the position; the binary calls both whenever one exists.
void StateTransition::ConvertFromNodeTiming(const StateNode& node, float* position, float nodePosition) const {
    const StateNodeTiming* const origin = mRuntime.mOriginNode->GetTimingSource();
    const StateNodeTiming* const timing = node.GetTiming();
    if (origin == nullptr && timing == nullptr) {
        return;
    }
    const float scale = timing->GetScale(origin->GetUnits(), 120.0F);
    *position = scale != 0.0F ? nodePosition / scale : 0.0F;
}

// Reconstructed from eboot.elf at 0x7F8250.
float StateTransition::ScaleByNodeLength(const StateNode& node, float fraction) const {
    const StateNodeTiming* const timing = node.GetTiming();
    if (timing == nullptr) {
        return 0.0F;
    }
    float length = 0.0F;
    timing->GetLength(length);
    return fraction * length;
}

// Reconstructed from eboot.elf at 0x7F8340.
StateTransitionInstance::StateTransitionInstance(StateGraphInstance& graph, const StateTransition& transition)
    : mGraph(&graph), mTransition(&transition), mElapsed(0.0F), mOriginPosition(0.0F), mSyncProgress(0.0F) {}

// Reconstructed from eboot.elf at 0x7F8360. The synchronized blend's
// progress starts at the origin's position past the start time, wrapped
// into the start window.
bool StateTransitionInstance::Begin(StateNodeInstance* from, StateNodeInstance* to) {
    if (mTransition->IsSynchronized()) {
        to->SetTimingPosition(mTransition->mAnimationBlendOffset);
        mOriginPosition = from->GetTimingPosition();
        const float start = mTransition->mAnimationBlendStartTime;
        const float whole = static_cast<float>(static_cast<int>(start));
        const float window = whole + 1.0F;
        float wrapped = 0.0F;
        if (window != 0.0F) {
            const float remainder = fmodf(mOriginPosition, window);
            wrapped = remainder < 0.0F ? remainder + window : remainder;
        }
        mSyncProgress = (wrapped < start ? wrapped + 1.0F + whole : wrapped) - start;
        return true;
    }
    return mTransition->TakesTime();
}

// Reconstructed from eboot.elf at 0x7F84B0. A synchronized blend follows
// the origin's timing over the blend duration; any other runs for its
// duration in its units. The blend is the curve at the clamped fraction.
bool StateTransitionInstance::Poll(StateNodeInstance* from, StateNodeInstance* to, float* blend) {
    static_cast<void>(to);
    Entity* const entity = mGraph->mEntity;
    float fraction;
    if (mTransition->IsSynchronized()) {
        if (from->mFinished) {
            *blend = 1.0F;
            return false;
        }
        const float position = from->GetTimingPosition();
        mSyncProgress = position - mOriginPosition + mSyncProgress;
        const float duration = mTransition->mAnimationBlendDuration;
        mOriginPosition = position;
        if (duration == 0.0F) {
            *blend = 1.0F;
            return false;
        }
        const float upper = duration > 0.0F ? duration : 0.0F;
        const float lower = duration < 0.0F ? duration : 0.0F;
        fraction = (upper < mSyncProgress ? upper : (mSyncProgress > lower ? mSyncProgress : lower)) / duration;
    } else {
        const int units = mGraph->ResolveTimeUnits(mTransition->mUnits);
        mElapsed += mGraph->mClock->DeltaTime(static_cast<TimeUnits>(units));
        const float duration = mTransition->mDuration.GetValue(entity);
        if (!(duration > 0.0F)) {
            *blend = 1.0F;
            return false;
        }
        const float upper = duration > 0.0F ? duration : 0.0F;
        const float lower = duration < 0.0F ? duration : 0.0F;
        fraction = (upper < mElapsed ? upper : (mElapsed > lower ? mElapsed : lower)) / duration;
    }
    if (fraction == 1.0F) {
        *blend = 1.0F;
        return false;
    }
    Waveform<float> curve = mTransition->mBlendCurve.GetValue(entity);
    *blend = curve.Evaluate(fraction);
    return true;
}

// Reconstructed from eboot.elf at 0x7F8A20.
void StateTransitionInstance::End() {}

// Reconstructed from eboot.elf at 0x7F8A80.
Symbol StateTransition::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x7F8A90.
Symbol StateTransition::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x7F8AA0.
int StateTransition::CurrentRev() const {
    return const_cast<StateTransition*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x7F8AC0.
bool StateTransition::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x7F8AF0.
Component* StateTransition::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x7F8B00. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateTransition::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateTransition*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateTransition(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x7F8E30.
PropRegistry& StateTransition::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x7F8E40.
ComMetaData& StateTransition::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x700E70. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateTransition::_Create() {
    return new StateTransition();
}

// Reconstructed from eboot.elf at 0x6FBC40. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateTransition::Init() {
    InitComponentClass<StateTransition>();
}
