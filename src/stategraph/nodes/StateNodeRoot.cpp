// stategraph/StateNodeRoot.o (0x78CF90 to 0x78DA27). The class's
// registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/nodes/StateNodeRoot.h"

#include <new>

#include "entity/props/PropMetadata.h"

namespace {

// "start_on_enter" sets the flag through the root, which redraws it. Name
// not in the reference map.
// Reconstructed from eboot.elf at 0x78D950.
void SetStartOnEnterAccessor(const PropAccessorArgs& args) {
    static_cast<StateNodeRoot*>(args.mComponent)->SetStartOnEnter(*static_cast<const bool*>(args.mValue));
}

}  // namespace

// The object's statics, in the order of its static initializer (0x78D970).
// The id of no object it first sets (0x1AB91A8) comes from a shared header
// and is not used here.
Symbol StateNodeRoot::sId("StateNodeRoot");
Symbol StateNodeRoot::sClassName("StateNodeRoot");
PropRegistry StateNodeRoot::sPropRegistry;
ComMetaData StateNodeRoot::sMetaData;

// Reconstructed from eboot.elf at 0x78CF90.
StateNodeRoot::StateNodeRoot() : mStartOnEnter(true), mPollFrequency(kPollAlways) {}

// Reconstructed from eboot.elf at 0x78CFD0. The root may not be created in
// the editor.
void StateNodeRoot::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeRoot prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }

    BoolMetadata& startOnEnter = TypeSpecificMetadata(
        registry.RegisterProp("start_on_enter", offsetof(StateNodeRoot, mStartOnEnter), kPropertyBool, 1),
        prototype.mStartOnEnter);
    startOnEnter.mDefault = prototype.mStartOnEnter;
    static_cast<void>(Symbol("prop"));
    startOnEnter.mHelp = String(
        "Should this StateGraph start immediately when its context entity is entered? If not, the state graph "
        "will remain inactive until started by code or script, or until activated by a SubGraph node.");
    startOnEnter.mSetter = SetStartOnEnterAccessor;

    IntMetadata<int>& pollFrequency = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "poll_frequency", offsetof(StateNodeRoot, mPollFrequency), kPropertyInt, 4).mMetadata.Get());
    pollFrequency.mDefault = prototype.mPollFrequency;
    static_cast<void>(Symbol("prop"));
    pollFrequency.mHelp =
        String("Should this stategraph poll every frame, or halve its workload by polling every other frame?");
    pollFrequency.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{kPollAlways, String("Always Poll"), String("Poll every frame")},
        AllowedValue<int>{kPollEveryOtherFrame, String("Poll Every Other Frame"), String("Poll every other frame")}};

    StateNode::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Root node of a StateGraph, which acts as its initial state.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mEditorAttributes[Symbol("state_creatable_node")] = Symbol("false");
}

// Reconstructed from eboot.elf at 0x78D610.
void StateNodeRoot::SetStartOnEnter(bool startOnEnter) {
    if (startOnEnter != mStartOnEnter) {
        mStartOnEnter = startOnEnter;
        _SendDisplayTextUpdated();
    }
}

// Reconstructed from eboot.elf at 0x78D630.
const char* StateNodeRoot::GetEditorText() const {
    return mStartOnEnter ? "Start on Enter" : "";
}

// Reconstructed from eboot.elf at 0x78D6F0.
Symbol StateNodeRoot::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x78D700.
Symbol StateNodeRoot::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x78D710.
int StateNodeRoot::CurrentRev() const {
    return const_cast<StateNodeRoot*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x78D730.
bool StateNodeRoot::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x78D760.
Component* StateNodeRoot::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x78D770. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateNodeRoot::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodeRoot*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeRoot(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x78D920.
PropRegistry& StateNodeRoot::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x78D930.
ComMetaData& StateNodeRoot::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x78D940.
bool StateNodeRoot::_HasActivity() const {
    return false;
}

// Reconstructed from eboot.elf at 0x6FEEE0. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateNodeRoot::_Create() {
    return new StateNodeRoot();
}

// Reconstructed from eboot.elf at 0x6F6F50. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateNodeRoot::Init() {
    InitComponentClass<StateNodeRoot>();
}
