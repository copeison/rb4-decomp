#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "entity/waveform/Waveform.h"
#include "stategraph/base/StateBase.h"
#include "stategraph/props/DrivenProp.h"
#include "utl/text/Symbol.h"

class StateCondition;
class StateGraphInstance;
class StateNode;
class StateNodeInstance;
class StateNodeTiming;

// A transition between two nodes of a state graph
// (stategraph/StateTransition.o, 0x7F5220-0x7FB537; class id
// "StateTransition"): its origin and target nodes, the condition that
// starts it, and how the nodes blend while it runs: for its duration along
// its blend curve, or synchronized to the nodes' animation timing. The
// vtable at 0x194ECF0 has 42 slots. The object is 600 bytes.
class StateTransition : public StateBase {
public:
    // How the nodes blend ("blend_mode"). The labels are the allowed
    // values'; the enumerator names are not in the reference map.
    enum BlendMode : int {
        // Synchronized to the animation timing of both nodes, when they
        // have one.
        kBlendAnimation = 0,
        kBlendNormal = 1,
    };

    // The nodes and condition the ids name, resolved by _Fixup. Name and
    // field names not in the reference map.
    struct RuntimeData {
        StateNode* mOriginNode;
        StateNode* mTargetNode;
        StateCondition* mCondition;
    };

    StateTransition();  // 0x7F5220
    // Copies the transition for an imprint; the blend curve draws a new
    // evaluation state and the links start empty. Inlined into _Imprint
    // (0x7F8B00).
    StateTransition(const StateTransition& other)
        : StateBase(other),
          mOriginState(other.mOriginState),
          mTargetState(other.mTargetState),
          mCondition(other.mCondition),
          mDuration(other.mDuration),
          mUnits(other.mUnits),
          mBlendCurve(other.mBlendCurve),
          mCanInterrupt(other.mCanInterrupt),
          mCanBeInterrupted(other.mCanBeInterrupted),
          mHsvColorBlending(other.mHsvColorBlending),
          mBlendMode(other.mBlendMode),
          mRotationBlending(other.mRotationBlending),
          mAnimationBlendStartTime(other.mAnimationBlendStartTime),
          mAnimationBlendDuration(other.mAnimationBlendDuration),
          mAnimationBlendOffset(other.mAnimationBlendOffset),
          mRuntime{nullptr, nullptr, nullptr} {}
    // Slots 0-1: 0x7F7C40 and 0x7F8A30.
    ~StateTransition() override;

    Symbol GetId() const override;          // slot 4: 0x7F8A80
    Symbol GetClassName() const override;   // slot 5: 0x7F8A90
    int CurrentRev() const override;        // slot 7: 0x7F8AA0
    bool IsA(Symbol type) const override;   // slot 8: 0x7F8AC0
    Component* AsComponent() override;      // slot 9: 0x7F8AF0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x7F8B00
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x7F8E30
    ComMetaData& _GetMetaData() override;       // slot 23: 0x7F8E40
    // Slot 41: resolves the origin, the target and the condition.
    void _Fixup() override;  // 0x7F7C90

    // Set the origin and the target, resolving them. Names not in the
    // reference map.
    void SetOrigin(GameObjectId origin);  // 0x7F7CF0
    void SetTarget(GameObjectId target);  // 0x7F7D20
    // Sets the condition and redraws the transition when it changed. Name
    // not in the reference map.
    void SetCondition(GameObjectId condition);  // 0x7F7D50
    // Takes the blend curve from the graph's inline waveform resource for
    // the "blend_curve" property, when there is one. Perhaps the map's
    // _LoadResources(ObjPtr const&), which this build's slot 21 would
    // hide; the name is inferred.
    void _LoadBlendCurve();  // 0x7F7D90
    // Slots 47 and 48 of the target node. No caller in this build. Names
    // not in the reference map.
    bool TargetUsesColorBlending() const;  // 0x7F7FD0
    bool TargetUsesRotationBlending() const;  // 0x7F7FE0
    // Whether the transition takes time: it is synchronized, or it has a
    // driven or positive duration. No caller in this build. Name not in the
    // reference map.
    bool TakesTime() const;  // 0x7F7FF0
    // Whether the nodes blend synchronized to their animation timing: the
    // animation blend mode with both nodes timed. Name not in the
    // reference map.
    bool IsSynchronized() const;  // 0x7F8050
    // The window of timing positions the synchronized blend may start in:
    // the whole part of the start time plus one, and a position wrapped
    // into it. Names not in the reference map.
    float GetStartWindow() const;            // 0x7F8090
    float WrapToStartWindow(float position) const;  // 0x7F80B0
    // Whether the origin is its folder's start node. No caller in this
    // build. Name not in the reference map.
    bool IsFromFolderStart() const;  // 0x7F8100
    // The timing of the origin and of the target. No caller in this build.
    // Names not in the reference map.
    const StateNodeTiming* GetOriginTiming() const;  // 0x7F8130
    const StateNodeTiming* GetTargetTiming() const;  // 0x7F8140
    // Convert a position on the origin's timing to the node's timing and
    // back, through the timings' scales at 120 per second, and scale a
    // fraction by the length of the node's timing. No caller in this
    // build. Names not in the reference map; the evidence is weak.
    float ConvertToNodeTiming(const StateNode& node, float position) const;  // 0x7F8150
    void ConvertFromNodeTiming(const StateNode& node, float* position, float nodePosition) const;  // 0x7F81C0
    float ScaleByNodeLength(const StateNode& node, float fraction) const;  // 0x7F8250

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6FBC40.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x700E70.
    static Component* _Create();
    // Registers the links, the blend settings and the editor views of the
    // synchronized blend's timing positions (percent, frames, time).
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x7F5420

