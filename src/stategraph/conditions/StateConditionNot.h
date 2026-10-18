#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/conditions/StateCondition.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The condition met when its child condition is not
// (stategraph/StateConditionNot.o, 0x85B310-0x85BCC7; class id
// "StateConditionNot"). It holds one child, which may not be a parameter
// comparison; without a child it is never met. The vtable at 0x1957228 has
// 54 slots. The object is 48 bytes.
class StateConditionNot : public StateCondition {
public:
    // The child, resolved by _Fixup. The map has the type; the field name
    // is not in the reference map.
    struct RuntimeData {
        RuntimeData();  // 0x85B310

        StateCondition* mChildCondition;
    };

    // The condition's run-time state: the child's instance, whose result it
    // inverts. The map's StateConditionNot::Instance. The vtable at
    // 0x19571E0 has seven slots. The object is 48 bytes, from the
    // small-block pool.
    class Instance : public StateConditionInstance {
    public:
        POOL_OVERLOAD(Instance)

        // Inlined into _CreateInstance (0x85BB30).
        Instance(const StateConditionNot& condition, StateGraphInstance& graph);  // 0x85B7C0
        ~Instance() override;  // slots 0-1: 0x85B820, 0x85B860

        // Slot 2: the inverse of the child's result; a held child result
        // holds this one. Without a child the result is false and held.
        bool _IsMet(bool useCached) override;  // 0x85B8B0
        void _ResetPerFrameState() override;   // slot 3: 0x85B900
        bool _IsResultCurrent() const override;  // slot 5: 0x85BBD0
        bool _UsesObject(GameObjectId object) const override;  // slot 6: 0x85B910

        // Field names are not in the reference map.
        StateConditionInstance* mChild;
        // The child's IsResultCurrent at the last evaluation.
        bool mResultCurrent;
    };

    StateConditionNot() : mChild{0xFFFFFFFFU}, mRuntime() {}
    // Copies the condition for an imprint; the child starts unresolved.
    // Inlined into _Imprint (0x85B9D0).
    StateConditionNot(const StateConditionNot& other)
        : StateCondition(other), mChild(other.mChild), mRuntime() {}
    // Slots 0-1: 0x808F90 (Component's destructor) and 0x85B930.
    ~StateConditionNot() override {}

    Symbol GetId() const override;          // slot 4: 0x85B950
    Symbol GetClassName() const override;   // slot 5: 0x85B960
    int CurrentRev() const override;        // slot 7: 0x85B970
    bool IsA(Symbol type) const override;   // slot 8: 0x85B990
    Component* AsComponent() override;      // slot 9: 0x85B9C0
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x85B9D0
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x85BB10
    ComMetaData& _GetMetaData() override;       // slot 23: 0x85BB20
    // Slot 41: resolves the child.
    void _Fixup() override;  // 0x85B4F0
    StateConditionInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 42: 0x85BB30
    int _GetNumAllowedChildren() const override;  // slot 43: 0x85BBB0
    int _GetNumChildren() const override;         // slot 44: 0x85B5F0
    void _AddChild(GameObjectId child) override;     // slot 45: 0x85B600
    void _RemoveChild(GameObjectId child) override;  // slot 46: 0x85B660
    GameObjectId _GetChild(unsigned int index) const override;  // slot 47: 0x85B690
    // Slot 49: a parameter comparison may not be the child.
    void _GetDisallowedChildTypes(eastl::vector<int>& types) const override;  // 0x85B530
    // Slot 50: "Not".
    const char* _GetDescription() const override;  // 0x85B6A0
    void _GetChildren(eastl::vector<GameObjectId>& children, bool recursive) const override;  // slot 51: 0x85B6B0
    // Slot 52: kNot.
    int _GetType() const override;  // 0x85BBC0

    // Whether a child is set. No caller in this build. Name not in the
    // reference map.
    bool HasChildCondition() const;  // 0x85B520
    // Sets the child, as _AddChild does. No caller in this build. Name not
    // in the reference map.
    void SetChildCondition(GameObjectId child);  // 0x85B630

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6FD0E0.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x7011C0.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x85B320

    static Symbol sId;  // 0x1AC43E0, "StateConditionNot"
    // Also "StateConditionNot". Name not in the reference map.
    static Symbol sClassName;  // 0x1AC43E8
    static PropRegistry sPropRegistry;  // 0x1AC43F0
    static ComMetaData sMetaData;       // 0x1AC4490

    // "child_condition": the child's object. It sits in StateCondition's
    // tail padding. Name not in the reference map.
    GameObjectId mChild;
    RuntimeData mRuntime;
};

static_assert(offsetof(StateConditionNot, mChild) == 36);
static_assert(offsetof(StateConditionNot, mRuntime) == 40);
static_assert(sizeof(StateConditionNot) == 48);
static_assert(offsetof(StateConditionNot::Instance, mChild) == 32);
static_assert(offsetof(StateConditionNot::Instance, mResultCurrent) == 40);
static_assert(sizeof(StateConditionNot::Instance) == 48);
