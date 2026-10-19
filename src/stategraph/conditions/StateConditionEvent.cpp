// stategraph/StateConditionEvent.o (0x8319A0 to 0x835D37). The class's
// registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/conditions/StateConditionEvent.h"

#include <new>
#include <string.h>
#include <strings.h>

#include "entity/core/Entity.h"
#include "entity/props/PropMetadata.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "utl/containers/Sort.h"
#include "utl/text/MakeString.h"

namespace {

// The id of no object, at 0x1AC21A0, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The array index the last node of the accessor's path names. Inlined into
// the array hooks. Name not in the reference map.
unsigned long LastIndex(const PropAccessorArgs& args) {
    const PropPath& path = *static_cast<const PropPath*>(args.mPath);
    return path[-1].mValue;
}

// A new event looks for its class again. Names of the accessors are not in
// the reference map.
// Reconstructed from eboot.elf at 0x8340D0.
void OnEventChanged(const PropAccessorArgs& args) {
    StateConditionEvent* const condition = static_cast<StateConditionEvent*>(args.mComponent);
    condition->mSourceComId = Symbol();
    condition->_SyncEventInfo();
    condition->_SendDisplayTextUpdated();
}

// Reconstructed from eboot.elf at 0x834100.
void OnSubEventChanged(const PropAccessorArgs& args) {
    static_cast<StateConditionEvent*>(args.mComponent)->_SendDisplayTextUpdated();
}

// "type" of a parameter: the name of its property type. The path is
// ("parameters" <index> "type").
// Reconstructed from eboot.elf at 0x834110.
Symbol GetParamTypeAccessor(const PropAccessorArgs& args) {
    const StateConditionEvent* const condition = static_cast<const StateConditionEvent*>(args.mComponent);
    const PropPath& path = *static_cast<const PropPath*>(args.mPath);
    return Symbol(TypeName(condition->mParams[path[1].mValue].mType, true));
}

// The "child_conditions" hooks keep the resolved children in step with the
// array: an insert adds the condition the inserted id names (the value
// arrives through mOldValue), a removal drops it, and a change resolves it
// again.
// Reconstructed from eboot.elf at 0x834170.
void InsertChildHook(const PropAccessorArgs& args) {
    StateConditionEvent* const condition = static_cast<StateConditionEvent*>(args.mComponent);
    const unsigned long index = LastIndex(args);
    const GameObjectId* const inserted = static_cast<const GameObjectId*>(args.mOldValue);
    GameObjectId id = inserted != nullptr ? *inserted : gNullObjectId;
    condition->mChildren._Insert(index, &id);
    StateCondition* const child = condition->_GetObjectBaseCom<StateCondition>(id, true);
    condition->mChildConditions.insert(condition->mChildConditions.begin() + index, child);
}

// Reconstructed from eboot.elf at 0x834370. The element is destroyed and
// the rest moved down through the array's vtable.
void RemoveChildHook(const PropAccessorArgs& args) {
    StateConditionEvent* const condition = static_cast<StateConditionEvent*>(args.mComponent);
    PropArray<GameObjectId>& children = condition->mChildren;
    const unsigned long index = LastIndex(args);
    children.Destruct(1, children.ElementAt(index));
    if (static_cast<unsigned int>(index) < children.mSize - 1) {
        children.Move(children.mSize - 1 - index, children.ElementAt(index), children.ElementAt(index + 1));
    }
    --children.mSize;
    condition->mChildConditions.erase(condition->mChildConditions.begin() + index);
}

// Reconstructed from eboot.elf at 0x834450.
void OnChildChanged(const PropAccessorArgs& args) {
    StateConditionEvent* const condition = static_cast<StateConditionEvent*>(args.mComponent);
    condition->mChildConditions.resize(condition->mChildren.size());
    const unsigned long index = LastIndex(args);
    condition->mChildConditions[index] =
        condition->_GetObjectBaseCom<StateCondition>(condition->mChildren[index], true);
}

}  // namespace

// The object's statics, in the order of its static initializer (0x835C80).
Symbol StateConditionEvent::sId("StateConditionEvent");
Symbol StateConditionEvent::sClassName("StateConditionEvent");
PropRegistry StateConditionEvent::sPropRegistry;
ComMetaData StateConditionEvent::sMetaData;

// Reconstructed from eboot.elf at 0x8319A0.
StateConditionEvent::StateConditionEvent()
    : mEvent(),
      mSourceComId(),
      mConsume(false),
      mSubEvent(),
      mParams(),
      mChildren(),
      mEventInfo(nullptr),
      mChildConditions() {}

