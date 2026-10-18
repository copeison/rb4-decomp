#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "entity/resources/Resource.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/transitions/StateTransition.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/time/TimeMgr.h"

class AnimDriverCom;
class DataArray;
class Entity;
class StateConditionInstance;
class StateGraphResource;
class StateNode;
class StateNodeInstance;

// A state graph running for an entity (stategraph/StateGraphInstance.o,
// 0x703CC0-0x706C38). It keeps the instances of the nodes that run, newest
// first, with the transitions blending between them, and one scope for
// each node from the outermost folder to the current node: the
// transitions that may leave it, their condition instances and the time
// spent in it. A StateGraphDriverCom owns and polls the instances; the
// entity's sinks tell it when graph objects are destroyed. The vtable at
// 0x193A670 has four slots. The object is 184 bytes, from the small-block
// pool.
class StateGraphInstance : public MsgSink {
public:
    POOL_OVERLOAD(StateGraphInstance)

    // Which frames a graph that polls every other frame polls on (the
    // driver's phases). Names not in the reference map.
    enum PollPhase : int {
        kPollEveryFrame = 0,
        kPollEvenFrames = 1,
        kPollOddFrames = 2,
    };

    // The clocks of a scope, by TimeUnits: since the transition into the
    // scope's node began, and since it ended. Names not in the reference
    // map.
    enum ScopeClock : unsigned int {
        kSinceIncomingStart = 0,
        kSinceIncomingEnd = 1,
    };

    // A transition that may leave a scope's node, with its condition's
    // instance. Name and field names not in the reference map.
    struct TransitionEntry {
        explicit TransitionEntry(StateTransition* transition);  // 0x704000
        // Inlined into _PushScope. Name not in the reference map.
        TransitionEntry(StateTransition* transition, StateConditionInstance* condition, bool synchronized)
            : mTransition(transition), mCondition(condition), mSynchronized(synchronized) {}

        StateTransition* mTransition;
        // Null for a transition without a condition.
        StateConditionInstance* mCondition;
        // The transition's IsSynchronized when the scope began.
        bool mSynchronized;
    };

    // One node of the path from the outermost folder to the current node.
    // Name and field names not in the reference map.
    struct TransitionScope {
        explicit TransitionScope(const StateNode* node);  // 0x704030

        const StateNode* mNode;
        eastl::vector<TransitionEntry> mTransitions;
        // The time in the scope by ScopeClock and TimeUnits.
        float mElapsed[2][4];
        // Set once the transition into the scope's node ended.
        bool mIncomingEnded;
        // Set once the node finished and none of its transitions could
        // leave it; StateConditionNodeFinished reads it.
        bool mNodeFinished;
    };

    // The graph at the path in the entity, inline in the entity's resource.
    // The map's StateGraphInstance(EntityPtr const&, ResourcePath); this
    // build adds the clock.
    StateGraphInstance(Entity* entity, TimeMgr::Clock* clock, ResourcePath path);  // 0x703CC0
    // Slots 0-1: 0x703EB0, 0x703FD0.
    ~StateGraphInstance() override;
    // Slot 2: deactivates the graph when the entity is destroyed, or when an
    // object or graph component it runs or depends on is.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0x705FD0

