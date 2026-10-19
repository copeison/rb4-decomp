#pragma once

#include <cstddef>
#include <cstdio>
#include <functional>
#include <new>

#include "entity/core/ComMetaData.h"
#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropRegistry.h"
#include "entity/props/PropUtl.h"
#include "os/memory/MemMgr.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/conditions/StateCondition.h"
#include "stategraph/conditions/StateConditionComparison.h"
#include "stategraph/props/DrivenProp.h"
#include "utl/containers/Vector.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// The condition that compares a property of a component of the graph's
// entity with comparison values (stategraph/StateConditionPropertyComparisonInit_A.o
// to _C.o; class id "StateConditionPropertyComparison_<type>"), the
// "Property Comparison" type of StateCondition::Create, which makes the
// float one. Each comparison value is the condition's own (a driven
// value), or the property's value when the node was entered or in the last
// frame. The vtable has 59 slots (0x19578F8 for float). The object is 736
// bytes for float.
template <class T>
class StateConditionPropertyComparison : public StateConditionComparison<T> {
public:
    using Base = StateConditionComparison<T>;

    // What a comparison value is compared against ("comparison_mode_a" and
    // "comparison_mode_b"). The labels are the allowed values'; the names
    // are not in the reference map.
    enum ComparisonMode : int {
        kSpecificValue = 0,
        kInitialValue = 1,
        kMostRecentValue = 2,
    };

    // The run-time state of a property comparison: the property's value
    // when the instance was made and in the last frame, read when a
    // comparison value asks for them. The vtable has nine slots (0x1957AE0
    // for float). The object is 64 bytes for float, from the small-block
    // pool.
    class Instance : public StateConditionComparison<T>::Instance {
    public:
        POOL_OVERLOAD(Instance)

        // Inlined into _CreateInstance (0x861530 for float).
        Instance(const StateConditionPropertyComparison& condition, StateGraphInstance& graph);
        // Slots 0-1: 0x861890 and 0x8618A0 for float.
        ~Instance() override {}

        // Slot 3: reads the most recent value when a comparison value
        // uses it.
        void _ResetPerFrameState() override;  // 0x8618D0
        // Slot 7: the initial or most recent value for a comparison value
        // in those modes.
        bool _OverrideComparisonValue(unsigned long index, T& value) const override;  // 0x861900
        // Slot 8: the property's value in the graph's entity, or false when
        // the property cannot be found or read; the report is compiled out.
        bool _GetValueToCompare(T& value) const override;  // 0x861930

        // Field names are not in the reference map.
        const StateConditionPropertyComparison* mPropertyComparison;
        T mInitialValue;
        T mLastValue;
    };

    StateConditionPropertyComparison() : mProp(), mComparisonModes{kSpecificValue, kSpecificValue} {}
    // Slots 0-1: 0x860A80 (Component's destructor) and 0x8610E0 for float.
    ~StateConditionPropertyComparison() override {}

    Symbol GetId() const override;          // slot 4: 0x861100
    Symbol GetClassName() const override;   // slot 5: 0x861110
    int CurrentRev() const override;        // slot 7: 0x861120
    bool IsA(Symbol type) const override;   // slot 8: 0x861140
    Component* AsComponent() override;      // slot 9: 0x861170
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x861180
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x861350
    ComMetaData& _GetMetaData() override;       // slot 23: 0x861360
    // Slot 42. The map's _CreateInstance(StateNodeInstance const&,
    // StateGraphInstance&) const.
    StateConditionInstance* _CreateInstance(StateGraphInstance& graph) const override;  // 0x861370
    // Slot 52: kPropertyComparison.
    int _GetType() const override;  // 0x8613B0
    // Slot 54: "property".
    const char* _GetValueToCompareGeneralDescription() const override;  // 0x8613C0
    // Slot 55: the property's path.
    const char* _GetValueToCompareDescription() const override;  // 0x8613D0
    // Slot 56: whether the property reference is complete.
    bool _HasValueToCompare() const override;  // 0x8614F0
    // Slot 58: "initial value" or "most recent value" for a comparison value
    // in those modes.
    const char* _GetComparisonValueDescriptionOverride(unsigned long index, Symbol name) const override;  // 0x861500

    // "StateConditionPropertyComparison_<type>", in the IdStr buffer
    // (0x1ABF310 for float).
    static const char* IdStr();
    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in the registering object (0x85D660 for float).
    static void Init();
    // The class factory (0x860950 for float).
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x85FA90

