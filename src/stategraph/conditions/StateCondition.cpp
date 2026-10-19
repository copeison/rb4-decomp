// stategraph/StateCondition.o (0x808AE0 to 0x809A92). The class's
// registration and factory are emitted in stategraph/StateGraph.o. The
// object also initializes the class symbols of the float comparison
// templates (0x8098C0, 0x809950), which are not reconstructed.
#include "stategraph/conditions/StateCondition.h"

#include <new>

#include "entity/core/Entity.h"
#include "entity/props/PropMetadata.h"
#include "os/memory/MemMgr.h"
#include "stategraph/conditions/StateConditionEvent.h"
#include "stategraph/conditions/StateConditionEventParameter.h"
#include "stategraph/conditions/StateConditionLogic.h"
#include "stategraph/conditions/StateConditionNodeFinished.h"
#include "stategraph/conditions/StateConditionNot.h"
#include "stategraph/conditions/StateConditionObjectDistance.h"
#include "stategraph/conditions/StateConditionPropertyComparison.h"
#include "stategraph/conditions/StateConditionRandom.h"
#include "stategraph/conditions/StateConditionSoundProgress.h"
#include "stategraph/conditions/StateConditionTimeElapsed.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "utl/text/MakeString.h"

namespace {

// The id of no object, at 0x1ABF060, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The editor names of the condition types (0x1950A10). Name not in the
// reference map.
const char* const kTypeNames[StateCondition::kNumTypes] = {
    "Time Elapsed",
    "Node Finished",
    "All of the following...",
    "Any of the following...",
    "Not",
    "Property Comparison",
    "Random",
    "Event",
    "Parameter Comparison",
    "Distance Check",
    "Sound Progress",
};

}  // namespace

// The object's statics, in the order of its static initializer (0x8099E0).
Symbol StateCondition::sId("StateCondition");
Symbol StateCondition::sClassName("StateCondition");
PropRegistry StateCondition::sPropRegistry;
ComMetaData StateCondition::sMetaData;

// Reconstructed from eboot.elf at 0x808AE0.
StateCondition::StateCondition() : mEvaluationMode(kReevaluateEveryFrame) {}

// Reconstructed from eboot.elf at 0x808B20.
void StateCondition::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateCondition prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    IntMetadata<int>& mode = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "evaluation_mode", offsetof(StateCondition, mEvaluationMode), kPropertyInt, 4).mMetadata.Get());
    mode.mDefault = prototype.mEvaluationMode;
    static_cast<void>(Symbol("prop"));
    mode.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            kReevaluateEveryFrame,
            String("Reevaluate Every Frame"),
            String(
                "(default) This condition will be reevaluated every frame and the result will persist for that "
                "frame only.")},
        AllowedValue<int>{
            kEvaluateExactlyOnce,
            String("Evaluate Exactly Once"),
            String(
                "This condition will be evaluated exactly once and the result will persist for its entire "
                "lifetime.")},
        AllowedValue<int>{
            kOnceTrueRemainTrue,
            String("Once True, Remain True"),
            String(
                "This condition will be evaluated until it becomes true; from then on it will remain true for "
                "its remaining lifetime regardless of the underlying state.")},
        AllowedValue<int>{
            kOnceFalseRemainFalse,
            String("Once False, Remain False"),
            String(
                "This condition will be evaluated until it becomes false; from then on it will remain false for "
                "its remaining lifetime regardless of the underlying state.")}};

    StateBase::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Base class for conditional statements within StateGraph transitions.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mInterface = sClassName;
}

// Reconstructed from eboot.elf at 0x808FA0. The logical types share
// StateConditionLogic and set its operator; the comparisons create the
// float instantiations.
StateCondition* StateCondition::Create(GameObject* object, Type type) {
    Symbol className;
    switch (type) {
    case kTimeElapsed:
        className = StateConditionTimeElapsed::sClassName;
        break;
    case kNodeFinished:
        className = StateConditionNodeFinished::sClassName;
        break;
    case kAllOf:
    case kAnyOf: {
        StateConditionLogic* const logic =
            static_cast<StateConditionLogic*>(object->CreateComponent(StateConditionLogic::sClassName, false));
        logic->mOperator = type;
        return logic;
    }
    case kNot:
        className = StateConditionNot::sClassName;
        break;
    case kPropertyComparison:
        className = StateConditionPropertyComparison<float>::sClassName;
        break;
    case kRandom:
        className = StateConditionRandom::sClassName;
        break;
    case kEvent:
        className = StateConditionEvent::sClassName;
        break;
    case kParameterComparison:
        className = StateConditionEventParameter<float>::sId;
        break;
    case kDistanceCheck:
        className = StateConditionObjectDistance::sClassName;
        break;
    case kSoundProgress:
        className = StateConditionSoundProgress::sClassName;
        break;
    default:
        return nullptr;
    }
    return static_cast<StateCondition*>(object->CreateComponent(className, false));
}

