#pragma once

#include <cstddef>
#include <functional>

#include "entity/props/PropMetadata.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/conditions/StateConditionComparison.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The TimeUnits a state graph object's "units" property offers: the
// timelines of the prototype entity's AnimDriverCom, or none. The
// transitions, the sound nodes, ClipBlendTreeCom and the time elapsed
// condition copy it into their IntMetadata<int>::mAllowedValues. It is
// built by a static initializer of its own at 0x891950 (the lambda is at
// 0x891770), which suggests an object after StateConditionTimeElapsed.o
// that the map does not have, as does GetAnimTypeAllowedValues (0x891390).
// Both are defined in StateConditionTimeElapsed.cpp. Name not in the
// reference map.
extern std::function<eastl::vector<AllowedValue<int>>(const TagArgs&)> gTimeUnitsAllowedValues;  // 0x1AC7CC0

// The condition that compares the time elapsed in the transition's scope
// (stategraph/StateConditionTimeElapsed.o, 0x8903F0-0x891387; class id
// "StateConditionTimeElapsed"): since the start or the end of the
// transition into the node, in the chosen TimeUnits or the driver's
// default timeline. Only the ordering operators apply; it defaults to ">"
// with (a) and (b) at least 0. The vtable at 0x195BFC0 has 59 slots. The
// object is 520 bytes.
class StateConditionTimeElapsed : public StateConditionComparison<float> {
public:
    // "type": where the time is measured from. The labels are the allowed
    // values'; the enumerator names are not in the reference map.
    enum Since : int {
        kStartOfIncomingTransition = 0,
        kEndOfIncomingTransition = 1,
    };

    // The condition's run-time state: the elapsed time is the value to
    // compare. The map's StateConditionTimeElapsed::Instance. The vtable at
    // 0x195C1A8 has nine slots. The object is 56 bytes, from the
    // small-block pool.
    class Instance : public StateConditionComparison<float>::Instance {
    public:
        POOL_OVERLOAD(Instance)

        // Inlined into _CreateInstance (0x891080). The map's constructor
        // takes the node instance as well.
        Instance(const StateConditionTimeElapsed& condition, StateGraphInstance& graph);  // 0x890D60
        // Slots 0-1: 0x891170 (StateConditionInstance's destructor) and
        // 0x891180.
        ~Instance() override {}

        // Slot 8: the graph's elapsed time.
        bool _GetValueToCompare(float& value) const override;  // 0x890E10

        // Name not in the reference map.
        const StateConditionTimeElapsed* mTimeElapsed;
    };

    // Out of line; Init and _Create call it.
    StateConditionTimeElapsed();  // 0x8903F0
    // Copies the condition for an imprint. Inlined into _Imprint
    // (0x890F00).
    StateConditionTimeElapsed(const StateConditionTimeElapsed& other)
        : StateConditionComparison<float>(other), mUnits(other.mUnits), mSince(other.mSince) {}
    // Slots 0-1: 0x890C40 (Component's destructor) and 0x890E60.
    ~StateConditionTimeElapsed() override {}

    Symbol GetId() const override;          // slot 4: 0x890E80
    Symbol GetClassName() const override;   // slot 5: 0x890E90
    int CurrentRev() const override;        // slot 7: 0x890EA0
    bool IsA(Symbol type) const override;   // slot 8: 0x890EC0
    Component* AsComponent() override;      // slot 9: 0x890EF0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x890F00
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x891060
    ComMetaData& _GetMetaData() override;       // slot 23: 0x891070
    // Slot 42. The map's _CreateInstance(StateNodeInstance const&,
    // StateGraphInstance&) const.
    StateConditionInstance* _CreateInstance(StateGraphInstance& graph) const override;  // 0x891080
    // Slot 52: kTimeElapsed.
    int _GetType() const override;  // 0x891160
    // Slot 54: "elapsed time".
    const char* _GetValueToCompareGeneralDescription() const override;  // 0x890C50
    // Slot 55: "Elapsed <units>", the driver's default timeline for the
    // default units.
    const char* _GetValueToCompareDescription() const override;  // 0x890C60
    // Slot 57: the ordering operators and kBetween.
    bool _AllowOperator(ComparisonOperator op) const override;  // 0x890D50

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6FE050.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x701340.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x890480

    static Symbol sId;  // 0x1AC7A60, "StateConditionTimeElapsed"
    // Also "StateConditionTimeElapsed". The class symbol GameObject::
    // CreateComponent takes. Name not in the reference map.
    static Symbol sClassName;  // 0x1AC7A68
    static PropRegistry sPropRegistry;  // 0x1AC7A70
    static ComMetaData sMetaData;       // 0x1AC7B10

    // Field names are not in the reference map.
    // "units" (TimeUnits); kNumTimeUnits takes the driver's default
    // timeline. It sits in the comparison's tail padding.
    int mUnits;
    // "type" (Since).
    int mSince;
};

static_assert(offsetof(StateConditionTimeElapsed, mUnits) == 508);
static_assert(offsetof(StateConditionTimeElapsed, mSince) == 512);
static_assert(sizeof(StateConditionTimeElapsed) == 520);
static_assert(offsetof(StateConditionTimeElapsed::Instance, mTimeElapsed) == 48);
static_assert(sizeof(StateConditionTimeElapsed::Instance) == 56);