    // Guarded template statics. For float: sId 0x1AC4B08, sClassName
    // 0x1ABF050 (which stategraph/StateCondition.o initializes at 0x809950
    // for StateCondition::Create), sPropRegistry 0x1AC4A50 and sMetaData
    // 0x1AC4898.
    static Symbol sId;
    // Also "StateConditionPropertyComparison_<type>". Name not in the
    // reference map.
    static Symbol sClassName;
    static PropRegistry sPropRegistry;
    static ComMetaData sMetaData;

    // "prop": the property to compare. Field names are not in the reference
    // map.
    PropRef mProp;
    // "comparison_mode_a" and "comparison_mode_b" (ComparisonMode).
    int mComparisonModes[2];
};

static_assert(offsetof(StateConditionPropertyComparison<float>, mProp) == 512);
static_assert(offsetof(StateConditionPropertyComparison<float>, mComparisonModes) == 728);
static_assert(sizeof(StateConditionPropertyComparison<float>) == 736);
static_assert(offsetof(StateConditionPropertyComparison<float>::Instance, mPropertyComparison) == 48);
static_assert(offsetof(StateConditionPropertyComparison<float>::Instance, mInitialValue) == 56);
static_assert(offsetof(StateConditionPropertyComparison<float>::Instance, mLastValue) == 60);
static_assert(sizeof(StateConditionPropertyComparison<float>::Instance) == 64);

// The object's statics.
template <class T>
Symbol StateConditionPropertyComparison<T>::sId(StateConditionPropertyComparison<T>::IdStr());
template <class T>
Symbol StateConditionPropertyComparison<T>::sClassName(StateConditionPropertyComparison<T>::IdStr());
template <class T>
PropRegistry StateConditionPropertyComparison<T>::sPropRegistry;
template <class T>
ComMetaData StateConditionPropertyComparison<T>::sMetaData;

// The map's IdStr()::sBuf.
template <class T>
const char* StateConditionPropertyComparison<T>::IdStr() {
    static char sBuf[64];
    sprintf(sBuf, "StateConditionPropertyComparison_%s", TypeName(DrivenPropType<T>::kType, true));
    return sBuf;
}

