// stategraph/StateConditionObjectDistance.o (0x85BCD0 to 0x85D627), newer
// than the reference map. The class's registration and factory are emitted
// in stategraph/StateGraph.o.
#include "stategraph/conditions/StateConditionObjectDistance.h"

#include <math.h>
#include <new>

#include "entity/core/Entity.h"
#include "entity/core/TransCom.h"
#include "os/memory/MemMgr.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "utl/text/MakeString.h"

namespace {

// The id of no object, at 0x1AC4638, which the static initializer sets
// first. Name not in the reference map.
GameObjectId gNullObjectId = {0xFFFFFFFFU};

// The TransCom of the entity's object with the id, or null. Inlined into
// _GetValueToCompare. Name not in the reference map.
const TransCom* GetObjectTrans(const Entity* entity, GameObjectId id) {
    if (id.mId == gNullObjectId.mId) {
        return nullptr;
    }
    const GameObject* const object = entity->GetObject(id);
    return object != nullptr ? object->GetCom<TransCom>() : nullptr;
}

// A new object changes the description. Names not in the reference map.
// Reconstructed from eboot.elf at 0x85D110.
void OnObjectAChanged(const PropAccessorArgs& args) {
    static_cast<StateBase*>(args.mComponent)->_SendDisplayTextUpdated();
}

// Reconstructed from eboot.elf at 0x85D120.
void OnObjectBChanged(const PropAccessorArgs& args) {
    static_cast<StateBase*>(args.mComponent)->_SendDisplayTextUpdated();
}

}  // namespace

// The object's statics, in the order of its static initializer (0x85D570).
Symbol StateConditionObjectDistance::sId("StateConditionObjectDistance");
Symbol StateConditionObjectDistance::sClassName("StateConditionObjectDistance");
PropRegistry StateConditionObjectDistance::sPropRegistry;
ComMetaData StateConditionObjectDistance::sMetaData;

// Reconstructed from eboot.elf at 0x85BCD0.
StateConditionObjectDistance::StateConditionObjectDistance()
    : mObjA{0xFFFFFFFFU}, mObjB{0xFFFFFFFFU}, mComparisonModes{kSpecificValue, kSpecificValue} {}

// Reconstructed from eboot.elf at 0x85BD50. The modes follow the property
// comparisons': they sit before the values, which show in the
// "Specific value" mode only.
void StateConditionObjectDistance::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateConditionObjectDistance prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    GameObjectIdMetadata& objA = TypeSpecificMetadata(
        registry.RegisterProp("obj_a", offsetof(StateConditionObjectDistance, mObjA), kPropertyGameObjectId, 4),
        prototype.mObjA);
    objA.mDefault = prototype.mObjA;
    static_cast<void>(Symbol("prop"));
    objA.mHelp = String("First object for distance check.");
    objA.mEntity.mMode = TagBase<Entity*>::kModeFunction;
    objA.mEntity.mFunction = StateBase::_GetContextEntityTagFunc();
    objA.mOnChanged = OnObjectAChanged;

    GameObjectIdMetadata& objB = TypeSpecificMetadata(
        registry.RegisterProp("obj_b", offsetof(StateConditionObjectDistance, mObjB), kPropertyGameObjectId, 4),
        prototype.mObjB);
    objB.mDefault = prototype.mObjB;
    static_cast<void>(Symbol("prop"));
    objB.mHelp = String("Second object for distance check.");
    objB.mEntity.mMode = TagBase<Entity*>::kModeFunction;
    objB.mEntity.mFunction = StateBase::_GetContextEntityTagFunc();
    objB.mOnChanged = OnObjectBChanged;

    eastl::vector<AllowedValue<int>> modes;
    unsigned int temp;
    MemPushTemp(temp, true, true);
    modes.push_back(AllowedValue<int>{
        kSpecificValue,
        String("Specific value"),
        String("Compare the target property against a specific value or the value of another property.")});
    modes.push_back(AllowedValue<int>{
        kInitialValue,
        String("Initial value"),
        String("Compare the target property against its own value as it was the moment this node was entered.")});
    modes.push_back(AllowedValue<int>{
        kMostRecentValue,
        String("Most recent value"),
        String("Compare the target property against its own most recent value (i.e. its value last frame).")});
    MemPopTemp(temp);

    IntMetadata<int>& modeA = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "comparison_mode_a", offsetof(StateConditionObjectDistance, mComparisonModes), kPropertyInt, 4)
                                                                   .mMetadata.Get());
    modeA.mDefault = prototype.mComparisonModes[0];
    static_cast<void>(Symbol("prop"));
    modeA.mHelp = String("Comparison mode for the value to be compared against.");
    // At 0x85D1B0.
    modeA.mDisplayName = std::function<String(const TagArgs&)>([](const TagArgs& args) {
        const StateConditionObjectDistance& condition =
            static_cast<const StateConditionObjectDistance&>(args.mComponent);
        return String(condition.mOperator == kBetween ? "compare against (a)" : "compare against");
    });
    modeA.mAllowedValues = modes;

    IntMetadata<int>& modeB = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "comparison_mode_b",
        offsetof(StateConditionObjectDistance, mComparisonModes) + sizeof(int),
        kPropertyInt,
        4).mMetadata.Get());
    modeB.mDefault = prototype.mComparisonModes[1];
    static_cast<void>(Symbol("prop"));
    modeB.mHelp = String("Comparison mode for the secondary value to be compared against.");
    modeB.mDisplayName = String("compare against (b)");
    modeB.mAllowedValues = modes;
    // At 0x85D2E0.
    modeB.mIsHidden = std::function<bool(const TagArgs&)>([](const TagArgs& args) {
        return static_cast<const StateConditionObjectDistance&>(args.mComponent).mOperator != kBetween;
    });

    StateConditionComparison<float>::_InitAsSuperclass(registry, metadata);
    registry.MoveProp(Symbol("comparison_mode_a"), Symbol("value_a"));
    registry.MoveProp(Symbol("comparison_mode_b"), Symbol("value_b"));
    // At 0x85D3E0.
    registry.FindProp(PropPath(Symbol("value_a")), nullptr)
        ->mMetadata->mIsHidden._Set(std::function<bool(const TagArgs&)>([](const TagArgs& args) {
            return static_cast<const StateConditionObjectDistance&>(args.mComponent).mComparisonModes[0] !=
                kSpecificValue;
        }));
    // At 0x85D4E0.
    registry.FindProp(PropPath(Symbol("value_b")), nullptr)
        ->mMetadata->mIsHidden._Set(std::function<bool(const TagArgs&)>([](const TagArgs& args) {
            const StateConditionObjectDistance& condition =
                static_cast<const StateConditionObjectDistance&>(args.mComponent);
            if (condition.mOperator != kBetween) {
                return true;
            }
            return condition.mComparisonModes[1] != kSpecificValue;
        }));
    metadata.mDescription = "Compares the distance between two objects against a specific value or another property.";
    metadata.mAuthor = "Don Wurster";
}

