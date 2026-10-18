// stategraph/StateConditionLogic.o (0x859690 to 0x85AE07). The class's
// registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/conditions/StateConditionLogic.h"

#include <new>

#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "stategraph/runtime/StateGraphInstance.h"

namespace {

// The id of no object, at 0x1AC3F18, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The array index the last node of the accessor's path names. Inlined into
// the array hooks. Name not in the reference map.
unsigned long LastIndex(const PropAccessorArgs& args) {
    const PropPath& path = *static_cast<const PropPath*>(args.mPath);
    return path[-1].mValue;
}

// "operator" redraws the condition when it changes. Name not in the
// reference map.
// Reconstructed from eboot.elf at 0x85AB20.
void OnOperatorChanged(const PropAccessorArgs& args) {
    static_cast<StateBase*>(args.mComponent)->_SendDisplayTextUpdated();
}

// The "child_conditions" hooks keep the resolved children in step with the
// array: an insert adds the child the inserted id names (the value arrives
// through mOldValue), a removal drops it, and a change resolves it again.
// Names not in the reference map.
// Reconstructed from eboot.elf at 0x85AB30.
void InsertChildHook(const PropAccessorArgs& args) {
    StateConditionLogic* const logic = static_cast<StateConditionLogic*>(args.mComponent);
    const unsigned long index = LastIndex(args);
    const GameObjectId* const inserted = static_cast<const GameObjectId*>(args.mOldValue);
    GameObjectId id = inserted != nullptr ? *inserted : gNullObjectId;
    logic->mChildren._Insert(index, &id);
    StateCondition* const child = logic->_GetObjectBaseCom<StateCondition>(id, true);
    logic->mRuntime.mChildConditions.insert(logic->mRuntime.mChildConditions.begin() + index, child);
}

// Reconstructed from eboot.elf at 0x85AC00. The element is destroyed and
// the rest moved down through the array's vtable.
void RemoveChildHook(const PropAccessorArgs& args) {
    StateConditionLogic* const logic = static_cast<StateConditionLogic*>(args.mComponent);
    PropArray<GameObjectId>& children = logic->mChildren;
    const unsigned long index = LastIndex(args);
    children.Destruct(1, children.ElementAt(index));
    if (static_cast<unsigned int>(index) < children.mSize - 1) {
        children.Move(children.mSize - 1 - index, children.ElementAt(index), children.ElementAt(index + 1));
    }
    --children.mSize;
    logic->mRuntime.mChildConditions.erase(logic->mRuntime.mChildConditions.begin() + index);
}

// Reconstructed from eboot.elf at 0x85ACC0.
void OnChildChanged(const PropAccessorArgs& args) {
    StateConditionLogic* const logic = static_cast<StateConditionLogic*>(args.mComponent);
    logic->mRuntime.mChildConditions.resize(logic->mChildren.size());
    const unsigned long index = LastIndex(args);
    logic->mRuntime.mChildConditions[index] =
        logic->_GetObjectBaseCom<StateCondition>(logic->mChildren[index], true);
}

}  // namespace

// The object's statics, in the order of its static initializer (0x85AD50).
Symbol StateConditionLogic::sId("StateConditionLogic");
Symbol StateConditionLogic::sClassName("StateConditionLogic");
PropRegistry StateConditionLogic::sPropRegistry;
ComMetaData StateConditionLogic::sMetaData;

// Reconstructed from eboot.elf at 0x859690.
StateConditionLogic::StateConditionLogic() : mOperator(kAllOf), mChildren(), mRuntime() {}

// Reconstructed from eboot.elf at 0x859710.
StateConditionLogic::RuntimeData::RuntimeData() : mChildConditions() {}

// Reconstructed from eboot.elf at 0x859730. The children are hidden; the
// editor shows them as the condition's children instead.
void StateConditionLogic::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateConditionLogic prototype;
    if (&prototype._GetPropRegistry() != &registry) {
        registry.AddInheritedRegistry(sClassName, prototype._GetPropRegistry());
    }
    IntMetadata<int>& op = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "operator", offsetof(StateConditionLogic, mOperator), kPropertyInt, 4).mMetadata.Get());
    op.mDefault = prototype.mOperator;
    static_cast<void>(Symbol("prop"));
    op.mHelp = String("The type of this logical condition.");
    op.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{kAllOf, String("And (&&)"), String("Return true when all child conditions are true.")},
        AllowedValue<int>{
            kAnyOf, String("Or (||)"), String("Return true when at least one child condition is true.")}};
    op.mOnChanged = OnOperatorChanged;

    PropInfo& childrenInfo = registry.RegisterPropArray(
        "child_conditions", offsetof(StateConditionLogic, mChildren), sizeof(PropArray<GameObjectId>));
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
        "Implements the logical AND (&&) and OR (||) operators for complex StateGraph transition conditions.";
    metadata.mAuthor = "Matt Sharpe";
}

// Reconstructed from eboot.elf at 0x859C90.
StateConditionLogic::~StateConditionLogic() {}

// Reconstructed from eboot.elf at 0x859D20.
void StateConditionLogic::_Fixup() {
    eastl::vector<StateCondition*>& conditions = mRuntime.mChildConditions;
    conditions.resize(mChildren.size());
    for (unsigned long index = 0; index < conditions.size(); ++index) {
        conditions[index] = _GetObjectBaseCom<StateCondition>(mChildren[index], true);
    }
}

// Reconstructed from eboot.elf at 0x859DE0.
void StateConditionLogic::_GetDisallowedChildTypes(eastl::vector<int>& types) const {
    types.push_back(kParameterComparison);
}

