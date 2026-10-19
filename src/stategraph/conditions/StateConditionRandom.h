#pragma once

#include <cstddef>

#include "os/memory/PoolAlloc.h"
#include "stategraph/conditions/StateConditionComparison.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The condition that compares a random number (stategraph/
// StateConditionRandom.o, 0x88E540-0x88F1A7; class id
// "StateConditionRandom"): the graph's random draw in [0, 1). Only the
// ordering operators apply; it defaults to "value < 0.5", with (a) and
// (b) in [0, 1]. The vtable at 0x195BAA0 has 59 slots. The object is 512
// bytes.
class StateConditionRandom : public StateConditionComparison<float> {
public:
    // The condition's run-time state: the graph's random number is the
    // value to compare. The map's StateConditionRandom::Instance. The
    // vtable at 0x195BC88 has nine slots. The object is 48 bytes, from the
    // small-block pool.
    class Instance : public StateConditionComparison<float>::Instance {
    public:
        POOL_OVERLOAD(Instance)

        // Inlined into _CreateInstance (0x88EEB0). The map's constructor
        // takes the node instance as well.
        Instance(const StateConditionRandom& condition, StateGraphInstance& graph);  // 0x88EBD0
        // Slots 0-1: 0x88EFA0 (StateConditionInstance's destructor) and
        // 0x88EFB0.
        ~Instance() override {}

        // Slot 8: StateGraphInstance::GetRandom.
        bool _GetValueToCompare(float& value) const override;  // 0x88EC80
    };

    // Out of line; Init and _Create call it.
    StateConditionRandom();  // 0x88E540
    // Copies the condition for an imprint. Inlined into _Imprint
    // (0x88ED40).
    StateConditionRandom(const StateConditionRandom& other) : StateConditionComparison<float>(other) {}
    // Slots 0-1: 0x88EB90 (Component's destructor) and 0x88ECA0.
    ~StateConditionRandom() override {}

    Symbol GetId() const override;          // slot 4: 0x88ECC0
    Symbol GetClassName() const override;   // slot 5: 0x88ECD0
    int CurrentRev() const override;        // slot 7: 0x88ECE0
    bool IsA(Symbol type) const override;   // slot 8: 0x88ED00
    Component* AsComponent() override;      // slot 9: 0x88ED30
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x88ED40
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x88EE90
    ComMetaData& _GetMetaData() override;       // slot 23: 0x88EEA0
    // Slot 42. The map's _CreateInstance(StateNodeInstance const&,
    // StateGraphInstance&) const.
    StateConditionInstance* _CreateInstance(StateGraphInstance& graph) const override;  // 0x88EEB0
    // Slot 52: kRandom.
    int _GetType() const override;  // 0x88EF90
    // Slot 54: "randomly generated value".
    const char* _GetValueToCompareGeneralDescription() const override;  // 0x88EBA0
    // Slot 55: "Random value".
    const char* _GetValueToCompareDescription() const override;  // 0x88EBB0
    // Slot 57: the ordering operators and kBetween.
    bool _AllowOperator(ComparisonOperator op) const override;  // 0x88EBC0

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6FD890.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x7012A0.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x88E660

    static Symbol sId;  // 0x1AC75A0, "StateConditionRandom"
    // Also "StateConditionRandom". The class symbol GameObject::
    // CreateComponent takes. Name not in the reference map.
    static Symbol sClassName;  // 0x1AC75A8
    static PropRegistry sPropRegistry;  // 0x1AC75B0
    static ComMetaData sMetaData;       // 0x1AC7650
};

static_assert(sizeof(StateConditionRandom) == 512);
static_assert(sizeof(StateConditionRandom::Instance) == 48);