// Reconstructed from eboot.elf at 0x831A70. The parameters are listed for
// the editor and not saved; the children are hidden, as the editor shows
// them under the condition.
void StateConditionEvent::_Init(PropRegistry& registry, ComMetaData& metadata) {
    RegisterPropStruct<ParamEntry>(Symbol("StateConditionEvent::ParamEntry"));
    StateConditionEvent prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&own != &registry) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    SymbolMetadata& event = TypeSpecificMetadata(
        registry.RegisterProp("event", offsetof(StateConditionEvent, mEvent), kPropertySymbol, sizeof(Symbol)),
        prototype.mEvent);
    event.mDefault = prototype.mEvent;
    static_cast<void>(Symbol("prop"));
    event.mHelp = String("The event that this condition is listening for.");
    // At 0x834670: the events every component of the prototype entity
    // exports, sorted without case and without duplicates.
    event.mAllowedValues =
        std::function<eastl::vector<AllowedValue<Symbol>>(const TagArgs&)>([](const TagArgs& args) {
            eastl::vector<AllowedValue<Symbol>> values;
            const Entity* const entity =
                static_cast<const StateConditionEvent&>(args.mComponent)._GetContextEntityPrototype();
            for (const Entity::Layer& layer : entity->mLayers) {
                for (GameObject* const object : layer.mObjects) {
                    if (object == nullptr) {
                        continue;
                    }
                    for (const GameObject::ComIndex& index : object->mComs) {
                        Component* const com = index.mCom;
                        if (com == nullptr) {
                            continue;
                        }
                        for (const ComMetaData::ExportedEvent& exported : com->_GetMetaData().mExportedEvents) {
                            values.push_back(AllowedValue<Symbol>{
                                exported.mName, String(nullptr), String(exported.mDescription.c_str())});
                        }
                    }
                }
            }
            if (!values.empty()) {
                eastl::sort(values.begin(), values.end(), [](const AllowedValue<Symbol>& a, const AllowedValue<Symbol>& b) {
                    return strcasecmp(a.mValue.Str(), b.mValue.Str()) < 0;
                });
            }
            AllowedValue<Symbol>* kept = values.begin();
            if (kept != values.end()) {
                AllowedValue<Symbol>* next = kept + 1;
                while (next != values.end() && kept->mValue != next->mValue) {
                    kept = next++;
                }
                if (next != values.end()) {
                    for (++next; next != values.end(); ++next) {
                        if (kept->mValue != next->mValue) {
                            *++kept = *next;
                        }
                    }
                    values.erase(kept + 1, values.end());
                }
            }
            return values;
        });
    event.mOnChanged = OnEventChanged;
    event.mInstanceFile = true;

    SymbolMetadata& subEvent = TypeSpecificMetadata(
        registry.RegisterProp("sub_event", offsetof(StateConditionEvent, mSubEvent), kPropertySymbol, sizeof(Symbol)),
        prototype.mSubEvent);
    subEvent.mDefault = prototype.mSubEvent;
    static_cast<void>(Symbol("prop"));
    subEvent.mHelp = String(
        "Optionally specify a sub event. This will be compared against the first argument of the event, "
        "interpreted as a string. Note that only certain events will follow this format. Eventually, there will "
        "be more full-featured support for checking event arguments but this unblocks some crucial development "
        "for now.");
    // At 0x8359F0: hidden while empty.
    subEvent.mIsHidden._Set([](const TagArgs& args) {
        return static_cast<const StateConditionEvent&>(args.mComponent).mSubEvent == Symbol();
    });
    subEvent.mOnChanged = OnSubEventChanged;

    SymbolMetadata& source = TypeSpecificMetadata(
        registry.RegisterProp(
            "source_com_id", offsetof(StateConditionEvent, mSourceComId), kPropertySymbol, sizeof(Symbol)),
        prototype.mSourceComId);
    source.mDefault = prototype.mSourceComId;
    static_cast<void>(Symbol("prop"));
    source.mHelp = String("The component that sends this event.");
    source.mReadOnly = true;

    BoolMetadata& consume = TypeSpecificMetadata(
        registry.RegisterProp("consume", offsetof(StateConditionEvent, mConsume), kPropertyBool, sizeof(bool)),
        prototype.mConsume);
    consume.mDefault = prototype.mConsume;
    static_cast<void>(Symbol("prop"));
    consume.mHelp = String(
        "Mainly for UI events: should this UI event get 'met', will it consume the event so other "
        "objects/entities in the UI hierarchy do not receive it? You should consume for ui_select and "
        "ui_button_down if you are not just doing visual FX but are actually handling logic.");

    PropInfo& paramsInfo = registry.RegisterPropArray(
        "parameters", offsetof(StateConditionEvent, mParams), sizeof(PropArray<ParamEntry>));
    ArrayMetadata& params = static_cast<ArrayMetadata&>(*paramsInfo.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    params.mHelp = String("This event's parameters, in the order they appear in the event.");
    params.mReadOnly = true;
    params.mSaves = false;
    PropRegistry::RegisterPropArrayItem(paramsInfo, kPropertyStruct, sizeof(ParamEntry));
    GroupMetadata& param = static_cast<GroupMetadata&>(*params.mItemInfo.mMetadata.Get());
    static_cast<void>(Symbol("struct_item"));
    const Symbol nameType;
    param.mHelp = String("A parameter within the event.");
    param.mKind = 1;
    PropRegistry& members = param.mRegistry;
    SymbolMetadata& name = TypeSpecificMetadata(
        members.RegisterProp("name", offsetof(ParamEntry, mName), kPropertySymbol, sizeof(Symbol)), nameType);
    name.mDefault = nameType;
    const Symbol typeType;
    SymbolMetadata& type = TypeSpecificMetadata(members.RegisterProp("type", -1, kPropertySymbol, 0), typeType);
    static_cast<void>(Symbol("prop"));
    type.mGetter = GetParamTypeAccessor;

    PropInfo& childrenInfo = registry.RegisterPropArray(
        "child_conditions", offsetof(StateConditionEvent, mChildren), sizeof(PropArray<GameObjectId>));
    ArrayMetadata& children = static_cast<ArrayMetadata&>(*childrenInfo.mMetadata.Get());
    static_cast<void>(Symbol("array"));
    children.mIsHidden = true;
    children.mInsert = InsertChildHook;
    children.mHook2 = reinterpret_cast<void*>(&RemoveChildHook);
    PropRegistry::RegisterPropArrayItem(childrenInfo, kPropertyGameObjectId, sizeof(GameObjectId));
    const GameObjectId itemType = {0xFFFFFFFFU};
    GameObjectIdMetadata& item = TypeSpecificMetadata(children.mItemInfo, itemType);
    static_cast<void>(Symbol("item"));
    item.mOnChanged = OnChildChanged;

    StateCondition::_InitAsSuperclass(registry, metadata);
    metadata.mDescription =
        "Enables StateGraph transitions based on asynchronous events (for example, user input)";
    metadata.mAuthor = "Matt Sharpe";
}