// Reconstructed from eboot.elf at 0x859EA0.
int StateConditionLogic::_GetNumChildren() const {
    return mChildren.size();
}

// Reconstructed from eboot.elf at 0x859EB0.
void StateConditionLogic::_AddChild(GameObjectId child) {
    const unsigned long count = mChildren.size();
    for (unsigned long index = 0; index < count; ++index) {
        if (mChildren[index].mId == child.mId) {
            return;
        }
    }
    mChildren._Insert(count, &child);
    mRuntime.mChildConditions.push_back(_GetObjectBaseCom<StateCondition>(child, true));
}

// Reconstructed from eboot.elf at 0x859FF0. The element is destroyed and
// the rest moved down through the array's vtable.
void StateConditionLogic::_RemoveChild(GameObjectId child) {
    const unsigned long count = mChildren.size();
    for (unsigned long index = 0; index < count; ++index) {
        if (mChildren[index].mId != child.mId) {
            continue;
        }
        mChildren.Destruct(1, mChildren.ElementAt(index));
        if (static_cast<unsigned int>(index) < mChildren.mSize - 1) {
            mChildren.Move(mChildren.mSize - 1 - index, mChildren.ElementAt(index), mChildren.ElementAt(index + 1));
        }
        --mChildren.mSize;
        mRuntime.mChildConditions.erase(mRuntime.mChildConditions.begin() + index);
        return;
    }
}

// Reconstructed from eboot.elf at 0x85A0C0.
GameObjectId StateConditionLogic::_GetChild(unsigned int index) const {
    return mChildren[index];
}

// Reconstructed from eboot.elf at 0x85A0D0.
const char* StateConditionLogic::_GetDescription() const {
    return mOperator == kAllOf ? "All of the following..." : "Any of the following...";
}

// Reconstructed from eboot.elf at 0x85A0F0.
void StateConditionLogic::_GetChildren(eastl::vector<GameObjectId>& children, bool recursive) const {
    for (StateCondition* const child : mRuntime.mChildConditions) {
        children.push_back(child->mObject->mId);
        if (recursive) {
            child->GetDescendants(children);
        }
    }
}

// Reconstructed from eboot.elf at 0x85A310. The result is current while
// every child's is.
StateConditionLogic::Instance::Instance(const StateConditionLogic& condition, StateGraphInstance& graph)
    : StateConditionInstance(condition, graph), mLogic(&condition), mChildren(), mResultCurrent(true) {
    const unsigned int count = condition.GetNumChildren();
    mChildren.reserve(count);
    for (unsigned long index = 0; index < count; ++index) {
        StateConditionInstance* const child = condition.mRuntime.mChildConditions[index]->CreateInstance(graph);
        mChildren.push_back(child);
        mResultCurrent = mResultCurrent && child->IsResultCurrent();
    }
}

// Reconstructed from eboot.elf at 0x85A540. The deleting destructor at
// 0x85A5C0 returns the instance to the pool.
StateConditionLogic::Instance::~Instance() {
    for (StateConditionInstance* const child : mChildren) {
        delete child;
    }
    mChildren.clear();
}

// Reconstructed from eboot.elf at 0x85A650.
bool StateConditionLogic::Instance::_IsMet(bool useCached) {
    if (mChildren.empty()) {
        mResultCurrent = true;
        mHeld = true;
        return false;
    }
    bool result = mLogic->mOperator == kAllOf;
    bool allHeld = true;
    mResultCurrent = true;
    for (StateConditionInstance* const child : mChildren) {
        const bool met = child->IsMet(useCached);
        const bool held = child->mHeld;
        if (met == (mLogic->mOperator == kAnyOf)) {
            useCached = true;
            result = met;
            if (held) {
                mHeld = true;
                return met;
            }
        }
        if (mResultCurrent && !held) {
            mResultCurrent = child->IsResultCurrent();
        }
        allHeld = allHeld && held;
    }
    if (allHeld) {
        mHeld = true;
    }
    return result;
}

// Reconstructed from eboot.elf at 0x85A750.
void StateConditionLogic::Instance::_ResetPerFrameState() {
    for (StateConditionInstance* const child : mChildren) {
        child->ResetPerFrameState();
    }
}

// Reconstructed from eboot.elf at 0x85A790.
bool StateConditionLogic::Instance::_UsesObject(GameObjectId object) const {
    for (const StateConditionInstance* const child : mChildren) {
        if (child->UsesObject(object)) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x85A870.
Symbol StateConditionLogic::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x85A880.
Symbol StateConditionLogic::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x85A890.
int StateConditionLogic::CurrentRev() const {
    return const_cast<StateConditionLogic*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x85A8B0.
bool StateConditionLogic::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x85A8E0.
Component* StateConditionLogic::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x85A8F0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateConditionLogic::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateConditionLogic*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateConditionLogic(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x85AA90.
PropRegistry& StateConditionLogic::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x85AAA0.
ComMetaData& StateConditionLogic::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x85AAB0.
StateConditionInstance* StateConditionLogic::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x85AAF0.
int StateConditionLogic::_GetNumAllowedChildren() const {
    return -1;
}

// Reconstructed from eboot.elf at 0x85AB00.
int StateConditionLogic::_GetType() const {
    return mOperator;
}

// Reconstructed from eboot.elf at 0x85AB10.
bool StateConditionLogic::Instance::_IsResultCurrent() const {
    return mResultCurrent;
}

// Reconstructed from eboot.elf at 0x701100. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateConditionLogic::_Create() {
    return new StateConditionLogic();
}

// Reconstructed from eboot.elf at 0x6FC8C0. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateConditionLogic::Init() {
    InitComponentClass<StateConditionLogic>();
}