// Reconstructed from eboot.elf at 0x85FA90. The comparison values' own
// values take the compared property's metadata, and show only in the
// "Specific value" mode. Properties saved before revision 1 are read by
// _LoadOldValues.
template <class T>
void StateConditionPropertyComparison<T>::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateConditionPropertyComparison prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&own != &registry) {
        registry.AddInheritedRegistry(sClassName, own);
    }
    PropRefMetadata& prop = TypeSpecificMetadata(
        registry.RegisterProp(
            "prop", offsetof(StateConditionPropertyComparison, mProp), kPropertyPropRef, sizeof(PropRef)),
        prototype.mProp);
    prop.mRefDefault = prototype.mProp;
    static_cast<void>(Symbol("prop"));
    prop.mHelp = String("The property this condition will compare.");
    prop.mEntity.mMode = TagBase<Entity*>::kModeFunction;
    prop.mEntity.mFunction = StateBase::_GetContextEntityTagFunc();
    prop.mExcludedTypes = {kPropertyAction};
    // A property of another type redraws the condition.
    prop.mRefOnChanged = [](const PropAccessorArgs& args) {
        const StateBase& condition = *static_cast<StateBase*>(args.mComponent);
        const PropInfo* const info =
            static_cast<const PropRef*>(args.mValue)->Find(condition._GetContextEntityPrototype(), nullptr);
        if (info == nullptr || info->mType == DrivenPropType<T>::kType) {
            condition._SendDisplayTextUpdated();
        }
    };

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
        "comparison_mode_a", offsetof(StateConditionPropertyComparison, mComparisonModes), kPropertyInt, 4)
                                                                   .mMetadata.Get());
    modeA.mDefault = prototype.mComparisonModes[0];
    static_cast<void>(Symbol("prop"));
    modeA.mHelp = String("Comparison mode for the value to be compared against.");
    modeA.mDisplayName = std::function<String(const TagArgs&)>([](const TagArgs& args) {
        const Base& condition = static_cast<const Base&>(args.mComponent);
        return String(condition.mOperator == StateCondition::kBetween ? "compare against (a)" : "compare against");
    });
    modeA.mAllowedValues = modes;

    IntMetadata<int>& modeB = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "comparison_mode_b",
        offsetof(StateConditionPropertyComparison, mComparisonModes) + sizeof(int),
        kPropertyInt,
        4).mMetadata.Get());
    modeB.mDefault = prototype.mComparisonModes[1];
    static_cast<void>(Symbol("prop"));
    modeB.mHelp = String("Comparison mode for the secondary value to be compared against.");
    modeB.mDisplayName = String("compare against (b)");
    modeB.mAllowedValues = modes;
    modeB.mIsHidden = std::function<bool(const TagArgs&)>([](const TagArgs& args) {
        return static_cast<const Base&>(args.mComponent).mOperator != StateCondition::kBetween;
    });

    std::function<void(const PropRegistry::RevisionArgs&)> onRevision = [](const PropRegistry::RevisionArgs& args) {
        if (args.mSavedRev <= 0) {
            static_cast<StateConditionPropertyComparison*>(args.mComponent)->_LoadOldValues(*args.mSavedProps);
        }
    };
    if (&own == &registry) {
        registry.SetCurrentRev(1);
        registry.mRevisionFunc = onRevision;
    }

    Base::_InitAsSuperclass(registry, metadata);
    registry.MoveProp(Symbol("comparison_mode_a"), Symbol("value_a"));
    registry.MoveProp(Symbol("comparison_mode_b"), Symbol("value_b"));

    // The own values take the metadata of the compared property.
    const TagRemapFunc remap = [](TagRemapArgs& args) {
        const StateConditionPropertyComparison& condition =
            static_cast<const StateConditionPropertyComparison&>(args.mComponent);
        args.mEntity = condition._GetContextEntityPrototype();
        args.mTargetCom = condition.mProp.Com(args.mEntity);
        args.mTargetPath = condition.mProp.mPath;
        args.mTargetInfo = condition.mProp.Find(args.mEntity, nullptr);
    };
    PropPath::Node member;
    PropPath::Node field;
    const T type = T();
    RemapPropMetadata(
        TypeSpecificMetadata(
            const_cast<PropInfo&>(
                *registry.FindProp(PropPath(member = Symbol("value_a"), field = Symbol("value")), nullptr)),
            type),
        remap);
    RemapPropMetadata(
        TypeSpecificMetadata(
            const_cast<PropInfo&>(
                *registry.FindProp(PropPath(member = Symbol("value_b"), field = Symbol("value")), nullptr)),
            type),
        remap);

    // The own values show in the "Specific value" mode only.
    registry.FindProp(PropPath(Symbol("value_a")), nullptr)
        ->mMetadata->mIsHidden._Set(std::function<bool(const TagArgs&)>([](const TagArgs& args) {
            return static_cast<const StateConditionPropertyComparison&>(args.mComponent).mComparisonModes[0] !=
                kSpecificValue;
        }));
    registry.FindProp(PropPath(Symbol("value_b")), nullptr)
        ->mMetadata->mIsHidden._Set(std::function<bool(const TagArgs&)>([](const TagArgs& args) {
            const StateConditionPropertyComparison& condition =
                static_cast<const StateConditionPropertyComparison&>(args.mComponent);
            if (condition.mOperator != StateCondition::kBetween) {
                return true;
            }
            return condition.mComparisonModes[1] != kSpecificValue;
        }));

    metadata.mDescription = "Compares a property value against a specific value or another property.";
    metadata.mAuthor = "Matt Sharpe";
}