// Reconstructed from eboot.elf at 0x809040.
const char* StateCondition::GetTypeName(Type type) {
    if (static_cast<unsigned int>(type) >= kNumTypes) {
        return "";
    }
    return kTypeNames[type];
}

// Reconstructed from eboot.elf at 0x809060.
StateConditionInstance* StateCondition::CreateInstance(StateGraphInstance& graph) const {
    return _CreateInstance(graph);
}

// Reconstructed from eboot.elf at 0x809070.
int StateCondition::GetType() const {
    return _GetType();
}

// Reconstructed from eboot.elf at 0x809080.
bool StateCondition::IsValid(const StateCondition* parent, String* error) const {
    return _IsValid(parent, error);
}

// Reconstructed from eboot.elf at 0x809090.
int StateCondition::GetNumChildren() const {
    return _GetNumChildren();
}

// Reconstructed from eboot.elf at 0x8090A0.
int StateCondition::GetNumAllowedChildren() const {
    return _GetNumAllowedChildren();
}

// Reconstructed from eboot.elf at 0x8090B0.
void StateCondition::GetAllowedChildTypes(eastl::vector<int>& types) {
    _GetAllowedChildTypes(types);
    if (types.empty()) {
        for (int type = 0; type != kNumTypes; ++type) {
            types.push_back(type);
        }
    }
    eastl::vector<int> disallowed;
    _GetDisallowedChildTypes(disallowed);
    if (disallowed.empty()) {
        return;
    }
    auto isDisallowed = [&disallowed](int type) {
        for (int other : disallowed) {
            if (other == type) {
                return true;
            }
        }
        return false;
    };
    int* kept = types.begin();
    while (kept != types.end() && !isDisallowed(*kept)) {
        ++kept;
    }
    if (kept == types.end()) {
        return;
    }
    for (int* type = kept + 1; type != types.end(); ++type) {
        if (!isDisallowed(*type)) {
            *kept++ = *type;
        }
    }
    types.erase(kept, types.end());
}

// Reconstructed from eboot.elf at 0x8092F0.
void StateCondition::AddChild(GameObjectId child) {
    _AddChild(child);
}

// Reconstructed from eboot.elf at 0x809300.
void StateCondition::RemoveChild(GameObjectId child) {
    _RemoveChild(child);
}

// Reconstructed from eboot.elf at 0x809310. The child's object must exist.
StateCondition* StateCondition::GetChildCondition(unsigned int index) const {
    Entity* const entity = mObject->mEntity;
    const GameObjectId child = _GetChild(index);
    if (sClassName == Symbol()) {
        return nullptr;
    }
    return entity->GetObject(child)->GetBaseCom<StateCondition>();
}

// Reconstructed from eboot.elf at 0x8093A0.
const char* StateCondition::GetDescription() const {
    return _GetDescription();
}

// Reconstructed from eboot.elf at 0x8093B0.
bool StateCondition::GetChildren(eastl::vector<GameObjectId>& children) const {
    _GetChildren(children, false);
    return !children.empty();
}

// Reconstructed from eboot.elf at 0x8093E0.
void StateCondition::GetDescendants(eastl::vector<GameObjectId>& descendants) const {
    _GetChildren(descendants, true);
}

// Reconstructed from eboot.elf at 0x809400.
StateConditionInstance* StateCondition::_CreateInstance(StateGraphInstance& graph) const {
    static_cast<void>(graph);
    return nullptr;
}

// Reconstructed from eboot.elf at 0x809410.
int StateCondition::_GetType() const {
    return kNumTypes;
}

// Reconstructed from eboot.elf at 0x809420.
StateConditionInstance::StateConditionInstance(const StateCondition& condition, StateGraphInstance& graph)
    : mCondition(&condition), mGraph(&graph), mResult(false), mHeld(false) {}

// Reconstructed from eboot.elf at 0x809460. Without `useCached`, or when
// the result is not current, the condition is evaluated again.
bool StateConditionInstance::IsMet(bool useCached) {
    if (mHeld || (useCached && mCondition->mEvaluationMode == StateCondition::kReevaluateEveryFrame &&
                  _IsResultCurrent())) {
        return mResult;
    }
    const bool result = _IsMet(useCached);
    mResult = result;
    const int mode = mCondition->mEvaluationMode;
    bool held = true;
    if (mode != StateCondition::kEvaluateExactlyOnce && !mHeld) {
        const bool unmet = !result;
        if (mode != StateCondition::kOnceTrueRemainTrue || unmet) {
            held = unmet && mode == StateCondition::kOnceFalseRemainFalse;
        }
    }
    mHeld = held;
    return result;
}

