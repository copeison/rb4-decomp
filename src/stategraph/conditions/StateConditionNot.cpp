// stategraph/StateConditionNot.o (0x85B310 to 0x85BCC7). The class's
// registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/conditions/StateConditionNot.h"

#include <new>

#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "stategraph/runtime/StateGraphInstance.h"

namespace {

// The id of no object, at 0x1AC43D8, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// "child_condition" sets the child through the condition. Name not in the
// reference map.
// Reconstructed from eboot.elf at 0x85BBE0.
void SetChildAccessor(const PropAccessorArgs& args) {
    StateConditionNot* const condition = static_cast<StateConditionNot*>(args.mComponent);
    const GameObjectId child = *static_cast<const GameObjectId*>(args.mValue);
    condition->mChild = child;
    condition->mRuntime.mChildCondition = condition->_GetObjectBaseCom<StateCondition>(child, true);
}

}  // namespace

// The object's statics, in the order of its static initializer (0x85BC10).
Symbol StateConditionNot::sId("StateConditionNot");
Symbol StateConditionNot::sClassName("StateConditionNot");
PropRegistry StateConditionNot::sPropRegistry;
ComMetaData StateConditionNot::sMetaData;

// Reconstructed from eboot.elf at 0x85B310.
StateConditionNot::RuntimeData::RuntimeData() : mChildCondition(nullptr) {}

// Reconstructed from eboot.elf at 0x85B320. The child is hidden; the
// editor shows it as the condition's child instead.
void StateConditionNot::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateConditionNot prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    GameObjectIdMetadata& child = TypeSpecificMetadata(
        registry.RegisterProp("child_condition", offsetof(StateConditionNot, mChild), kPropertyGameObjectId, 4),
        prototype.mChild);
    child.mDefault = prototype.mChild;
    static_cast<void>(Symbol("prop"));
    child.mIsHidden = true;
    child.mSetter = SetChildAccessor;
    StateCondition::_InitAsSuperclass(registry, metadata);
    metadata.mDescription =
        "Implements the logical NOT (!) operator for complex StateGraph transition conditions.";
    metadata.mAuthor = "Matt Sharpe";
}

// Reconstructed from eboot.elf at 0x85B4F0.
void StateConditionNot::_Fixup() {
    mRuntime.mChildCondition = _GetObjectBaseCom<StateCondition>(mChild, true);
}

// Reconstructed from eboot.elf at 0x85B520.
bool StateConditionNot::HasChildCondition() const {
    return mChild.mId != gNullObjectId.mId;
}

// Reconstructed from eboot.elf at 0x85B530.
void StateConditionNot::_GetDisallowedChildTypes(eastl::vector<int>& types) const {
    types.push_back(kParameterComparison);
}

// Reconstructed from eboot.elf at 0x85B5F0.
int StateConditionNot::_GetNumChildren() const {
    return mChild.mId != gNullObjectId.mId;
}

// Reconstructed from eboot.elf at 0x85B600.
void StateConditionNot::_AddChild(GameObjectId child) {
    mChild = child;
    mRuntime.mChildCondition = _GetObjectBaseCom<StateCondition>(child, true);
}

// Reconstructed from eboot.elf at 0x85B630.
void StateConditionNot::SetChildCondition(GameObjectId child) {
    mChild = child;
    mRuntime.mChildCondition = _GetObjectBaseCom<StateCondition>(child, true);
}

// Reconstructed from eboot.elf at 0x85B660.
void StateConditionNot::_RemoveChild(GameObjectId child) {
    static_cast<void>(child);
    const GameObjectId none = gNullObjectId;
    mChild = none;
    mRuntime.mChildCondition = _GetObjectBaseCom<StateCondition>(none, true);
}

// Reconstructed from eboot.elf at 0x85B690.
GameObjectId StateConditionNot::_GetChild(unsigned int index) const {
    static_cast<void>(index);
    return mChild;
}

// Reconstructed from eboot.elf at 0x85B6A0.
const char* StateConditionNot::_GetDescription() const {
    return "Not";
}

// Reconstructed from eboot.elf at 0x85B6B0.
void StateConditionNot::_GetChildren(eastl::vector<GameObjectId>& children, bool recursive) const {
    if (mChild.mId == gNullObjectId.mId) {
        return;
    }
    children.push_back(mChild);
    if (recursive) {
        mRuntime.mChildCondition->GetDescendants(children);
    }
}

// Reconstructed from eboot.elf at 0x85B7C0. A missing child leaves no
// instance and a current result.
StateConditionNot::Instance::Instance(const StateConditionNot& condition, StateGraphInstance& graph)
    : StateConditionInstance(condition, graph), mChild(nullptr), mResultCurrent(true) {
    const StateCondition* const child = condition.mRuntime.mChildCondition;
    if (child != nullptr) {
        mChild = child->CreateInstance(graph);
        mResultCurrent = mChild->IsResultCurrent();
    }
}

// Reconstructed from eboot.elf at 0x85B820. The deleting destructor at
// 0x85B860 returns the instance to the pool.
StateConditionNot::Instance::~Instance() {
    delete mChild;
}

// Reconstructed from eboot.elf at 0x85B8B0.
bool StateConditionNot::Instance::_IsMet(bool useCached) {
    if (mChild == nullptr) {
        mHeld = true;
        return false;
    }
    const bool met = mChild->IsMet(useCached);
    if (mChild->mHeld) {
        mHeld = true;
    }
    mResultCurrent = mChild->IsResultCurrent();
    return !met;
}

// Reconstructed from eboot.elf at 0x85B900.
void StateConditionNot::Instance::_ResetPerFrameState() {
    if (mChild != nullptr) {
        mChild->ResetPerFrameState();
    }
}

// Reconstructed from eboot.elf at 0x85B910.
bool StateConditionNot::Instance::_UsesObject(GameObjectId object) const {
    if (mChild == nullptr) {
        return false;
    }
    return mChild->UsesObject(object);
}

// Reconstructed from eboot.elf at 0x85B950.
Symbol StateConditionNot::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x85B960.
Symbol StateConditionNot::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x85B970.
int StateConditionNot::CurrentRev() const {
    return const_cast<StateConditionNot*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x85B990.
bool StateConditionNot::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x85B9C0.
Component* StateConditionNot::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x85B9D0. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateConditionNot::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateConditionNot*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateConditionNot(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x85BB10.
PropRegistry& StateConditionNot::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x85BB20.
ComMetaData& StateConditionNot::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x85BB30.
StateConditionInstance* StateConditionNot::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x85BBB0.
int StateConditionNot::_GetNumAllowedChildren() const {
    return 1;
}

// Reconstructed from eboot.elf at 0x85BBC0.
int StateConditionNot::_GetType() const {
    return kNot;
}

// Reconstructed from eboot.elf at 0x85BBD0.
bool StateConditionNot::Instance::_IsResultCurrent() const {
    return mResultCurrent;
}

// Reconstructed from eboot.elf at 0x7011C0. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateConditionNot::_Create() {
    return new StateConditionNot();
}

// Reconstructed from eboot.elf at 0x6FD0E0. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateConditionNot::Init() {
    InitComponentClass<StateConditionNot>();
}
