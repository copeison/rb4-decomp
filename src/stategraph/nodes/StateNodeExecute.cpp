// stategraph/StateNodeExecute.o (0x718F00 to 0x719A37). The class's
// registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/nodes/StateNodeExecute.h"

#include <new>

#include "entity/props/PropMetadata.h"
#include "utl/data/DataFile.h"

namespace {

// "cmd" redraws the node, whose text is the command. Name not in the
// reference map.
// Reconstructed from eboot.elf at 0x719970.
void OnCmdChanged(const PropAccessorArgs& args) {
    static_cast<StateNodeExecute*>(args.mComponent)->_SendDisplayTextUpdated();
}

}  // namespace

// The object's statics, in the order of its static initializer (0x719980).
// The id of no object it first sets (0x1AB3938) comes from a shared header
// and is not used here.
Symbol StateNodeExecute::sId("StateNodeExecute");
Symbol StateNodeExecute::sClassName("StateNodeExecute");
PropRegistry StateNodeExecute::sPropRegistry;
ComMetaData StateNodeExecute::sMetaData;

// Reconstructed from eboot.elf at 0x718F00.
StateNodeExecute::StateNodeExecute() : mCmd(), mPersistent(false) {}

// Reconstructed from eboot.elf at 0x718F40.
void StateNodeExecute::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeExecute prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    SymbolMetadata& cmd = TypeSpecificMetadata(
        registry.RegisterProp("cmd", offsetof(StateNodeExecute, mCmd), kPropertySymbol, sizeof(Symbol)),
        prototype.mCmd);
    cmd.mDefault = prototype.mCmd;
    static_cast<void>(Symbol("prop"));
    cmd.mHelp = String("The script command fired by this node.");
    cmd.mOnChanged = OnCmdChanged;

    BoolMetadata& persistent = TypeSpecificMetadata(
        registry.RegisterProp("persistent", offsetof(StateNodeExecute, mPersistent), kPropertyBool, 1),
        prototype.mPersistent);
    persistent.mDefault = prototype.mPersistent;
    static_cast<void>(Symbol("prop"));
    persistent.mHelp =
        String("If true, this script command will be executed once per frame while this state is active.");

    StateNode::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "A state that instantly executes a script command.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("e");
}

// Reconstructed from eboot.elf at 0x7193A0.
const char* StateNodeExecute::GetEditorText() const {
    return mCmd.Str();
}

// Reconstructed from eboot.elf at 0x7193B0.
StateNodeExecute::Instance::Instance(const StateNodeExecute& node, StateGraphInstance& graph)
    : StateNodeInstance(node, graph), mCommand(DataReadString(node.mCmd.Str())), mExecuteNode(&node) {}

// Reconstructed from eboot.elf at 0x7193F0.
void StateNodeExecute::Instance::_Enter() {
    if (!mExecuteNode->mPersistent) {
        DataExecute(mCommand.mData);
    }
}

// Reconstructed from eboot.elf at 0x719490.
bool StateNodeExecute::Instance::_Poll() {
    if (mExecuteNode->mPersistent) {
        DataExecute(mCommand.mData);
    }
    return mExecuteNode->mPersistent;
}

// Reconstructed from eboot.elf at 0x7195E0.
Symbol StateNodeExecute::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x7195F0.
Symbol StateNodeExecute::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x719600.
int StateNodeExecute::CurrentRev() const {
    return const_cast<StateNodeExecute*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x719620.
bool StateNodeExecute::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x719650.
Component* StateNodeExecute::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x719660. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateNodeExecute::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodeExecute*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeExecute(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x719820.
PropRegistry& StateNodeExecute::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x719830.
ComMetaData& StateNodeExecute::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x719840.
StateNodeInstance* StateNodeExecute::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x7198B0. The deleting destructor at
// 0x719910 returns the instance to the pool.
StateNodeExecute::Instance::~Instance() {}

// Reconstructed from eboot.elf at 0x6FFD30. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodeExecute::_Create() {
    return new StateNodeExecute();
}

// Reconstructed from eboot.elf at 0x6F9250. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodeExecute::Init() {
    InitComponentClass<StateNodeExecute>();
}
