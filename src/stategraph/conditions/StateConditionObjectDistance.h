#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/conditions/StateConditionComparison.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The condition that compares the distance between two objects of the
// graph's entity (stategraph/StateConditionObjectDistance.o,
// 0x85BCD0-0x85D627; class id "StateConditionObjectDistance", newer than
// the map's build): the distance between the world positions of their
// TransComs. Each comparison value may instead be the distance when the
// node was entered or the most recent one, as the property comparisons
// offer. The vtable at 0x19573E8 has 59 slots. The object is 528 bytes.
// The class is not in the reference map: the names of its members follow
// the class id, the property names and StateConditionPropertyComparison<T>.
class StateConditionObjectDistance : public StateConditionComparison<float> {
public:
    // What a comparison value compares against ("comparison_mode_a" and
    // "_b"). The labels are the allowed values', as the property
    // comparisons'; the enumerator names are not in the reference map.
    enum ComparisonMode : int {
        kSpecificValue = 0,
        kInitialValue = 1,
        kMostRecentValue = 2,
    };

    // The condition's run-time state: the distance when the instance was
    // created and the one measured at the last frame reset. The vtable at
    // 0x19575D0 has nine slots. The object is 64 bytes, from the
    // small-block pool. Name not in the reference map, as for the class.
    class Instance : public StateConditionComparison<float>::Instance {
    public:
        POOL_OVERLOAD(Instance)

        // Inlined into _CreateInstance (0x85CFD0).
        Instance(const StateConditionObjectDistance& condition, StateGraphInstance& graph);  // 0x85CAD0
        // Slots 0-1: 0x85D0D0 (StateConditionInstance's destructor) and
        // 0x85D0E0.
        ~Instance() override {}

        // Slot 3: measures the most recent distance.
        void _ResetPerFrameState() override;  // 0x85CDA0
        // Slot 7: the initial or most recent distance, as the mode asks.
        bool _OverrideComparisonValue(unsigned long index, float& value) const override;  // 0x85CD70
        // Slot 8: the distance, or false without both objects and their
        // TransComs.
        bool _GetValueToCompare(float& value) const override;  // 0x85CB90

        // Field names are not in the reference map.
        const StateConditionObjectDistance* mObjectDistance;
        float mInitialDistance;
        float mRecentDistance;
    };

    // Out of line; Init and _Create call it.
    StateConditionObjectDistance();  // 0x85BCD0
    // Copies the condition for an imprint. Inlined into _Imprint
    // (0x85CE50).
    StateConditionObjectDistance(const StateConditionObjectDistance& other)
        : StateConditionComparison<float>(other),
          mObjA(other.mObjA),
          mObjB(other.mObjB),
          mComparisonModes{other.mComparisonModes[0], other.mComparisonModes[1]} {}
    // Slots 0-1: 0x85C9D0 (Component's destructor) and 0x85CDB0.
    ~StateConditionObjectDistance() override {}

    Symbol GetId() const override;          // slot 4: 0x85CDD0
    Symbol GetClassName() const override;   // slot 5: 0x85CDE0
    int CurrentRev() const override;        // slot 7: 0x85CDF0
    bool IsA(Symbol type) const override;   // slot 8: 0x85CE10
    Component* AsComponent() override;      // slot 9: 0x85CE40
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x85CE50
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x85CFB0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x85CFC0
    StateConditionInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 42: 0x85CFD0
    // Slot 52: kDistanceCheck.
    int _GetType() const override;  // 0x85D0C0
    // Slot 54: "distance".
    const char* _GetValueToCompareGeneralDescription() const override;  // 0x85C9E0
    // Slot 55: "distance((<a>)->(<b>))" with the objects' serials.
    const char* _GetValueToCompareDescription() const override;  // 0x85C9F0
    // Slot 56: both objects are set.
    bool _HasValueToCompare() const override;  // 0x85CA80
    // Slot 58: "initial value" or "most recent value" for those modes.
    const char* _GetComparisonValueDescriptionOverride(unsigned long index, Symbol name) const override;  // 0x85CAA0

    // Registers the class (InitComponentClass). This build emits it in
    // stategraph/StateGraph.o at 0x6FD4B0.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x701250.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x85BD50

    static Symbol sId;  // 0x1AC4640, "StateConditionObjectDistance"
    // Also "StateConditionObjectDistance". The class symbol GameObject::
    // CreateComponent takes.
    static Symbol sClassName;  // 0x1AC4648
    static PropRegistry sPropRegistry;  // 0x1AC4650
    static ComMetaData sMetaData;       // 0x1AC46F0

    // "obj_a" and "obj_b". The first sits in the comparison's tail padding.
    GameObjectId mObjA;
    GameObjectId mObjB;
    // "comparison_mode_a" and "comparison_mode_b" (ComparisonMode).
    int mComparisonModes[2];
};

static_assert(offsetof(StateConditionObjectDistance, mObjA) == 508);
static_assert(offsetof(StateConditionObjectDistance, mObjB) == 512);
static_assert(offsetof(StateConditionObjectDistance, mComparisonModes) == 516);
static_assert(sizeof(StateConditionObjectDistance) == 528);
static_assert(offsetof(StateConditionObjectDistance::Instance, mObjectDistance) == 48);
static_assert(offsetof(StateConditionObjectDistance::Instance, mInitialDistance) == 56);
static_assert(offsetof(StateConditionObjectDistance::Instance, mRecentDistance) == 60);
static_assert(sizeof(StateConditionObjectDistance::Instance) == 64);