// Reconstructed from eboot.elf at 0x861100.
template <class T>
Symbol StateConditionPropertyComparison<T>::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x861110.
template <class T>
Symbol StateConditionPropertyComparison<T>::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x861120.
template <class T>
int StateConditionPropertyComparison<T>::CurrentRev() const {
    return const_cast<StateConditionPropertyComparison*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x861140.
template <class T>
bool StateConditionPropertyComparison<T>::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x861170.
template <class T>
Component* StateConditionPropertyComparison<T>::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x861180. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
template <class T>
char* StateConditionPropertyComparison<T>::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy =
        reinterpret_cast<StateConditionPropertyComparison*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateConditionPropertyComparison(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return this->_ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x861350.
template <class T>
PropRegistry& StateConditionPropertyComparison<T>::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x861360.
template <class T>
ComMetaData& StateConditionPropertyComparison<T>::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x861370.
template <class T>
StateConditionInstance* StateConditionPropertyComparison<T>::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x8613B0.
template <class T>
int StateConditionPropertyComparison<T>::_GetType() const {
    return StateCondition::kPropertyComparison;
}

// Reconstructed from eboot.elf at 0x8613C0.
template <class T>
const char* StateConditionPropertyComparison<T>::_GetValueToCompareGeneralDescription() const {
    return "property";
}

// Reconstructed from eboot.elf at 0x8613D0.
template <class T>
const char* StateConditionPropertyComparison<T>::_GetValueToCompareDescription() const {
    char path[512] = {};
    mProp.mPath.ToString(path, sizeof(path));
    return (FormatString("%s") << path).Str();
}

// Reconstructed from eboot.elf at 0x8614F0.
template <class T>
bool StateConditionPropertyComparison<T>::_HasValueToCompare() const {
    return mProp.IsComplete();
}

// Reconstructed from eboot.elf at 0x861500.
template <class T>
const char* StateConditionPropertyComparison<T>::_GetComparisonValueDescriptionOverride(
    unsigned long index,
    Symbol name) const {
    static_cast<void>(name);
    switch (mComparisonModes[index]) {
    case kInitialValue:
        return "initial value";
    case kMostRecentValue:
        return "most recent value";
    default:
        return nullptr;
    }
}

// Reconstructed from eboot.elf at 0x860950.
template <class T>
Component* StateConditionPropertyComparison<T>::_Create() {
    return new StateConditionPropertyComparison();
}

// Reconstructed from eboot.elf at 0x85D660. Emitted in the registering
// object, which calls it.
template <class T>
void StateConditionPropertyComparison<T>::Init() {
    InitComponentClass<StateConditionPropertyComparison>();
}

// Reconstructed from eboot.elf at 0x861530. The property is read when a
// comparison value compares against it.
template <class T>
StateConditionPropertyComparison<T>::Instance::Instance(
    const StateConditionPropertyComparison& condition,
    StateGraphInstance& graph)
    : StateConditionComparison<T>::Instance(condition, graph), mPropertyComparison(&condition) {
    if (condition.mComparisonModes[0] != kSpecificValue || condition.mComparisonModes[1] != kSpecificValue) {
        Instance::_GetValueToCompare(mInitialValue);
        mLastValue = mInitialValue;
    }
}

// Reconstructed from eboot.elf at 0x8618D0.
template <class T>
void StateConditionPropertyComparison<T>::Instance::_ResetPerFrameState() {
    const StateConditionPropertyComparison& condition = *mPropertyComparison;
    if (condition.mComparisonModes[0] == kMostRecentValue || condition.mComparisonModes[1] == kMostRecentValue) {
        _GetValueToCompare(mLastValue);
    }
}

// Reconstructed from eboot.elf at 0x861900.
template <class T>
bool StateConditionPropertyComparison<T>::Instance::_OverrideComparisonValue(unsigned long index, T& value) const {
    switch (mPropertyComparison->mComparisonModes[index]) {
    case kMostRecentValue:
        value = mLastValue;
        return true;
    case kInitialValue:
        value = mInitialValue;
        return true;
    default:
        return false;
    }
}

// Reconstructed from eboot.elf at 0x861930.
template <class T>
bool StateConditionPropertyComparison<T>::Instance::_GetValueToCompare(T& value) const {
    const PropRef& prop = mPropertyComparison->mProp;
    GameObject* const object = prop.Obj(this->_GetContextEntity());
    if (object == nullptr) {
        return false;
    }
    Component* const com = object->FindCom(prop.mCom);
    if (com == nullptr) {
        return false;
    }
    void* storage = com;
    const PropInfo* const info = com->_GetPropRegistry().FindProp(prop.mPath, &storage);
    if (info != nullptr && _GetProp(*com, prop.mPath, *info, storage, &value, DrivenPropType<T>::kType)) {
        return true;
    }
    // The report is compiled out; its arguments remain.
    static_cast<void>(com->MakeErrorName());
    char path[512] = {};
    prop.mPath.ToString(path, sizeof(path));
    return false;
}

// The registrations of the StateConditionPropertyComparison<T> condition
// types: each calls its types' Init.
void StateConditionPropertyComparisonInit_A();  // 0x85D630
void StateConditionPropertyComparisonInit_B();  // 0x870010
void StateConditionPropertyComparisonInit_C();  // 0x881B80
