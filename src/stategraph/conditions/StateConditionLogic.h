#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/conditions/StateCondition.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The condition that combines its child conditions: met when all of them
// are (kAllOf) or when any of them is (kAnyOf)
// (stategraph/StateConditionLogic.o, 0x859690-0x85AE07; class id
// "StateConditionLogic"). StateCondition::Create makes both logical types
// with it and sets the operator. The children may not be parameter
// comparisons. The vtable at 0x1956DD0 has 54 slots. The object is 112
// bytes.
class StateConditionLogic : public StateCondition {
public:
    // The resolved children, in the order of mChildren. The map has the
    // type; the field name is not in the reference map.
    struct RuntimeData {
        RuntimeData();  // 0x859710

        eastl::vector<StateCondition*> mChildConditions;
    };

    // The condition's run-time state: an instance of each child. The map's
    // StateConditionLogic::Instance. The vtable at 0x1956F90 has seven
    // slots. The object is 80 bytes, from the small-block pool.
    class Instance : public StateConditionInstance {
    public:
        POOL_OVERLOAD(Instance)

        // Inlined into _CreateInstance (0x85AAB0).
        Instance(const StateConditionLogic& condition, StateGraphInstance& graph);  // 0x85A310
        ~Instance() override;  // slots 0-1: 0x85A540, 0x85A5C0

        // Slot 2: evaluates the children in order. The first child whose
        // result decides the operator decides it; the children after it
        // are evaluated with `useCached`, and a held deciding child holds
        // the result at once. Without children the result is false and
        // held; it is held as well once every child's is.
        bool _IsMet(bool useCached) override;  // 0x85A650
        void _ResetPerFrameState() override;     // slot 3: 0x85A750
        bool _IsResultCurrent() const override;  // slot 5: 0x85AB10
        bool _UsesObject(GameObjectId object) const override;  // slot 6: 0x85A790

        // Field names are not in the reference map.
        const StateConditionLogic* mLogic;
        eastl::vector<StateConditionInstance*> mChildren;
        // Whether every child not held had a current result at the last
        // evaluation.
        bool mResultCurrent;
    };

    // Inlined into the factory (0x701100).
    StateConditionLogic();  // 0x859690
    // Copies the condition for an imprint; the children start unresolved.
    // Inlined into _Imprint (0x85A8F0).
    StateConditionLogic(const StateConditionLogic& other)
        : StateCondition(other), mOperator(other.mOperator), mChildren(), mRuntime() {
        mChildren._Copy(other.mChildren);
    }
    // Slots 0-1: 0x859C90, 0x85A7E0.
    ~StateConditionLogic() override;

    Symbol GetId() const override;          // slot 4: 0x85A870
    Symbol GetClassName() const override;   // slot 5: 0x85A880
    int CurrentRev() const override;        // slot 7: 0x85A890
    bool IsA(Symbol type) const override;   // slot 8: 0x85A8B0
    Component* AsComponent() override;      // slot 9: 0x85A8E0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x85A8F0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x85AA90
    ComMetaData& _GetMetaData() override;       // slot 23: 0x85AAA0
    // Slot 41: resolves the children.
    void _Fixup() override;  // 0x859D20
    StateConditionInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 42: 0x85AAB0
    // Slot 43: any number of children (-1).
    int _GetNumAllowedChildren() const override;  // 0x85AAF0
    int _GetNumChildren() const override;         // slot 44: 0x859EA0
    // Slot 45: appends the child unless it is one already.
    void _AddChild(GameObjectId child) override;     // 0x859EB0
    void _RemoveChild(GameObjectId child) override;  // slot 46: 0x859FF0
    GameObjectId _GetChild(unsigned int index) const override;  // slot 47: 0x85A0C0
    // Slot 49: a parameter comparison may not be a child.
    void _GetDisallowedChildTypes(eastl::vector<int>& types) const override;  // 0x859DE0
    // Slot 50: "All of the following..." or "Any of the following...".
    const char* _GetDescription() const override;  // 0x85A0D0
    void _GetChildren(eastl::vector<GameObjectId>& children, bool recursive) const override;  // slot 51: 0x85A0F0
    // Slot 52: the operator.
    int _GetType() const override;  // 0x85AB00

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6FC8C0.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x701100.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x859730

    static Symbol sId;  // 0x1AC3F20, "StateConditionLogic"
    // Also "StateConditionLogic". Name not in the reference map.
    static Symbol sClassName;  // 0x1AC3F28
    static PropRegistry sPropRegistry;  // 0x1AC3F30
    static ComMetaData sMetaData;       // 0x1AC3FD0

    // "operator": kAllOf (the default) or kAnyOf. It sits in
    // StateCondition's tail padding. Name not in the reference map.
    int mOperator;
    // "child_conditions" (hidden): the children's objects. Name not in the
    // reference map.
    PropArray<GameObjectId> mChildren;
    RuntimeData mRuntime;
};

static_assert(offsetof(StateConditionLogic, mOperator) == 36);
static_assert(offsetof(StateConditionLogic, mChildren) == 40);
static_assert(offsetof(StateConditionLogic, mRuntime) == 80);
static_assert(sizeof(StateConditionLogic) == 112);
static_assert(offsetof(StateConditionLogic::Instance, mLogic) == 32);
static_assert(offsetof(StateConditionLogic::Instance, mChildren) == 40);
static_assert(offsetof(StateConditionLogic::Instance, mResultCurrent) == 72);
static_assert(sizeof(StateConditionLogic::Instance) == 80);
