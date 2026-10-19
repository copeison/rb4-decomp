// stategraph/StateConditionRandom.o (0x88E540 to 0x88F1A7). The class's
// registration and factory are emitted in stategraph/StateGraph.o.
#include "stategraph/conditions/StateConditionRandom.h"

#include <new>

#include "stategraph/runtime/StateGraphInstance.h"

// The object's statics, in the order of its static initializer (0x88F0F0),
// which sets the null object id at 0x1AC7598 first.
Symbol StateConditionRandom::sId("StateConditionRandom");
Symbol StateConditionRandom::sClassName("StateConditionRandom");
PropRegistry StateConditionRandom::sPropRegistry;
ComMetaData StateConditionRandom::sMetaData;

// Reconstructed from eboot.elf at 0x88E540. The condition defaults to
// "random value < 0.5".
StateConditionRandom::StateConditionRandom() {
    mOperator = kLess;
    mValues[0] = DrivenProp<float>(0.5F);
}

// Reconstructed from eboot.elf at 0x88E660. Comparisons saved before the
// registry's revision 1 had their values under other names.
void StateConditionRandom::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateConditionRandom prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&own != &registry) {
        registry.AddInheritedRegistry(sClassName, own);
    }
    // At 0x88F060.
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision = [](const PropRegistry::RevisionArgs& args) {
        if (args.mSavedRev <= 0) {
            static_cast<StateConditionRandom*>(args.mComponent)->_LoadOldValues(*args.mSavedProps);
        }
    };
    if (&own == &registry) {
        registry.SetCurrentRev(1);
        registry.mRevisionFunc = onRevision;
    }

    StateConditionComparison<float>::_InitAsSuperclass(registry, metadata);
    PropPath::Node member;
    PropPath::Node field;
    FloatMetadata& valueA = static_cast<FloatMetadata&>(
        *registry.FindProp(PropPath(member = Symbol("value_a"), field = Symbol("value")), nullptr)->mMetadata.Get());
    FloatMetadata& valueB = static_cast<FloatMetadata&>(
        *registry.FindProp(PropPath(member = Symbol("value_b"), field = Symbol("value")), nullptr)->mMetadata.Get());
    valueA.mMin = 0.0F;
    valueB.mMin = 0.0F;
    valueA.mMax = 1.0F;
    valueB.mMax = 1.0F;
    metadata.mDescription = "Enables randomization of transition selection based on a randomly generated number.";
    metadata.mAuthor = "Matt Sharpe";
}

// Reconstructed from eboot.elf at 0x88EBA0.
const char* StateConditionRandom::_GetValueToCompareGeneralDescription() const {
    return "randomly generated value";
}

// Reconstructed from eboot.elf at 0x88EBB0.
const char* StateConditionRandom::_GetValueToCompareDescription() const {
    return "Random value";
}

// Reconstructed from eboot.elf at 0x88EBC0.
bool StateConditionRandom::_AllowOperator(ComparisonOperator op) const {
    return static_cast<unsigned int>(op - kLess) < 5;
}

// Reconstructed from eboot.elf at 0x88EBD0.
StateConditionRandom::Instance::Instance(const StateConditionRandom& condition, StateGraphInstance& graph)
    : StateConditionComparison<float>::Instance(condition, graph) {}

// Reconstructed from eboot.elf at 0x88EC80.
bool StateConditionRandom::Instance::_GetValueToCompare(float& value) const {
    value = mGraph->GetRandom();
    return true;
}

// Reconstructed from eboot.elf at 0x88ECC0.
Symbol StateConditionRandom::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x88ECD0.
Symbol StateConditionRandom::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x88ECE0.
int StateConditionRandom::CurrentRev() const {
    return const_cast<StateConditionRandom*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x88ED00.
bool StateConditionRandom::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x88ED30.
Component* StateConditionRandom::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x88ED40. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateConditionRandom::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateConditionRandom*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateConditionRandom(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x88EE90.
PropRegistry& StateConditionRandom::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x88EEA0.
ComMetaData& StateConditionRandom::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x88EEB0.
StateConditionInstance* StateConditionRandom::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x88EF90.
int StateConditionRandom::_GetType() const {
    return kRandom;
}

// Reconstructed from eboot.elf at 0x7012A0. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateConditionRandom::_Create() {
    return new StateConditionRandom();
}

// Reconstructed from eboot.elf at 0x6FD890. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateConditionRandom::Init() {
    InitComponentClass<StateConditionRandom>();
}