// Reconstructed from eboot.elf at 0x8094E0.
bool StateConditionInstance::IsResultCurrent() const {
    if (mCondition->mEvaluationMode != StateCondition::kReevaluateEveryFrame) {
        return false;
    }
    return _IsResultCurrent();
}

// Reconstructed from eboot.elf at 0x809500.
void StateConditionInstance::ResetPerFrameState() {
    _ResetPerFrameState();
}

// Reconstructed from eboot.elf at 0x809510.
bool StateConditionInstance::UsesObject(GameObjectId object) const {
    if (mCondition->mObject->mId.mId == object.mId) {
        return true;
    }
    return _UsesObject(object);
}

// Reconstructed from eboot.elf at 0x809530.
bool StateConditionInstance::_UsesObject(GameObjectId object) const {
    static_cast<void>(object);
    return false;
}

// Reconstructed from eboot.elf at 0x809540.
bool StateConditionInstance::OnEvent(DataArray* event) {
    return _OnEvent(event);
}

// Reconstructed from eboot.elf at 0x809550. The report is compiled out;
// its arguments remain.
bool StateConditionInstance::_OnEvent(DataArray* event) {
    static_cast<void>(event);
    static_cast<void>(mCondition->MakeErrorName());
    return false;
}

// Reconstructed from eboot.elf at 0x809570.
Entity* StateConditionInstance::_GetContextEntity() const {
    return mGraph->mEntity;
}

// Reconstructed from eboot.elf at 0x8095A0.
Symbol StateCondition::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x8095B0.
Symbol StateCondition::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x8095C0.
int StateCondition::CurrentRev() const {
    return const_cast<StateCondition*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x8095E0.
bool StateCondition::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x809610.
Component* StateCondition::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x809620. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateCondition::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateCondition*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateCondition(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x809750.
PropRegistry& StateCondition::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x809760.
ComMetaData& StateCondition::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x809770.
int StateCondition::_GetNumAllowedChildren() const {
    return 0;
}

// Reconstructed from eboot.elf at 0x809780.
int StateCondition::_GetNumChildren() const {
    return 0;
}

// Reconstructed from eboot.elf at 0x809790. The report is compiled out.
void StateCondition::_AddChild(GameObjectId child) {
    static_cast<void>(child);
    static_cast<void>(GetId());
}

// Reconstructed from eboot.elf at 0x8097A0. The report is compiled out.
void StateCondition::_RemoveChild(GameObjectId child) {
    static_cast<void>(child);
    static_cast<void>(GetId());
}

// Reconstructed from eboot.elf at 0x8097B0. The report is compiled out.
GameObjectId StateCondition::_GetChild(unsigned int index) const {
    static_cast<void>(index);
    static_cast<void>(GetId());
    return gNullObjectId;
}

// Reconstructed from eboot.elf at 0x8097F0.
const char* StateCondition::_GetDescription() const {
    FormatString description("<%s - implement _GetDescription>");
    description << GetId();
    return description.Str();
}

// Reconstructed from eboot.elf at 0x809880.
bool StateCondition::_IsValid(const StateCondition* parent, String* error) const {
    static_cast<void>(parent);
    static_cast<void>(error);
    return true;
}

// Reconstructed from eboot.elf at 0x809890.
bool StateConditionInstance::_IsMet(bool useCached) {
    static_cast<void>(useCached);
    return true;
}

// Reconstructed from eboot.elf at 0x8098B0.
bool StateConditionInstance::_IsResultCurrent() const {
    return true;
}

// Reconstructed from eboot.elf at 0x80D9E0. The binary emits it outside
// StateCondition.o, among the comparison conditions.
void StateCondition::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("StateCondition", sMetaData);
    const ComMetaData::EditorRestriction restrictions =
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions);
    static const long heap = MemFindHeap("metadata");
    MemPushHeap(heap);
    _Init(registry, metadata);
    MemPopHeap();
    metadata.mEditorRestrictions = ComMetaData::InheritEditorRestrictions(
        static_cast<ComMetaData::EditorRestriction>(metadata.mEditorRestrictions),
        restrictions);
}

// Reconstructed from eboot.elf at 0x700EC0. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateCondition::_Create() {
    return new StateCondition();
}

// Reconstructed from eboot.elf at 0x6FC060. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateCondition::Init() {
    InitComponentClass<StateCondition>();
}
