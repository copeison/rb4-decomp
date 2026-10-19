#pragma once

#include <cstddef>

#include "entity/core/ComMetaData.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRegistry.h"
#include "entity/props/PropUtl.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/conditions/StateCondition.h"
#include "stategraph/props/DrivenProp.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// Writes `value` to the component's property at the path, converted from
// T. A property that cannot be found or set is reported, which this build
// compiles out; its arguments remain. The counterpart of GetPropValue,
// inlined by the event conditions (for example into
// StateConditionEvent::_AddChild at 0x832C20). Name not in the reference
// map.
template <class T>
void SetPropValue(Component& component, const PropPath& path, const T& value) {
    void* storage = &component;
    const PropInfo* const info = component._GetPropRegistry().FindProp(path, &storage);
    if (info == nullptr || !_SetProp(component, path, *info, storage, &value, DrivenPropType<T>::kType, false)) {
        static_cast<void>(component.MakeErrorName());
        char text[512] = {};
        path.ToString(text, sizeof(text));
    }
}

// The characters of a node that can stand in for a string, without
// evaluating it: a symbol's or a resource path's own, or a string array's.
// Inlined into the event conditions (for example into
// StateConditionEvent::Instance::_OnSpecifiedEvent at 0x8338F0). Name not
// in the reference map.
inline const char* EventNodeText(const DataNode& node) {
    if (node.Type() == kDataResourcePath || node.Type() == kDataSymbol) {
        return node.mValue.symbol;
    }
    return reinterpret_cast<const char*>(node.mValue.array->mNodes);
}

// The condition met when the entity the graph runs for sends an event
// (stategraph/StateConditionEvent.o, 0x8319A0-0x835D37; class id
// "StateConditionEvent"). The event is one a component class of the graph's
// prototype entity exports ("event", found through "source_com_id"); its
// parameters are listed for the editor, and child parameter comparisons
// (StateConditionEventParameter<T>) test them. "sub_event" further compares
// the event's first argument as a string. The vtable at 0x1954AC0 has 54
// slots. The object is 192 bytes.
class StateConditionEvent : public StateCondition {
public:
    // One parameter of the event: its name and type, read-only in the
    // editor. The map names the type; the field names are not in the
    // reference map.
    struct ParamEntry {
        Symbol mName;
        PropertyType mType;
    };

    // The condition's run-time state: subscribed to the event on the
    // graph's entity, it is met by an event whose parameters every child
    // accepts. It is a MsgSink at 0x20 for the subscription. The vtable at
    // 0x1954C80 has eight slots, the MsgSink one at 0x1954CD0 four. The
    // object is 128 bytes, from the small-block pool.
    class Instance : public StateConditionInstance, public MsgSink {
    public:
        POOL_OVERLOAD(Instance)

        // Creates the children's instances and subscribes to the event under
        // "on_specified_event". The map's constructor takes the node
        // instance as well.
        Instance(const StateConditionEvent& condition, StateGraphInstance& graph);  // 0x8332F0
        // Slots 0-1: 0x8335E0, 0x833770; the MsgSink thunks are at 0x8336A0
        // and 0x833830. Deletes the children's instances and unsubscribes.
        ~Instance() override;

        // Slot 2: met without an event, or once the event came.
        bool _IsMet(bool useCached) override;  // 0x833840
        void _ResetPerFrameState() override;   // slot 3: 0x833860
        bool _UsesObject(GameObjectId object) const override;  // slot 6: 0x8338A0
        // Slot 7, and MsgSink's slot 2 through the thunk at 0x833C90: hands
        // "on_specified_event" to _OnSpecifiedEvent; anything else is
        // unhandled.
        DataNode Handle(DataArray* msg, bool warn) override;  // 0x833BA0

        // The event: met when every child accepts it, which consumes it when
        // the condition asks; with a sub event, met when the event's first
        // argument names it instead.
        DataNode _OnSpecifiedEvent(DataArray* msg);  // 0x8338F0

        // Field names are not in the reference map.
        const StateConditionEvent* mEvent;
        bool mMet;
        MsgSource::EventSinkElem mSink;
        eastl::vector<StateConditionInstance*> mChildren;
    };

    // Out of line; Init and _Create call it.
    StateConditionEvent();  // 0x8319A0
    // Copies the condition for an imprint; the event information and the
    // children start unresolved. Inlined into _Imprint (0x833E10).
    StateConditionEvent(const StateConditionEvent& other)
        : StateCondition(other),
          mEvent(other.mEvent),
          mSourceComId(other.mSourceComId),
          mConsume(other.mConsume),
          mSubEvent(other.mSubEvent),
          mParams(other.mParams),
          mChildren(other.mChildren),
          mEventInfo(nullptr),
          mChildConditions() {}
    // Slots 0-1: 0x832590, 0x833CB0.
    ~StateConditionEvent() override {}