    static Symbol sId;  // 0x1ABE0C0, "StateTransition"
    // Also "StateTransition". Name not in the reference map.
    static Symbol sClassName;  // 0x1ABE0C8
    static PropRegistry sPropRegistry;  // 0x1ABE0D0
    static ComMetaData sMetaData;       // 0x1ABE170

    // Field names are not in the reference map; they follow the
    // properties.
    GameObjectId mOriginState;  // "origin_state"
    GameObjectId mTargetState;  // "target_state"
    GameObjectId mCondition;    // "condition"
    // "duration", in mUnits.
    DrivenProp<float> mDuration;
    // "units": the TimeUnits of the duration; 4 takes the context entity's
    // AnimDriverCom default timeline.
    int mUnits;
    // "blend_curve": the blend ratio over the duration.
    DrivenProp<Waveform<float>> mBlendCurve;
    bool mCanInterrupt;      // "can_interrupt"
    bool mCanBeInterrupted;  // "can_be_interrupted"
    bool mHsvColorBlending;  // "hsv_color_blending"
    int mBlendMode;          // "blend_mode" (BlendMode)
    // "rotation_blending": 0 Euler, 1 quaternion.
    int mRotationBlending;
    // "animation_blend_start_time", "animation_blend_duration" and
    // "animation_blend_offset", in timing positions, for a synchronized
    // blend.
    float mAnimationBlendStartTime;
    float mAnimationBlendDuration;
    float mAnimationBlendOffset;
    RuntimeData mRuntime;
};

static_assert(offsetof(StateTransition, mOriginState) == 32);
static_assert(offsetof(StateTransition, mCondition) == 40);
static_assert(offsetof(StateTransition, mDuration) == 48);
static_assert(offsetof(StateTransition, mUnits) == 280);
static_assert(offsetof(StateTransition, mBlendCurve) == 288);
static_assert(offsetof(StateTransition, mCanInterrupt) == 552);
static_assert(offsetof(StateTransition, mBlendMode) == 556);
static_assert(offsetof(StateTransition, mAnimationBlendStartTime) == 564);
static_assert(offsetof(StateTransition, mRuntime) == 576);
static_assert(sizeof(StateTransition) == 600);

// A transition running in a graph: how long it has run and, for a
// synchronized blend, where the timings stand. StateGraphInstance keeps
// them newest first, the transition at index i blending from node
// instance i + 1 into node instance i. The object is 32 bytes. Name and
// field names not in the reference map; the map's StateTransition::Poll(
// StateGraphInstance&) const is this build's Poll.
class StateTransitionInstance {
public:
    StateTransitionInstance(StateGraphInstance& graph, const StateTransition& transition);  // 0x7F8340

    // Called on each running transition before it is dropped; nothing
    // here. Name not in the reference map; the evidence is weak.
    void End();  // 0x7F8A20
    // Starts the blend from the origin's instance into the target's: a
    // synchronized blend moves the target to the offset and notes where
    // the origin stands. True when the transition takes time.
    bool Begin(StateNodeInstance* from, StateNodeInstance* to);  // 0x7F8360
    // Advances the transition and returns the blend into the target
    // through `blend`; false once it is complete.
    bool Poll(StateNodeInstance* from, StateNodeInstance* to, float* blend);  // 0x7F84B0

    StateGraphInstance* mGraph;
    const StateTransition* mTransition;
    // The time the transition has run, in its units.
    float mElapsed;
    // A synchronized blend's last origin position and how far it moved.
    float mOriginPosition;
    float mSyncProgress;
};

static_assert(offsetof(StateTransitionInstance, mElapsed) == 16);
static_assert(offsetof(StateTransitionInstance, mSyncProgress) == 24);
static_assert(sizeof(StateTransitionInstance) == 32);
