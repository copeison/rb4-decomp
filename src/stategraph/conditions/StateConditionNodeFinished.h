#pragma once

#include <cstddef>

#include "os/memory/PoolAlloc.h"
#include "stategraph/conditions/StateCondition.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The condition met once the node of the transition's scope finished
// (stategraph/StateConditionNodeFinished.o, 0x85AE10-0x85B307; class id
// "StateConditionNodeFinished"). It adds no properties. The vtable at
// 0x1957020 has 54 slots. The object is 40 bytes.
class StateConditionNodeFinished : public StateCondition {
public:
    // The condition's run-time state: met, and held, once the scope's node
    // finished (StateGraphInstance::IsScopeNodeFinished). The vtable at
    // 0x1956FD8 has seven slots. The object is 32 bytes, from the
    // small-block pool.
    class Instance : public StateConditionInstance {
    public:
        POOL_OVERLOAD(Instance)

        Instance(const StateConditionNodeFinished& condition, StateGraphInstance& graph);  // 0x85AF60
        // Slots 0-1: 0x85B210 (the base destructor) and 0x85B220.
        ~Instance() override {}

        bool _IsMet(bool useCached) override;  // slot 2: 0x85AF90
    };

    StateConditionNodeFinished() {}
    // Copies the condition for an imprint. Inlined into _Imprint (0x85B060).
    StateConditionNodeFinished(const StateConditionNodeFinished& other) : StateCondition(other) {}
    // Slots 0-1: 0x808F90 (Component's destructor) and 0x85AFC0.
    ~StateConditionNodeFinished() override {}

    Symbol GetId() const override;          // slot 4: 0x85AFE0
    Symbol GetClassName() const override;   // slot 5: 0x85AFF0
    int CurrentRev() const override;        // slot 7: 0x85B000
    bool IsA(Symbol type) const override;   // slot 8: 0x85B020
    Component* AsComponent() override;      // slot 9: 0x85B050
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x85B060
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x85B190
    ComMetaData& _GetMetaData() override;       // slot 23: 0x85B1A0
    StateConditionInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 42: 0x85B1B0
    // Slot 50: "Is finished...".
    const char* _GetDescription() const override;  // 0x85AF50
    // Slot 52: kNodeFinished.
    int _GetType() const override;  // 0x85B200

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6FCD20.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x701150.
    static Component* _Create();
    // The description is the Not condition's, as in the binary.
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x85AE10

    static Symbol sId;  // 0x1AC4180, "StateConditionNodeFinished"
    // Also "StateConditionNodeFinished". Name not in the reference map.
    static Symbol sClassName;  // 0x1AC4188
    static PropRegistry sPropRegistry;  // 0x1AC4190
    static ComMetaData sMetaData;       // 0x1AC4230
};

static_assert(sizeof(StateConditionNodeFinished) == 40);
static_assert(sizeof(StateConditionNodeFinished::Instance) == 32);
