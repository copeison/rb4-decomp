#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "stategraph/base/StateBase.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class DataArray;
class Entity;
class StateConditionInstance;
class StateGraphInstance;

// A condition of a state graph transition (stategraph/StateCondition.o,
// 0x808AE0-0x809A92; class id "StateCondition"). Conditions are components
// on their own objects; the logical ones hold other conditions as their
// children. A running graph evaluates each through its
// StateConditionInstance. The vtable at 0x1950818 has 54 slots. The object
// is 40 bytes.
class StateCondition : public StateBase {
public:
    // The condition classes Create makes, named by GetTypeName. The map
    // names the type; the enumerator names follow the type names.
    enum Type : int {
        kTimeElapsed = 0,
        kNodeFinished = 1,
        kAllOf = 2,
        kAnyOf = 3,
        kNot = 4,
        kPropertyComparison = 5,
        kRandom = 6,
        kEvent = 7,
        kParameterComparison = 8,
        kDistanceCheck = 9,
        kSoundProgress = 10,
        kNumTypes = 11,
    };

    // How long a result holds ("evaluation_mode"). The labels are the
    // allowed values'; the enumerator names are not in the reference map.
    enum EvaluationMode : int {
        kReevaluateEveryFrame = 0,
        kEvaluateExactlyOnce = 1,
        kOnceTrueRemainTrue = 2,
        kOnceFalseRemainFalse = 3,
    };

    StateCondition();  // 0x808AE0
    // Copies the condition for an imprint. Inlined into _Imprint (0x809620).
    StateCondition(const StateCondition& other)
        : StateBase(other), mEvaluationMode(other.mEvaluationMode) {}
    // Slots 0-1: 0x808F90 (Component's destructor) and 0x809580.
    ~StateCondition() override {}

    Symbol GetId() const override;          // slot 4: 0x8095A0
    Symbol GetClassName() const override;   // slot 5: 0x8095B0
    int CurrentRev() const override;        // slot 7: 0x8095C0
    bool IsA(Symbol type) const override;   // slot 8: 0x8095E0
    Component* AsComponent() override;      // slot 9: 0x809610
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x809620
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x809750
    ComMetaData& _GetMetaData() override;       // slot 23: 0x809760

    // Slot 42: a new instance of the condition for the graph; none here.
    // The map's _CreateInstance(StateNodeInstance const&,
    // StateGraphInstance&) const; this build drops the node instance.
    virtual StateConditionInstance* _CreateInstance(StateGraphInstance& graph) const;  // 0x809400
    // Slots 43-44: how many children the condition may hold (-1 for any
    // number) and holds. None here.
    virtual int _GetNumAllowedChildren() const;  // 0x809770
    virtual int _GetNumChildren() const;         // 0x809780
    // Slots 45-47: add, remove and return a child. The base reports that
    // the condition has no children; the reports are compiled out.
    virtual void _AddChild(GameObjectId child);                // 0x809790
    virtual void _RemoveChild(GameObjectId child);             // 0x8097A0
    virtual GameObjectId _GetChild(unsigned int index) const;  // 0x8097B0
    // Slot 48 at 0x8097D0: called before the allowed child types are
    // gathered; nothing here, and no class overrides it. Name not in the
    // reference map; the evidence is weak.
    virtual void _PrepareAllowedChildTypes() {}
    // Slot 49 at 0x8097E0: the condition types the condition may not hold
    // as children; none here. Name not in the reference map.
    virtual void _GetDisallowedChildTypes(eastl::vector<int>& types) const {
        static_cast<void>(types);
    }
    // Slot 50: the text the editor shows on the condition.
    virtual const char* _GetDescription() const;  // 0x8097F0
    // Slot 51 at 0x809870: appends the objects of the child conditions,
    // and of theirs when `recursive` is set; nothing here. Name not in the
    // reference map.
    virtual void _GetChildren(eastl::vector<GameObjectId>& children, bool recursive) const {
        static_cast<void>(children);
        static_cast<void>(recursive);
    }
    // Slot 52: the condition's Type; kNumTypes here. Name not in the
    // reference map.
    virtual int _GetType() const;  // 0x809410
    // Slot 53: true here, and no class overrides it. Name not in the
    // reference map; the evidence is weak.
    virtual bool _IsSelectable() const;  // 0x809880

