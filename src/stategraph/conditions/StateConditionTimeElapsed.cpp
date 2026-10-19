// stategraph/StateConditionTimeElapsed.o (0x8903F0 to 0x891387). The
// class's registration and factory are emitted in stategraph/StateGraph.o.
// GetAnimTypeAllowedValues (0x891390) and gTimeUnitsAllowedValues (0x891770)
// follow, with a static initializer of their own (0x891950).
#include "stategraph/conditions/StateConditionTimeElapsed.h"

#include <new>

#include "anim/char/CharAnimResource.h"
#include "anim/char/CharClipResource.h"
#include "anim/core/AnimResource.h"
#include "anim/drivers/AnimDriverCom.h"
#include "anim/props/PropAnimResource.h"
#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"
#include "stategraph/resources/StateGraphResource.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "utl/text/MakeString.h"
#include "utl/time/TimeMgr.h"

namespace {

// A new unit changes the description. Name not in the reference map.
// Reconstructed from eboot.elf at 0x8911B0.
void OnUnitsChanged(const PropAccessorArgs& args) {
    static_cast<StateBase*>(args.mComponent)->_SendDisplayTextUpdated();
}

}  // namespace

// The object's statics, in the order of its static initializer (0x8912D0),
// which sets the null object id at 0x1AC7A58 first.
Symbol StateConditionTimeElapsed::sId("StateConditionTimeElapsed");
Symbol StateConditionTimeElapsed::sClassName("StateConditionTimeElapsed");
PropRegistry StateConditionTimeElapsed::sPropRegistry;
ComMetaData StateConditionTimeElapsed::sMetaData;

// Reconstructed from eboot.elf at 0x891390.
eastl::vector<AllowedValue<Symbol>> GetAnimTypeAllowedValues() {
    eastl::vector<AllowedValue<Symbol>> values;
    values.emplace_back(AllowedValue<Symbol>{PropAnimResource::Id(), String("PropAnim"), String("A PropAnimResource.")});
    values.emplace_back(AllowedValue<Symbol>{CharAnimResource::Id(), String("CharAnim"), String("A CharAnimResource.")});
    values.emplace_back(AllowedValue<Symbol>{
        CharClipResource::Id(),
        String("CharClip"),
        String("A blendtree-compatible character animation clip, as defined by a CharClipCom.")});
    return values;
}

// Reconstructed from eboot.elf at 0x891770. The graph's prototype entity
// comes from the component's StateGraphResource.
std::function<eastl::vector<AllowedValue<int>>(const TagArgs&)> gTimeUnitsAllowedValues =
    [](const TagArgs& args) {
        EntityResource* const resource = args.mComponent.mObject->mEntity->mResource;
        StateGraphResource* const graph = resource != nullptr && resource->IsA(StateGraphResource::Id())
            ? static_cast<StateGraphResource*>(resource)
            : nullptr;
        const AnimDriverCom* const driver =
            graph->GetContextEntityPrototype()->GetRoot()->GetCom<AnimDriverCom>();
        if (driver != nullptr) {
            return driver->GetTimeUnitsAllowedValues();
        }
        return eastl::vector<AllowedValue<int>>();
    };

// Reconstructed from eboot.elf at 0x8903F0. The condition defaults to ">"
// and to the default units since the end of the incoming transition.
StateConditionTimeElapsed::StateConditionTimeElapsed() : mUnits(kNumTimeUnits), mSince(kEndOfIncomingTransition) {
    mOperator = kGreater;
}