// Reconstructed from eboot.elf at 0x85C9E0.
const char* StateConditionObjectDistance::_GetValueToCompareGeneralDescription() const {
    return "distance";
}

// Reconstructed from eboot.elf at 0x85C9F0.
const char* StateConditionObjectDistance::_GetValueToCompareDescription() const {
    FormatString text("distance((%d)->(%d))");
    text << (mObjA.mId >> 16) << (mObjB.mId >> 16);
    return text.Str();
}

// Reconstructed from eboot.elf at 0x85CA80.
bool StateConditionObjectDistance::_HasValueToCompare() const {
    if (mObjA.mId == gNullObjectId.mId) {
        return false;
    }
    return mObjB.mId != gNullObjectId.mId;
}

// Reconstructed from eboot.elf at 0x85CAA0.
const char* StateConditionObjectDistance::_GetComparisonValueDescriptionOverride(
    unsigned long index,
    Symbol name) const {
    static_cast<void>(name);
    const int mode = mComparisonModes[index];
    if (mode == kInitialValue) {
        return "initial value";
    }
    if (mode == kMostRecentValue) {
        return "most recent value";
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x85CAD0.
StateConditionObjectDistance::Instance::Instance(
    const StateConditionObjectDistance& condition,
    StateGraphInstance& graph)
    : StateConditionComparison<float>::Instance(condition, graph), mObjectDistance(&condition) {
    Instance::_GetValueToCompare(mInitialDistance);
    mRecentDistance = mInitialDistance;
}

// Reconstructed from eboot.elf at 0x85CB90. The binary's square root is
// the fast-math expansion of sqrtf.
bool StateConditionObjectDistance::Instance::_GetValueToCompare(float& value) const {
    const StateConditionObjectDistance& condition = *mObjectDistance;
    if (condition.mObjA.mId == gNullObjectId.mId || condition.mObjB.mId == gNullObjectId.mId) {
        return false;
    }
    const TransCom* const transA = GetObjectTrans(_GetContextEntity(), condition.mObjA);
    const TransCom* const transB = GetObjectTrans(_GetContextEntity(), condition.mObjB);
    if (transA == nullptr || transB == nullptr) {
        return false;
    }
    const float dx = transB->mWorldXfm.v.x - transA->mWorldXfm.v.x;
    const float dy = transB->mWorldXfm.v.y - transA->mWorldXfm.v.y;
    const float dz = transB->mWorldXfm.v.z - transA->mWorldXfm.v.z;
    value = sqrtf(dy * dy + dx * dx + dz * dz);
    return true;
}

// Reconstructed from eboot.elf at 0x85CD70.
bool StateConditionObjectDistance::Instance::_OverrideComparisonValue(unsigned long index, float& value) const {
    const int mode = mObjectDistance->mComparisonModes[index];
    if (mode == kMostRecentValue) {
        value = mRecentDistance;
        return true;
    }
    if (mode == kInitialValue) {
        value = mInitialDistance;
        return true;
    }
    return false;
}

// Reconstructed from eboot.elf at 0x85CDA0.
void StateConditionObjectDistance::Instance::_ResetPerFrameState() {
    _GetValueToCompare(mRecentDistance);
}

// Reconstructed from eboot.elf at 0x85CDD0.
Symbol StateConditionObjectDistance::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x85CDE0.
Symbol StateConditionObjectDistance::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x85CDF0.
int StateConditionObjectDistance::CurrentRev() const {
    return const_cast<StateConditionObjectDistance*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x85CE10.
bool StateConditionObjectDistance::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x85CE40.
Component* StateConditionObjectDistance::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x85CE50. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
char* StateConditionObjectDistance::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy =
        reinterpret_cast<StateConditionObjectDistance*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateConditionObjectDistance(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x85CFB0.
PropRegistry& StateConditionObjectDistance::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x85CFC0.
ComMetaData& StateConditionObjectDistance::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x85CFD0.
StateConditionInstance* StateConditionObjectDistance::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x85D0C0.
int StateConditionObjectDistance::_GetType() const {
    return kDistanceCheck;
}

// Reconstructed from eboot.elf at 0x701250. The binary emits the factory in
// stategraph/StateGraph.o with the class's Init.
Component* StateConditionObjectDistance::_Create() {
    return new StateConditionObjectDistance();
}

// Reconstructed from eboot.elf at 0x6FD4B0. The binary emits the
// registration in stategraph/StateGraph.o, which calls it.
void StateConditionObjectDistance::Init() {
    InitComponentClass<StateConditionObjectDistance>();
}