    // Creates the component of the type's class on the object; the
    // logical types share a class.
    static StateCondition* Create(GameObject* object, Type type);  // 0x808FA0
    // The editor name of the type, or empty.
    static const char* GetTypeName(Type type);  // 0x809040
    // A new instance for the graph (slot 42). The map's signature adds the
    // node instance.
    StateConditionInstance* CreateInstance(StateGraphInstance& graph) const;  // 0x809060
    // Slot 52. Name not in the reference map.
    int GetType() const;  // 0x809070
    // Slot 53. Name not in the reference map.
    bool IsSelectable() const;  // 0x809080
    int GetNumChildren() const;         // 0x809090
    int GetNumAllowedChildren() const;  // 0x8090A0
    // Lists the types the condition may hold as children: every type unless
    // the list holds some already, less those slot 49 disallows. Name not
    // in the reference map.
    void GetAllowedChildTypes(eastl::vector<int>& types);  // 0x8090B0
    void AddChild(GameObjectId child);     // 0x8092F0
    void RemoveChild(GameObjectId child);  // 0x809300
    // The condition on the object of the child at the index. Name not in
    // the reference map.
    StateCondition* GetChildCondition(unsigned int index) const;  // 0x809310
    const char* GetDescription() const;  // 0x8093A0
    // Appends the objects of the child conditions; true when there are
    // any. Name not in the reference map.
    bool GetChildren(eastl::vector<GameObjectId>& children) const;  // 0x8093B0
    // Appends the objects of all the descendant conditions. Name not in the
    // reference map.
    void GetDescendants(eastl::vector<GameObjectId>& descendants) const;  // 0x8093E0

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6FC060.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x700EC0.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x808B20
    // Makes StateCondition the superclass of a subclass's metadata, as
    // StateBase::_InitAsSuperclass does. Emitted outside the object. Name
    // not in the reference map.
    static void _InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata);  // 0x80D9E0

    static Symbol sId;  // 0x1ABF068, "StateCondition"
    // Also "StateCondition". Name not in the reference map.
    static Symbol sClassName;  // 0x1ABF070
    static PropRegistry sPropRegistry;  // 0x1ABF080
    static ComMetaData sMetaData;       // 0x1ABF120

    // "evaluation_mode" (EvaluationMode). Name not in the reference map.
    int mEvaluationMode;
};

static_assert(offsetof(StateCondition, mEvaluationMode) == 32);
static_assert(sizeof(StateCondition) == 40);

// The run-time state of a condition in a running graph (the map's
// StateConditionInstance in stategraph/StateCondition.o): the last result
// and whether the evaluation mode holds it. The vtable at 0x19509D8 has
// seven slots. The base object is 32 bytes; the subclasses come from the
// small-block pool.
class StateConditionInstance {
public:
    // The map's constructor takes the node instance as well.
    StateConditionInstance(const StateCondition& condition, StateGraphInstance& graph);  // 0x809420
    // Slots 0-1: 0x809440, 0x809450.
    virtual ~StateConditionInstance() {}

    // Slot 2: evaluates the condition; true here. The map's _IsMet()
    // const; this build passes IsMet's flag through.
    virtual bool _IsMet(bool useCached);  // 0x809890
    // Slot 3 at 0x8098A0: forgets what the frame's evaluation kept;
    // nothing here.
    virtual void _ResetPerFrameState() {}
    // Slot 4: takes an event the condition listens for. The base reports
    // that the condition takes no events; the report is compiled out.
    virtual bool _OnEvent(DataArray* event);  // 0x809550
    // Slot 5: whether the result of the frame may be reused; true here.
    // Name not in the reference map; the evidence is weak.
    virtual bool _IsResultCurrent() const;  // 0x8098B0
    // Slot 6: whether the condition depends on the object, besides its
    // own. False here. Name not in the reference map.
    virtual bool _UsesObject(GameObjectId object) const;  // 0x809530

    // The result: the held one while the evaluation mode holds it, or,
    // with `useCached`, the frame's one for a condition evaluated every
    // frame whose result is current; otherwise a new evaluation, which the
    // mode may then hold.
    bool IsMet(bool useCached);  // 0x809460
    // Slot 5 for a condition evaluated every frame. Name not in the
    // reference map.
    bool IsResultCurrent() const;  // 0x8094E0
    void ResetPerFrameState();  // 0x809500
    // Whether the condition is the object's or depends on it. Name not in
    // the reference map.
    bool UsesObject(GameObjectId object) const;  // 0x809510
    // Slot 4. The map's OnEvent(DataArray*).
    bool OnEvent(DataArray* event);  // 0x809540
    // The entity the graph runs for.
    Entity* _GetContextEntity() const;  // 0x809570

    // Field names are not in the reference map.
    const StateCondition* mCondition;
    StateGraphInstance* mGraph;
    bool mResult;
    // Set once the evaluation mode holds the result.
    bool mHeld;
};

static_assert(offsetof(StateConditionInstance, mCondition) == 8);
static_assert(offsetof(StateConditionInstance, mResult) == 24);
static_assert(offsetof(StateConditionInstance, mHeld) == 25);
static_assert(sizeof(StateConditionInstance) == 32);
