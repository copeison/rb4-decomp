// stategraph/StateNodeTriggerAction.o (0x7F2E70 to 0x7F3A17), newer than
// the reference map. The class's registration and factory are emitted in
// stategraph/StateGraph.o.
#include "stategraph/nodes/StateNodeTriggerAction.h"

#include <new>

#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "stategraph/runtime/StateGraphInstance.h"

namespace {

// "action" redraws the node, whose text is the action's. Name not in the
// reference map.
// Reconstructed from eboot.elf at 0x7F3950.
void OnActionChanged(const PropAccessorArgs& args) {
    static_cast<StateNodeTriggerAction*>(args.mComponent)->_SendDisplayTextUpdated();
}

}  // namespace

// The object's statics, in the order of its static initializer (0x7F3960).
// The id of no object it first sets (0x1ABDBF8) comes from a shared header
// and is not used here.
Symbol StateNodeTriggerAction::sId("StateNodeTriggerAction");
Symbol StateNodeTriggerAction::sClassName("StateNodeTriggerAction");
PropRegistry StateNodeTriggerAction::sPropRegistry;
ComMetaData StateNodeTriggerAction::sMetaData;

// Reconstructed from eboot.elf at 0x7F2E70.
StateNodeTriggerAction::StateNodeTriggerAction() : mAction(), mPersistent(false) {}

// Reconstructed from eboot.elf at 0x7F2EB0. The action is looked up in the
// context entity and must be an action property, which may be neither
// read-only nor non-animatable.
void StateNodeTriggerAction::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeTriggerAction prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    PropRefMetadata& action = TypeSpecificMetadata(
        registry.RegisterProp("action", offsetof(StateNodeTriggerAction, mAction), kPropertyPropRef, sizeof(PropRef)),
        prototype.mAction);
    action.mRefDefault = prototype.mAction;
    static_cast<void>(Symbol("prop"));
    action.mHelp = String("The action triggered by this node.");
    action.mEntity.mMode = TagBase<Entity*>::kModeFunction;
    action.mEntity.mFunction = StateBase::_GetContextEntityTagFunc();
    action.mAllowedTypes = {kPropertyAction};
    action.mAllowsNonAnimatable = false;
    action.mAllowsReadOnly = false;
    action.mRefOnChanged = OnActionChanged;

    BoolMetadata& persistent = TypeSpecificMetadata(
        registry.RegisterProp("persistent", offsetof(StateNodeTriggerAction, mPersistent), kPropertyBool, 1),
        prototype.mPersistent);
    persistent.mDefault = prototype.mPersistent;
    static_cast<void>(Symbol("prop"));
    persistent.mHelp =
        String("If true, this action command will be triggered once per frame while this node is active.");

    StateNode::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "A node that instantly triggers an \"action\" property.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("t");
}

// Reconstructed from eboot.elf at 0x7F3430.
const char* StateNodeTriggerAction::GetEditorText() const {
    return mAction.ValueToString(_GetContextEntityPrototype());
}

// Reconstructed from eboot.elf at 0x7F3460.
StateNodeTriggerAction::Instance::Instance(const StateNodeTriggerAction& node, StateGraphInstance& graph)
    : StateNodeInstance(node, graph), mTriggerNode(&node) {}

// Reconstructed from eboot.elf at 0x7F3490. The action's arguments are
// built on the stack with the info after them; the evidence that they are
// a TagArgs is weak.
bool StateNodeTriggerAction::Instance::_Poll() {
    GameObject* object = nullptr;
    Component* com = nullptr;
    const PropInfo* info = nullptr;
    if (!mTriggerNode->mAction.Resolve(mGraph->mEntity, object, com, info) || info->mType != kPropertyAction) {
        return false;
    }
    const ActionMetadata& metadata = static_cast<const ActionMetadata&>(*info->mMetadata.Get());
    metadata.mAction(TagArgs(*com, mTriggerNode->mAction.mPath));
    return mTriggerNode->mPersistent;
}

// Reconstructed from eboot.elf at 0x7F3600.
Symbol StateNodeTriggerAction::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x7F3610.
Symbol StateNodeTriggerAction::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x7F3620.
int StateNodeTriggerAction::CurrentRev() const {
    return const_cast<StateNodeTriggerAction*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x7F3640.
bool StateNodeTriggerAction::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x7F3670.
Component* StateNodeTriggerAction::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x7F3680. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateNodeTriggerAction::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy =
        reinterpret_cast<StateNodeTriggerAction*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeTriggerAction(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x7F3890.
PropRegistry& StateNodeTriggerAction::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x7F38A0.
ComMetaData& StateNodeTriggerAction::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x7F38B0.
StateNodeInstance* StateNodeTriggerAction::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x7F3910. The deleting destructor at
// 0x7F3920 returns the instance to the pool.
StateNodeTriggerAction::Instance::~Instance() {}

// Reconstructed from eboot.elf at 0x700570. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodeTriggerAction::_Create() {
    return new StateNodeTriggerAction();
}

// Reconstructed from eboot.elf at 0x6FA820. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodeTriggerAction::Init() {
    InitComponentClass<StateNodeTriggerAction>();
}