// Reconstructed from eboot.elf at 0x832660.
const char* StateConditionEvent::_GetDescription() const {
    if (mSubEvent == Symbol()) {
        FormatString text("Event '%s' occurs");
        text << mEvent;
        return text.Str();
    }
    FormatString text("Event '%s : %s' occurs");
    text << mEvent << mSubEvent;
    return text.Str();
}

// Reconstructed from eboot.elf at 0x832710.
bool StateConditionEvent::_OnResourcesLoaded() {
    if (!StateBase::_OnResourcesLoaded()) {
        return false;
    }
    _SyncEventInfo();
    return true;
}

// Reconstructed from eboot.elf at 0x832740. Without a class, every
// component of the prototype entity is searched and the first class that
// exports the event is kept.
void StateConditionEvent::_SyncEventInfo() {
    const ComMetaData::ExportedEvent* info = nullptr;
    if (mEvent != Symbol()) {
        if (mSourceComId == Symbol()) {
            const Entity* const entity = _GetContextEntityPrototype();
            if (entity != nullptr) {
                for (const Entity::Layer& layer : entity->mLayers) {
                    for (GameObject* const object : layer.mObjects) {
                        if (object == nullptr) {
                            continue;
                        }
                        for (const GameObject::ComIndex& index : object->mComs) {
                            Component* const com = index.mCom;
                            if (info != nullptr || com == nullptr) {
                                continue;
                            }
                            for (const ComMetaData::ExportedEvent& exported : com->_GetMetaData().mExportedEvents) {
                                if (exported.mName == mEvent) {
                                    info = &exported;
                                    break;
                                }
                            }
                            if (info != nullptr) {
                                mSourceComId = com->GetId();
                            }
                        }
                    }
                }
            }
        } else {
            const ComMetaData* const source = ComMetaData::GetMetaData(mSourceComId);
            if (source != nullptr) {
                for (const ComMetaData::ExportedEvent& exported : source->mExportedEvents) {
                    if (exported.mName == mEvent) {
                        info = &exported;
                        break;
                    }
                }
            }
        }
    }
    if (info == mEventInfo) {
        return;
    }
    mEventInfo = info;
    mParams.Resize(0);
    if (mEventInfo == nullptr) {
        return;
    }
    const eastl::vector<PropRegistry::PropEntry>& entries = mEventInfo->mParams.mProps;
    mParams.Reserve(static_cast<unsigned int>(entries.size()));
    for (const PropRegistry::PropEntry& entry : entries) {
        const ParamEntry param = {entry.mName, entry.mInfo.mType};
        mParams._Insert(mParams.mSize, &param);
    }
}

