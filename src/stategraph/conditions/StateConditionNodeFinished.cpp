// stategraph/StateConditionNodeFinished.o (0x85AE10 to 0x85B307). The
// class's registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/conditions/StateConditionNodeFinished.h"

#include <new>

#include "entity/props/PropArray.h"
#include "stategraph/runtime/StateGraphInstance.h"

// The object's statics, in the order of its static initializer (0x85B250).
// The id of no object it first sets (0x1AC4178) comes from a shared header
// and is not used here.
Symbol StateConditionNodeFinished::sId("StateConditionNodeFinished");
Symbol StateConditionNodeFinished::sClassName("StateConditionNodeFinished");
PropRegistry StateConditionNodeFinished::sPropRegistry;
ComMetaData StateConditionNodeFinished::sMetaData;

// Reconstructed from eboot.elf at 0x85AE10.
void StateConditionNodeFinished::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateConditionNodeFinished prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    StateCondition::_InitAsSuperclass(registry, metadata);
    metadata.mDescription =
        "Implements the logical NOT (!) operator for complex StateGraph transition conditions.";
    metadata.mAuthor = "Matt Sharpe";
}

// Reconstructed from eboot.elf at 0x85AF50.
const char* StateConditionNodeFinished::_GetDescription() const {
    return "Is finished...";
}

// Reconstructed from eboot.elf at 0x85AF60.
StateConditionNodeFinished::Instance::Instance(const StateConditionNodeFinished& condition, StateGraphInstance& graph)
    : StateConditionInstance(condition, graph) {}

// Reconstructed from eboot.elf at 0x85AF90.
bool StateConditionNodeFinished::Instance::_IsMet(bool useCached) {
    static_cast<void>(useCached);
    if (!mGraph->IsScopeNodeFinished()) {
        return false;
    }
    mHeld = true;
    return true;
}

// Reconstructed from eboot.elf at 0x85AFE0.
Symbol StateConditionNodeFinished::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x85AFF0.
Symbol StateConditionNodeFinished::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x85B000.
int StateConditionNodeFinished::CurrentRev() const {
    return const_cast<StateConditionNodeFinished*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x85B020.
bool StateConditionNodeFinished::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x85B050.
Component* StateConditionNodeFinished::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x85B060. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateConditionNodeFinished::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateConditionNodeFinished*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateConditionNodeFinished(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x85B190.
PropRegistry& StateConditionNodeFinished::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x85B1A0.
ComMetaData& StateConditionNodeFinished::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x85B1B0.
StateConditionInstance* StateConditionNodeFinished::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x85B200.
int StateConditionNodeFinished::_GetType() const {
    return kNodeFinished;
}

// Reconstructed from eboot.elf at 0x701150. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateConditionNodeFinished::_Create() {
    return new StateConditionNodeFinished();
}

// Reconstructed from eboot.elf at 0x6FCD20. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateConditionNodeFinished::Init() {
    InitComponentClass<StateConditionNodeFinished>();
}
