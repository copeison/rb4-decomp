#pragma once

#include <cstddef>

#include "entity/props/PropRef.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/nodes/StateNode.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The node that triggers an "action" property (stategraph/StateNodeTriggerAction.o,
// after the map's build, 0x7F2E70-0x7F3A17; class id
// "StateNodeTriggerAction"). "action" refers to an action property of a
// component in the context entity; the node runs its action on its first
// poll, or on every poll while it is active when it is "persistent". The
// vtable at 0x194E7B0 has 50 slots. The object is 376 bytes.
class StateNodeTriggerAction : public StateNode {
public:
    // The node's run-time state. The vtable at 0x194E950 has 15 slots. The
    // object is 40 bytes, from the small-block pool. Name not in the
    // reference map; it follows the other nodes' Instance.
    class Instance : public StateNodeInstance {
    public:
        POOL_OVERLOAD(Instance)

        // Also inlined into _CreateInstance (0x7F38B0).
        Instance(const StateNodeTriggerAction& node, StateGraphInstance& graph);  // 0x7F3460
        ~Instance() override;  // slots 0-1: 0x7F3910, 0x7F3920

        // Slot 5: runs the action when it resolves to an action property;
        // the node works for as long as it is persistent.
        bool _Poll() override;  // 0x7F3490

        // Name not in the reference map.
        const StateNodeTriggerAction* mTriggerNode;
    };

    StateNodeTriggerAction();  // 0x7F2E70
    // Copies the node for an imprint. Inlined into _Imprint (0x7F3680).
    StateNodeTriggerAction(const StateNodeTriggerAction& other)
        : StateNode(other), mAction(other.mAction), mPersistent(other.mPersistent) {}
    // Slots 0-1: 0x7F33A0, 0x7F3560.
    ~StateNodeTriggerAction() override {}

    Symbol GetId() const override;          // slot 4: 0x7F3600
    Symbol GetClassName() const override;   // slot 5: 0x7F3610
    int CurrentRev() const override;        // slot 7: 0x7F3620
    bool IsA(Symbol type) const override;   // slot 8: 0x7F3640
    Component* AsComponent() override;      // slot 9: 0x7F3670
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x7F3680
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x7F3890
    ComMetaData& _GetMetaData() override;       // slot 23: 0x7F38A0
    // Slot 42: the action's display name in the context entity.
    const char* GetEditorText() const override;  // 0x7F3430
    StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 45: 0x7F38B0

    // Registers the class (InitComponentClass). Emitted in
    // stategraph/StateGraph.o at 0x6FA820.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x700570.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x7F2EB0

    static Symbol sId;  // 0x1ABDC00, "StateNodeTriggerAction"
    // The class symbol GameObject::CreateComponent takes. Name not in the
    // reference map.
    static Symbol sClassName;  // 0x1ABDC08
    static PropRegistry sPropRegistry;  // 0x1ABDC10
    static ComMetaData sMetaData;       // 0x1ABDCB0

    // Field names are not in the reference map; they follow the
    // properties.
    // "action": the action property.
    PropRef mAction;
    // "persistent": trigger the action on every poll instead of once.
    bool mPersistent;
};

static_assert(offsetof(StateNodeTriggerAction, mAction) == 152);
static_assert(offsetof(StateNodeTriggerAction, mPersistent) == 368);
static_assert(sizeof(StateNodeTriggerAction) == 376);
static_assert(offsetof(StateNodeTriggerAction::Instance, mTriggerNode) == 32);
static_assert(sizeof(StateNodeTriggerAction::Instance) == 40);