// Reconstructed from eboot.elf at 0x832A60.
void StateConditionEvent::_Fixup() {
    mChildConditions.resize(mChildren.size());
    for (unsigned long index = 0; index < mChildConditions.size(); ++index) {
        mChildConditions[index] = _GetObjectBaseCom<StateCondition>(mChildren[index], true);
    }
}

// Reconstructed from eboot.elf at 0x832B50.
void StateConditionEvent::_GetAllowedChildTypes(eastl::vector<int>& types) const {
    types.push_back(kParameterComparison);
}

// Reconstructed from eboot.elf at 0x832C10.
int StateConditionEvent::_GetNumChildren() const {
    return mChildren.size();
}

// Reconstructed from eboot.elf at 0x832C20. A child names this
// condition's object as its "parent".
void StateConditionEvent::_AddChild(GameObjectId child) {
    for (unsigned long index = 0; index < mChildren.size(); ++index) {
        if (mChildren[index].mId == child.mId) {
            return;
        }
    }
    unsigned long index = 0;
    while (index < mChildren.size() && mChildren[index].mId != child.mId) {
        ++index;
    }
    if (index == mChildren.size()) {
        mChildren._Insert(index, &child);
        mChildConditions.push_back(_GetObjectBaseCom<StateCondition>(child, true));
    }
    static Symbol parent;
    if (parent == Symbol()) {
        parent = Symbol("parent");
    }
    SetPropValue(*mChildConditions.back(), PropPath(parent), mObject->mId);
}

// Reconstructed from eboot.elf at 0x8330A0. The element is destroyed and
// the rest moved down through the array's vtable.
void StateConditionEvent::_RemoveChild(GameObjectId child) {
    for (unsigned long index = 0; index < mChildren.size(); ++index) {
        if (mChildren[index].mId != child.mId) {
            continue;
        }
        mChildren.Destruct(1, mChildren.ElementAt(index));
        if (static_cast<unsigned int>(index) < mChildren.mSize - 1) {
            mChildren.Move(mChildren.mSize - 1 - index, mChildren.ElementAt(index), mChildren.ElementAt(index + 1));
        }
        --mChildren.mSize;
        mChildConditions.erase(mChildConditions.begin() + index);
        return;
    }
}

// Reconstructed from eboot.elf at 0x833190.
GameObjectId StateConditionEvent::_GetChild(unsigned int index) const {
    return mChildren[index];
}

// Reconstructed from eboot.elf at 0x8331A0.
void StateConditionEvent::_GetChildren(eastl::vector<GameObjectId>& children, bool recursive) const {
    static_cast<void>(recursive);
    for (const GameObjectId& child : mChildren) {
        children.push_back(child);
    }
}

// Reconstructed from eboot.elf at 0x8332D0.
const PropRegistry* StateConditionEvent::GetEventRegistry() const {
    if (mEventInfo == nullptr) {
        return nullptr;
    }
    return &mEventInfo->mParams;
}

