#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

class AudioClipCom;
class Entity;
class StateGraphInstance;

// The node that triggers an AudioClipCom (stategraph/StateNodeTriggerAudioClip.o,
// 0x7F3A20-0x7F5216, after the map's build; class id
// "StateNodeTriggerAudioClip"). It triggers the clip on entering and
// finishes at once, when the clip starts or when it ends; leaving before
// then may stop the clip. The vtable at 0x194E9D8 has 50 slots. The object
// is 168 bytes.
class StateNodeTriggerAudioClip : public StateNode {
public:
    // When the node finishes ("finish_mode"). The labels are the allowed
    // values'; the enumerator names are not in the reference map.
    enum FinishMode : int {
        kFinishImmediately = 0,
        kFinishOnStart = 1,
        kFinishOnEnd = 2,
    };

    // The node's run-time state, named like the other nodes' instances;
    // the class is newer than the reference map. The vtable at 0x194EB78 has 15 slots. The object is 48 bytes,
    // from the small-block pool.
    class Instance : public StateNodeInstance {
    public:
        POOL_OVERLOAD(Instance)

        // Inlined into _CreateInstance (0x7F4CD0).
        Instance(const StateNodeTriggerAudioClip& node, StateGraphInstance& graph)
            : StateNodeInstance(node, graph), mTriggerAudioClip(&node), mFinished(false) {}
        // Slots 0-1 at 0x7F4D30 (StateNodeInstance's destructor) and
        // 0x7F4D40.
        ~Instance() override {}

        // Slot 2: triggers the clip.
        void _Enter() override;  // 0x7F44F0
        // Slot 5: false once the finish mode is met.
        bool _Poll() override;  // 0x7F45B0
        // Slot 6: stops a clip the node did not wait for, when asked to.
        void _Exit() override;  // 0x7F4900

        // Whether the clip started: true without a clip or when it does not
        // wait for a start; false without an entity. Name not in the
        // reference map.
        bool _IsClipStarted() const;  // 0x7F4770
        // Whether the clip is done: true without a clip; false without an
        // entity. Name not in the reference map.
        bool _IsClipDone() const;  // 0x7F4840

        // Field names are not in the reference map.
        const StateNodeTriggerAudioClip* mTriggerAudioClip;
        // Set once the finish mode was met.
        bool mFinished;
    };

    StateNodeTriggerAudioClip();  // 0x7F3A20
    // Copies the node for an imprint. Inlined into _Imprint (0x7F4AF0).
    StateNodeTriggerAudioClip(const StateNodeTriggerAudioClip& other)
        : StateNode(other),
          mAudioClip(other.mAudioClip),
          mFinishMode(other.mFinishMode),
          mStopOnEarlyExit(other.mStopOnEarlyExit) {}
    // Slots 0-1: 0x7F4150, 0x7F49D0.
    ~StateNodeTriggerAudioClip() override {}

    Symbol GetId() const override;          // slot 4: 0x7F4A70
    Symbol GetClassName() const override;   // slot 5: 0x7F4A80
    int CurrentRev() const override;        // slot 7: 0x7F4A90
    bool IsA(Symbol type) const override;   // slot 8: 0x7F4AB0
    Component* AsComponent() override;      // slot 9: 0x7F4AE0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x7F4AF0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x7F4CB0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x7F4CC0
    // Slot 42: the clip's description, or "(none)".
    const char* GetEditorText() const override;  // 0x7F41E0
    StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 45: 0x7F4CD0

    // The AudioClipCom of the "audio_clip" object in the entity, or null.
    // Inlined by every user. Name not in the reference map.
    AudioClipCom* FindAudioClip(Entity* entity) const;

    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6FAC90.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x7005C0.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x7F3A60

    static Symbol sId;  // 0x1ABDE60, "StateNodeTriggerAudioClip"
    // Also "StateNodeTriggerAudioClip". Name not in the reference map.
    static Symbol sClassName;  // 0x1ABDE68
    static PropRegistry sPropRegistry;  // 0x1ABDE70
    static ComMetaData sMetaData;       // 0x1ABDF10

    // Field names are not in the reference map; they follow the
    // properties.
    GameObjectId mAudioClip;  // "audio_clip"
    int mFinishMode;          // "finish_mode" (FinishMode)
    bool mStopOnEarlyExit;    // "stop_on_early_exit"
};

static_assert(offsetof(StateNodeTriggerAudioClip, mAudioClip) == 152);
static_assert(offsetof(StateNodeTriggerAudioClip, mFinishMode) == 156);
static_assert(offsetof(StateNodeTriggerAudioClip, mStopOnEarlyExit) == 160);
static_assert(sizeof(StateNodeTriggerAudioClip) == 168);
static_assert(offsetof(StateNodeTriggerAudioClip::Instance, mTriggerAudioClip) == 32);
static_assert(offsetof(StateNodeTriggerAudioClip::Instance, mFinished) == 40);
static_assert(sizeof(StateNodeTriggerAudioClip::Instance) == 48);