    Symbol GetId() const override;          // slot 4: 0x833D90
    Symbol GetClassName() const override;   // slot 5: 0x833DA0
    int CurrentRev() const override;        // slot 7: 0x833DB0
    bool IsA(Symbol type) const override;   // slot 8: 0x833DD0
    Component* AsComponent() override;      // slot 9: 0x833E00
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x833E10
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x833FF0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x834000
    // Slot 29: also looks up the event. The map's
    // _LoadResources(ObjPtr const&).
    bool _OnResourcesLoaded() override;  // 0x832710
    // Slot 41: resolves the children. The map's _Fixup(bool).
    void _Fixup() override;  // 0x832A60
    StateConditionInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 42: 0x834010
    int _GetNumAllowedChildren() const override;  // slot 43: 0x834050
    int _GetNumChildren() const override;         // slot 44: 0x832C10
    // Slot 45: adds a child that is not one yet and makes this condition's
    // object its "parent".
    void _AddChild(GameObjectId child) override;     // 0x832C20
    void _RemoveChild(GameObjectId child) override;  // slot 46: 0x8330A0
    GameObjectId _GetChild(unsigned int index) const override;  // slot 47: 0x833190
    // Slot 48: only parameter comparisons.
    void _GetAllowedChildTypes(eastl::vector<int>& types) const override;  // 0x832B50
    // Slot 50: "Event '<event>' occurs", with the sub event.
    const char* _GetDescription() const override;  // 0x832660
    // Slot 51: the children; they have no children of their own.
    void _GetChildren(eastl::vector<GameObjectId>& children, bool recursive) const override;  // 0x8331A0
    // Slot 52: kEvent.
    int _GetType() const override;  // 0x834060

    // The registry of the event's parameters, or null without an event.
    const PropRegistry* GetEventRegistry() const;  // 0x8332D0
    // Finds the exported event: on the class "source_com_id" names, or on
    // the prototype entity's components, recording the class. A new event
    // lists its parameters again.
    void _SyncEventInfo();  // 0x832740

    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in stategraph/StateGraph.o at 0x6FC410.
    static void Init();
    // The class factory. Emitted in stategraph/StateGraph.o at 0x700F10.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x831A70

    static Symbol sId;  // 0x1AC21A8, "StateConditionEvent"
    // Also "StateConditionEvent". The class symbol GameObject::
    // CreateComponent takes. Name not in the reference map.
    static Symbol sClassName;  // 0x1AC21B0
    static PropRegistry sPropRegistry;  // 0x1AC21C0
    static ComMetaData sMetaData;       // 0x1AC2260

    // Field names are not in the reference map.
    // "event".
    Symbol mEvent;
    // "source_com_id": the class that exports the event.
    Symbol mSourceComId;
    // "consume".
    bool mConsume;
    // "sub_event".
    Symbol mSubEvent;
    // "parameters" (read-only, not saved).
    PropArray<ParamEntry> mParams;
    // "child_conditions" (hidden).
    PropArray<GameObjectId> mChildren;
    // The exported event, resolved by _SyncEventInfo.
    const ComMetaData::ExportedEvent* mEventInfo;
    // The children, resolved by _Fixup.
    eastl::vector<StateCondition*> mChildConditions;
};

static_assert(sizeof(StateConditionEvent::ParamEntry) == 16);
static_assert(offsetof(StateConditionEvent, mEvent) == 40);
static_assert(offsetof(StateConditionEvent, mSourceComId) == 48);
static_assert(offsetof(StateConditionEvent, mConsume) == 56);
static_assert(offsetof(StateConditionEvent, mSubEvent) == 64);
static_assert(offsetof(StateConditionEvent, mParams) == 72);
static_assert(offsetof(StateConditionEvent, mChildren) == 112);
static_assert(offsetof(StateConditionEvent, mEventInfo) == 152);
static_assert(offsetof(StateConditionEvent, mChildConditions) == 160);
static_assert(sizeof(StateConditionEvent) == 192);
static_assert(offsetof(StateConditionEvent::Instance, mEvent) == 40);
static_assert(offsetof(StateConditionEvent::Instance, mMet) == 48);
static_assert(offsetof(StateConditionEvent::Instance, mSink) == 56);
static_assert(offsetof(StateConditionEvent::Instance, mChildren) == 96);
static_assert(sizeof(StateConditionEvent::Instance) == 128);