// Reconstructed from eboot.elf at 0x8332F0. Without an event nothing is
// subscribed.
StateConditionEvent::Instance::Instance(const StateConditionEvent& condition, StateGraphInstance& graph)
    : StateConditionInstance(condition, graph), mEvent(&condition), mMet(false), mSink(), mChildren() {
    mSink.mSink = nullptr;
    mSink.mSource = nullptr;
    const unsigned int count = condition.GetNumChildren();
    mChildren.reserve(count);
    for (unsigned long index = 0; index < count; ++index) {
        mChildren.push_back(condition.mChildConditions[index]->CreateInstance(graph));
    }
    static Symbol onSpecifiedEvent;
    if (onSpecifiedEvent == Symbol()) {
        onSpecifiedEvent = Symbol("on_specified_event");
    }
    const Symbol event = mEvent->mEvent;
    if (event != Symbol()) {
        _GetContextEntity()->AddSink(&mSink, static_cast<MsgSink*>(this), event, onSpecifiedEvent);
    }
}

// Reconstructed from eboot.elf at 0x8335E0. The deleting destructor at
// 0x833770 returns the instance to the pool. The binary unsubscribes after
// freeing the children's vector.
StateConditionEvent::Instance::~Instance() {
    for (StateConditionInstance* const child : mChildren) {
        delete child;
    }
    mChildren.clear();
    LinkedList::Node& link = mSink.mLink;
    if (link.mNext != &link && link.mPrev != &link) {
        mSink.mSource->RemoveSink(&mSink);
    }
}

// Reconstructed from eboot.elf at 0x833840.
bool StateConditionEvent::Instance::_IsMet(bool useCached) {
    static_cast<void>(useCached);
    return mEvent->mEvent == Symbol() || mMet;
}

// Reconstructed from eboot.elf at 0x833860.
void StateConditionEvent::Instance::_ResetPerFrameState() {
    mMet = false;
    for (StateConditionInstance* const child : mChildren) {
        child->ResetPerFrameState();
    }
}

// Reconstructed from eboot.elf at 0x8338A0.
bool StateConditionEvent::Instance::_UsesObject(GameObjectId object) const {
    for (StateConditionInstance* const child : mChildren) {
        if (child->UsesObject(object)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x8338F0. The children see the event in
// turn until one rejects it. A consumed event is handled with 0.
DataNode StateConditionEvent::Instance::_OnSpecifiedEvent(DataArray* msg) {
    DataNode result;
    static Symbol onSpecifiedEvent;
    if (onSpecifiedEvent == Symbol()) {
        onSpecifiedEvent = Symbol("on_specified_event");
    }
    if (msg->Sym(1) != onSpecifiedEvent) {
        return result;
    }
    bool met = true;
    for (StateConditionInstance* const child : mChildren) {
        if (!child->OnEvent(msg)) {
            met = false;
            break;
        }
    }
    if (met) {
        mMet = true;
        if (mEvent->mConsume) {
            result = DataNode(0);
        }
    }
    if (mEvent->mSubEvent != Symbol() && msg->Size() >= 3 && msg->Node(2).CompatibleType(kDataString)) {
        const char* const text = EventNodeText(msg->Node(2));
        mMet = text != nullptr ? strcmp(mEvent->mSubEvent.Str(), text) == 0 : mEvent->mSubEvent == Symbol();
    }
    return result;
}

// Reconstructed from eboot.elf at 0x833BA0. The MsgSink thunk is at
// 0x833C90.
DataNode StateConditionEvent::Instance::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol onSpecifiedEvent;
    if (onSpecifiedEvent == Symbol()) {
        onSpecifiedEvent = Symbol("on_specified_event");
    }
    if (type == onSpecifiedEvent) {
        DataNode result = _OnSpecifiedEvent(msg);
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    return DataNode();
}

// Reconstructed from eboot.elf at 0x833D90.
Symbol StateConditionEvent::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x833DA0.
Symbol StateConditionEvent::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x833DB0.
int StateConditionEvent::CurrentRev() const {
    return const_cast<StateConditionEvent*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x833DD0.
bool StateConditionEvent::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x833E00.
Component* StateConditionEvent::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x833E10. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateConditionEvent::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateConditionEvent*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateConditionEvent(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x833FF0.
PropRegistry& StateConditionEvent::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x834000.
ComMetaData& StateConditionEvent::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x834010.
StateConditionInstance* StateConditionEvent::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x834050.
int StateConditionEvent::_GetNumAllowedChildren() const {
    return -1;
}

// Reconstructed from eboot.elf at 0x834060.
int StateConditionEvent::_GetType() const {
    return kEvent;
}

// Reconstructed from eboot.elf at 0x700F10. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateConditionEvent::_Create() {
    return new StateConditionEvent();
}

// Reconstructed from eboot.elf at 0x6FC410. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateConditionEvent::Init() {
    InitComponentClass<StateConditionEvent>();
}