// Reconstructed from eboot.elf at 0x890480. Comparisons saved before the
// registry's revision 1 had their values under other names.
void StateConditionTimeElapsed::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateConditionTimeElapsed prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    IntMetadata<int>& units = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("units", offsetof(StateConditionTimeElapsed, mUnits), kPropertyInt, 4).mMetadata.Get());
    units.mDefault = prototype.mUnits;
    static_cast<void>(Symbol("prop"));
    units.mHelp = String("The time units to be considered when checking the elapsed time.");
    units.mAllowedValues.mMode = TagBase<eastl::vector<AllowedValue<int>>>::kModeFunction;
    units.mAllowedValues.mFunction = gTimeUnitsAllowedValues;
    units.mOnChanged = OnUnitsChanged;

    IntMetadata<int>& since = static_cast<IntMetadata<int>&>(
        *registry.RegisterProp("type", offsetof(StateConditionTimeElapsed, mSince), kPropertyInt, 4).mMetadata.Get());
    since.mDefault = prototype.mSince;
    static_cast<void>(Symbol("prop"));
    since.mDisplayName = String("since");
    since.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            kStartOfIncomingTransition,
            String("Start of Incoming Transition"),
            String(
                "The amount of time that has elapsed since the transition into this node began; useful primarily "
                "for interrupting transitions after a certain period of time.")},
        AllowedValue<int>{
            kEndOfIncomingTransition,
            String("End of Incoming Transition"),
            String(
                "(default) The amount of time that has elapsed since the transition into this node ended; useful "
                "when you want to ensure a node has been fully blended in for some period of time before "
                "exiting.")}};

    // At 0x891240.
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision = [](const PropRegistry::RevisionArgs& args) {
        if (args.mSavedRev <= 0) {
            static_cast<StateConditionTimeElapsed*>(args.mComponent)->_LoadOldValues(*args.mSavedProps);
        }
    };
    if (&registry == &sPropRegistry) {
        sPropRegistry.SetCurrentRev(1);
        sPropRegistry.mRevisionFunc = onRevision;
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
    metadata.mDescription = "Enables time-based StateGraph transition sequencing.";
    metadata.mAuthor = "Matt Sharpe";
}

// Reconstructed from eboot.elf at 0x890C50.
const char* StateConditionTimeElapsed::_GetValueToCompareGeneralDescription() const {
    return "elapsed time";
}

// Reconstructed from eboot.elf at 0x890C60.
const char* StateConditionTimeElapsed::_GetValueToCompareDescription() const {
    int units = mUnits;
    if (units == kNumTimeUnits) {
        const AnimDriverCom* const driver = _GetContextEntityPrototype()->GetRoot()->GetCom<AnimDriverCom>();
        if (driver != nullptr) {
            units = driver->mDefaultTimeline;
        }
    }
    FormatString text("Elapsed %s");
    text << GetTimeUnitsName(static_cast<TimeUnits>(units), false);
    return text.Str();
}

// Reconstructed from eboot.elf at 0x890D50.
bool StateConditionTimeElapsed::_AllowOperator(ComparisonOperator op) const {
    return static_cast<unsigned int>(op - kLess) < 5;
}

// Reconstructed from eboot.elf at 0x890D60.
StateConditionTimeElapsed::Instance::Instance(const StateConditionTimeElapsed& condition, StateGraphInstance& graph)
    : StateConditionComparison<float>::Instance(condition, graph), mTimeElapsed(&condition) {}

// Reconstructed from eboot.elf at 0x890E10.
bool StateConditionTimeElapsed::Instance::_GetValueToCompare(float& value) const {
    const int units = mGraph->ResolveTimeUnits(mTimeElapsed->mUnits);
    value = mGraph->GetElapsedTime(mTimeElapsed->mSince, units);
    return true;
}

// Reconstructed from eboot.elf at 0x890E80.
Symbol StateConditionTimeElapsed::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x890E90.
Symbol StateConditionTimeElapsed::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x890EA0.
int StateConditionTimeElapsed::CurrentRev() const {
    return const_cast<StateConditionTimeElapsed*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x890EC0.
bool StateConditionTimeElapsed::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x890EF0.
Component* StateConditionTimeElapsed::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x890F00. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateConditionTimeElapsed::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateConditionTimeElapsed*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateConditionTimeElapsed(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x891060.
PropRegistry& StateConditionTimeElapsed::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x891070.
ComMetaData& StateConditionTimeElapsed::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x891080.
StateConditionInstance* StateConditionTimeElapsed::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x891160.
int StateConditionTimeElapsed::_GetType() const {
    return kTimeElapsed;
}

// Reconstructed from eboot.elf at 0x701340. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateConditionTimeElapsed::_Create() {
    return new StateConditionTimeElapsed();
}

// Reconstructed from eboot.elf at 0x6FE050. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateConditionTimeElapsed::Init() {
    InitComponentClass<StateConditionTimeElapsed>();
}