    // Starts the graph at its root node, at the node on the object, or at
    // the transition on the object after entering and polling its origin.
    void Activate(GameObjectId object);  // 0x704080
    // Enters the node: rebuilds the scopes down to it, follows folders to
    // their start nodes and the transitions that leave them at once, and
    // adds the node's instance as the newest. Name not in the reference map.
    StateNodeInstance* _EnterNode(const StateNode* node);  // 0x704290
    // Marks the scopes from the innermost out as past their incoming
    // transitions, stopping at the first that is. Inlined into Activate,
    // _ActivateTransition and Poll. Name not in the reference map.
    void _EndIncomingTransitions();  // 0x7043D0
    // Starts the transition into its target: a transition that takes no
    // time drops every other node at once.
    void _ActivateTransition(StateTransition* transition);  // 0x704450
    // Polls the graph through _Poll.
    bool Poll();  // 0x7046F0
    // Polls the graph, looking up its PerfTimerMgr timer first. No caller
    // in this build. Name not in the reference map.
    bool PollTimed();  // 0x704700
    // Advances the clocks, the transitions and the nodes, then takes the
    // transitions whose conditions are met, at most a thousand in a row and
    // never more than 32 node instances. False once no node runs. Name not
    // in the reference map.
    bool _Poll();  // 0x704730
    // Adds the frame's time to every scope's clocks. The map has
    // _ResetElapsedTime(); this build only advances them. Name not in the
    // reference map.
    void _UpdateElapsedTime();  // 0x704C50
    // Exits every node and drops the transitions and scopes.
    void Deactivate();  // 0x704D80
    // The first transition, from the scope at the index outwards, that may
    // be taken: one whose condition is met and, when synchronized, whose
    // start position the current node's timing just crossed. Only
    // interruptible ones while `interrupting`; an unconditional one is the
    // fallback otherwise. Name not in the reference map.
    StateTransition* _FindTransition(unsigned long scope, bool interrupting, bool entering);  // 0x704E10
    // Drops every scope. No caller in this build. Name not in the
    // reference map.
    void _ClearScopes();  // 0x703FC0
    // The graph resource. Name not in the reference map.
    StateGraphResource* GetResource() const;  // 0x705170
    // The current scope's time on the clock in the units. Name not in the
    // reference map.
    float GetElapsedTime(unsigned int clock, int units) const;  // 0x705180
    // Drops the scopes from the index on, with their condition instances.
    // Name not in the reference map.
    void _PopScopes(unsigned long index);  // 0x7051B0
    // Matches the scopes to the node's folders and the node, keeping the
    // ones that match and replacing the rest; the depth of the node's
    // scope plus one. Name not in the reference map.
    int _PushScopes(const StateNode* node);  // 0x705300
    // Adds the node's scope with its transitions. Name not in the
    // reference map.
    void _PushScope(const StateNode* node);  // 0x7053C0
    // Drops the innermost scope. No caller in this build. Name not in the
    // reference map.
    void _PopScope();  // 0x705760
    // Whether the current scope's node finished. Name not in the reference
    // map.
    bool IsScopeNodeFinished() const;  // 0x7058B0
    // A random value in [0, 1) drawn once per transition search.
    float GetRandom();  // 0x7058D0
    // Whether the node being polled blends in or out: two or more nodes
    // run, and it is not, or is, the oldest. Names not in the reference
    // map.
    bool _IsBlendingIn() const;   // 0x705930
    bool _IsBlendingOut() const;  // 0x705950
    // The transition the node being polled blends through: into it, or out
    // of the oldest node. Names not in the reference map.
    bool UseHSVColorBlending() const;  // 0x705970
    const StateTransitionInstance* _GetPolledTransition() const;  // 0x7059A0
    int GetRotationBlending() const;  // 0x7059D0
    // The first node instance that answers for the object
    // (StateNodeInstance::FindInstance). Name not in the reference map.
    StateNodeInstance* FindNodeInstance(const GameObject* object);  // 0x705A00
    // The units with TimeUnits 4 standing for the anim driver's default
    // timeline. Name not in the reference map.
    int ResolveTimeUnits(int units) const;  // 0x705A50
    // Whether the graph polls on the driver's poll count. Name not in the
    // reference map.
    bool ShouldPoll(unsigned long count) const;  // 0x705A60
    // The message handlers inlined into Handle.
    DataNode _OnEntityDestroyed(const DataArray* msg);     // 0x705A80
    DataNode _OnObjectDestroyed(const DataArray* msg);     // 0x705B30
    // Deactivates the graph when a node instance, a running or possible
    // transition or a condition uses the object.
    void _DeactivateIfActive(GameObjectId object);         // 0x705B70
    DataNode _OnComponentDestroyed(const DataArray* msg);  // 0x705F30

    // Field names are not in the reference map.
    Entity* mEntity;
    // The entity root's AnimDriverCom, for the default timeline.
    AnimDriverCom* mAnimDriver;
    ResourcePtr<StateGraphResource> mResource;
    // GetRandom's value; negative until drawn.
    float mRandom;
    // The running transitions, newest first.
    eastl::vector<StateTransitionInstance> mTransitions;
    // The node instances, newest first.
    eastl::vector<StateNodeInstance*> mNodeInstances;
    // The node instance being polled, or -1.
    unsigned long mPollIndex;
    // A PerfTimerMgr timer for PollTimed, or -1.
    unsigned long mPerfTimer;
    eastl::vector<TransitionScope> mScopes;
    // The scope being searched or built, or -1.
    unsigned long mScopeIndex;
    // The current node's timing position at the last search.
    float mTimingPosition;
    // Set until the first poll, which does not advance the clocks.
    bool mSkipTimeUpdate;
    // A PollPhase.
    int mPollPhase;
    TimeMgr::Clock* mClock;
};

static_assert(sizeof(StateGraphInstance::TransitionEntry) == 24);
static_assert(offsetof(StateGraphInstance::TransitionScope, mElapsed) == 40);
static_assert(offsetof(StateGraphInstance::TransitionScope, mIncomingEnded) == 72);
static_assert(sizeof(StateGraphInstance::TransitionScope) == 80);
static_assert(offsetof(StateGraphInstance, mEntity) == 8);
static_assert(offsetof(StateGraphInstance, mResource) == 24);
static_assert(offsetof(StateGraphInstance, mTransitions) == 40);
static_assert(offsetof(StateGraphInstance, mNodeInstances) == 72);
static_assert(offsetof(StateGraphInstance, mPollIndex) == 104);
static_assert(offsetof(StateGraphInstance, mScopes) == 120);
static_assert(offsetof(StateGraphInstance, mScopeIndex) == 152);
static_assert(offsetof(StateGraphInstance, mTimingPosition) == 160);
static_assert(offsetof(StateGraphInstance, mSkipTimeUpdate) == 164);
static_assert(offsetof(StateGraphInstance, mPollPhase) == 168);
static_assert(offsetof(StateGraphInstance, mClock) == 176);
static_assert(sizeof(StateGraphInstance) == 184);
