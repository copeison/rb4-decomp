#pragma once

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <new>
#include <type_traits>

#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropArray.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropRegistry.h"
#include "entity/props/PropUtl.h"
#include "entity/waveform/Waveform.h"
#include "math/color/Color.h"
#include "math/random/Rand.h"
#include "os/memory/MemMgr.h"
#include "stategraph/base/StateBase.h"
#include "stategraph/conditions/StateCondition.h"
#include "stategraph/props/DrivenProp.h"
#include "stategraph/runtime/StateGraphInstance.h"
#include "utl/containers/Vector.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class StateGraphInstance;

// Whether two values of a comparison are equal. Floats are equal within
// 1e-4; ids, colours and waveforms compare their parts. Inlined into the
// comparisons. Name not in the reference map.
template <class T>
inline bool ComparisonEquals(const T& value, const T& other) {
    return value == other;
}
inline bool ComparisonEquals(const float& value, const float& other) {
    return std::fabs(value - other) <= 0.0001F;
}
inline bool ComparisonEquals(const GameObjectId& value, const GameObjectId& other) {
    return value.mId == other.mId;
}
inline bool ComparisonEquals(const Hmx::Color& value, const Hmx::Color& other) {
    return value.red == other.red && value.green == other.green && value.blue == other.blue &&
        value.alpha == other.alpha;
}
// The same resources, or equal ones, and the same modulating resource and
// data. The first modulation word is compared as a float.
template <class T>
inline bool ComparisonEquals(const Waveform<T>& value, const Waveform<T>& other) {
    const WaveformResource<T>* const resource = value.mResource.Get();
    const WaveformResource<T>* const otherResource = other.mResource.Get();
    if (resource != otherResource && !(*resource == *otherResource)) {
        return false;
    }
    const WaveformResource<T>* const modulation = value.mOverride.Get();
    const WaveformResource<T>* const otherModulation = other.mOverride.Get();
    if (modulation == nullptr || otherModulation == nullptr) {
        return (modulation == nullptr) == (otherModulation == nullptr);
    }
    if (reinterpret_cast<const float&>(value.mOverrideData[0]) !=
            reinterpret_cast<const float&>(other.mOverrideData[0]) ||
        value.mOverrideData[1] != other.mOverrideData[1]) {
        return false;
    }
    return modulation == otherModulation || *modulation == *otherModulation;
}

// Compares a number with comparison value (a), or with the span between (a)
// and (b). With `random`, (a) moves that fraction of the way to (b) first.
// The map's CompareNumber<T>(T, StateCondition::ComparisonOperator, T, T);
// this build passes the random fraction. Inlined into the instances' _IsMet.
template <class T>
bool CompareNumber(T value, StateCondition::ComparisonOperator op, T a, T b, const float* random) {
    if (op == StateCondition::kBetween) {
        const T low = b < a ? b : a;
        const T high = a < b ? b : a;
        return low <= value && value < high;
    }
    if (random != nullptr) {
        const float from = static_cast<float>(a);
        a = static_cast<T>((static_cast<float>(b) - from) * *random + from);
    }
    switch (op) {
    case StateCondition::kEqual:
        return ComparisonEquals(value, a);
    case StateCondition::kNotEqual:
        return !ComparisonEquals(value, a);
    case StateCondition::kLess:
        return value < a;
    case StateCondition::kLessEqual:
        return value <= a;
    case StateCondition::kGreater:
        return value > a;
    case StateCondition::kGreaterEqual:
        return value >= a;
    default:
        return false;
    }
}

// Compares the value with the comparison values: numbers through
// CompareNumber, every other type by equality alone. The map's
// Compare<T>(T const&, StateCondition::ComparisonOperator, T const&,
// T const&); this build passes the random fraction, which only numbers use.
template <class T>
bool Compare(const T& value, StateCondition::ComparisonOperator op, const T& a, const T& b, const float* random) {
    if constexpr (std::is_arithmetic<T>::value && !std::is_same<T, bool>::value) {
        return CompareNumber<T>(value, op, a, b, random);
    } else {
        static_cast<void>(b);
        static_cast<void>(random);
        switch (op) {
        case StateCondition::kEqual:
            return ComparisonEquals(value, a);
        case StateCondition::kNotEqual:
            return !ComparisonEquals(value, a);
        default:
            return false;
        }
    }
}

// A condition that compares a value with comparison value (a), or with the
// span between (a) and (b), by its operator
// (stategraph/StateConditionComparisonInit_A.o and _B.o; class id
// "StateConditionComparison_<type>"). Each comparison value is a
// DrivenProp<T>; with "randomize" set, a number is compared with a value
// drawn at random between (a) and (b) for each instance. Subclasses give the
// value to compare: a property (StateConditionPropertyComparison<T>), the
// time elapsed and others. The vtable has 59 slots (0x1950C58 for float).
// The object is 512 bytes for float.
template <class T>
class StateConditionComparison : public StateCondition {
public:
    // The run-time state of a comparison (the map's
    // StateConditionComparison<T>::Instance): the random fraction for the
    // instance. Subclasses give the value to compare. The vtable has nine
    // slots (0x1955080 for float). The object is 48 bytes; subclasses come
    // from the small-block pool.
    class Instance : public StateConditionInstance {
    public:
        // The fraction is drawn when the condition randomizes. Inlined into
        // the subclasses' constructors, for example at 0x861530.
        Instance(const StateConditionComparison& condition, StateGraphInstance& graph)
            : StateConditionInstance(condition, graph),
              mComparison(&condition),
              mRandom(condition.IsRandomized() ? gRand.Float() : 0.0F) {}
        // Slots 0-1: 0x83C8B0 (StateConditionInstance's) and 0x83C8C0 for
        // float.
        ~Instance() override {}

        // Slot 2: false without a value to compare; otherwise the value
        // against the comparison values.
        bool _IsMet(bool useCached) override;  // 0x83C8E0
        // Slot 7 at 0x83C890 for float: puts a comparison value other than
        // the condition's own in `value`; none here.
        virtual bool _OverrideComparisonValue(unsigned long index, T& value) const {
            static_cast<void>(index);
            static_cast<void>(value);
            return false;
        }
        // Slot 8: the value to compare, or false when there is none.
        virtual bool _GetValueToCompare(T& value) const = 0;

        // Comparison value (a) or (b): the override, or the condition's own
        // value, driven in the graph's entity. Inlined, apart from the
        // waveform copies (0x855E30 for Waveform<float>).
        T _GetComparisonValue(unsigned long index) const;

        // Field names are not in the reference map.
        const StateConditionComparison* mComparison;
        // The fraction of the way from (a) to (b) of the value to compare
        // against when the condition randomizes.
        float mRandom;
    };

    StateConditionComparison() : mOperator(kEqual), mValues(), mRandomize(false) {}
    // Slots 0-1: 0x80D9D0 (Component's destructor) and 0x80E880 for float.
    ~StateConditionComparison() override {}

    Symbol GetId() const override;          // slot 4: 0x80E8A0
    Symbol GetClassName() const override;   // slot 5: 0x80E8B0
    int CurrentRev() const override;        // slot 7: 0x80E8C0
    bool IsA(Symbol type) const override;   // slot 8: 0x80E8E0
    Component* AsComponent() override;      // slot 9: 0x80E910
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x80E920
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x80EA60
    ComMetaData& _GetMetaData() override;       // slot 23: 0x80EA70
    // Slot 50: "<value> <operator> <(a)>", with the random span or the
    // upper bound.
    const char* _GetDescription() const override;  // 0x80EA80
    // Slot 54 at 0x80EC50 for float: what the condition compares, in the
    // operators' help.
    virtual const char* _GetValueToCompareGeneralDescription() const {
        return "value";
    }
    // Slot 55 at 0x80EC60 for float: the value compared, in the
    // description.
    virtual const char* _GetValueToCompareDescription() const {
        return gNullStr;
    }
    // Slot 56 at 0x80EC70 for float: whether there is a value to compare;
    // the comparison values are read-only without one.
    virtual bool _HasValueToCompare() const {
        return true;
    }
    // Slot 57 at 0x80EC80 for float: whether the editor offers the
    // operator.
    virtual bool _AllowOperator(ComparisonOperator op) const {
        static_cast<void>(op);
        return true;
    }
    // Slot 58 at 0x80EC90 for float: the description of a comparison value
    // in place of its own; none here.
    virtual const char* _GetComparisonValueDescriptionOverride(unsigned long index, Symbol name) const {
        static_cast<void>(index);
        static_cast<void>(name);
        return nullptr;
    }

    // The description of comparison value (a) or (b), the property `name`:
    // the override, the driving property, or the value.
    const char* _GetComparisonValueDescription(unsigned long index, Symbol name) const;  // 0x80ECA0
    // Reads the values saved before revision 1, under "comparison_value_a",
    // "comparison_prop_a" and the (b) names. Emitted outside the object
    // (0x83B6E0 for float).
    void _LoadOldValues(const Component& saved);
    // Whether the instances draw a value between (a) and (b): the flag is
    // set, the type is a number and the operator is not kBetween. Inlined.
    // Name not in the reference map.
    bool IsRandomized() const {
        return mRandomize && IsNumber(DrivenPropType<T>::kType) && mOperator != kBetween;
    }

    // "StateConditionComparison_<type>", in the IdStr buffer.
    static const char* IdStr();
    // Registers the class (InitComponentClass). Inline in the map; this
    // build emits it in StateConditionComparisonInit_A.o (0x809AE0 for
    // float).
    static void Init();
    // The class factory (0x80D8F0 for float).
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x80CEC0
    // Makes the comparison the superclass of a subclass's metadata, as
    // StateCondition::_InitAsSuperclass does, under the name
    // "StateConditionComparison". Emitted outside the object (0x83A300 for
    // float). Name not in the reference map.
    static void _InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata);

    // Guarded template statics (0x1AB2208, 0x1AB2218, 0x1AB23E0 and
    // 0x1AB2228 for float).
    static Symbol sId;
    // Also "StateConditionComparison_<type>". Name not in the reference map.
    static Symbol sClassName;
    static PropRegistry sPropRegistry;
    static ComMetaData sMetaData;

    // "operator" (ComparisonOperator). It sits in StateCondition's tail
    // padding. Field names are not in the reference map.
    int mOperator;
    // "value_a" and "value_b".
    DrivenProp<T> mValues[2];
    // "randomize".
    bool mRandomize;
};

static_assert(offsetof(StateConditionComparison<float>, mOperator) == 36);
static_assert(offsetof(StateConditionComparison<float>, mValues) == 40);
static_assert(offsetof(StateConditionComparison<float>, mRandomize) == 504);
static_assert(sizeof(StateConditionComparison<float>) == 512);
static_assert(offsetof(StateConditionComparison<Hmx::Color>, mRandomize) == 520);
static_assert(offsetof(StateConditionComparison<PropRef>, mRandomize) == 920);
static_assert(offsetof(StateConditionComparison<Waveform<float>>, mRandomize) == 568);
static_assert(offsetof(StateConditionComparison<float>::Instance, mComparison) == 32);
static_assert(offsetof(StateConditionComparison<float>::Instance, mRandom) == 40);
static_assert(sizeof(StateConditionComparison<float>::Instance) == 48);

// The object's statics. The class symbols are "StateConditionComparison_"
// and the type's name; IdStr formats them in its buffer.
template <class T>
Symbol StateConditionComparison<T>::sId(StateConditionComparison<T>::IdStr());
template <class T>
Symbol StateConditionComparison<T>::sClassName(StateConditionComparison<T>::IdStr());
template <class T>
PropRegistry StateConditionComparison<T>::sPropRegistry;
template <class T>
ComMetaData StateConditionComparison<T>::sMetaData;

// The map's IdStr()::sBuf (0x1AC0810 for signed char).
template <class T>
const char* StateConditionComparison<T>::IdStr() {
    static char sBuf[64];
    sprintf(sBuf, "StateConditionComparison_%s", TypeName(DrivenPropType<T>::kType, true));
    return sBuf;
}

// Reconstructed from eboot.elf at 0x80CEC0. The operators the editor offers
// follow _AllowOperator; the ordering ones and "Between" are for numbers.
template <class T>
void StateConditionComparison<T>::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateConditionComparison prototype;
    if (&prototype._GetPropRegistry() != &registry) {
        registry.AddInheritedRegistry(sClassName, prototype._GetPropRegistry());
    }
    IntMetadata<int>& op = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "operator", offsetof(StateConditionComparison, mOperator), kPropertyInt, 4).mMetadata.Get());
    op.mDefault = prototype.mOperator;
    static_cast<void>(Symbol("prop"));
    op.mHelp = String("The comparison operator to apply.");
    op.mAllowedValues = std::function<eastl::vector<AllowedValue<int>>(const TagArgs&)>([](const TagArgs& args) {
        const StateConditionComparison& condition = static_cast<const StateConditionComparison&>(args.mComponent);
        eastl::vector<AllowedValue<int>> values;
        const String help(
            (FormatString("Return true when the %s is %%s the comparison value/property.")
             << condition._GetValueToCompareGeneralDescription())
                .Str());
        if (condition._AllowOperator(kEqual)) {
            values.push_back(AllowedValue<int>{
                kEqual, String("=="), String((FormatString(help.c_str()) << "**equal to**").Str())});
        }
        if (condition._AllowOperator(kNotEqual)) {
            values.push_back(AllowedValue<int>{
                kNotEqual, String("!="), String((FormatString(help.c_str()) << "**not equal to**").Str())});
        }
        if (IsNumber(DrivenPropType<T>::kType)) {
            if (condition._AllowOperator(kLess)) {
                values.push_back(AllowedValue<int>{
                    kLess, String("<"), String((FormatString(help.c_str()) << "**less than**").Str())});
            }
            if (condition._AllowOperator(kLessEqual)) {
                values.push_back(AllowedValue<int>{
                    kLessEqual,
                    String("<="),
                    String((FormatString(help.c_str()) << "**less than or equal to**").Str())});
            }
            if (condition._AllowOperator(kGreater)) {
                values.push_back(AllowedValue<int>{
                    kGreater, String(">"), String((FormatString(help.c_str()) << "**greater than**").Str())});
            }
            if (condition._AllowOperator(kGreaterEqual)) {
                values.push_back(AllowedValue<int>{
                    kGreaterEqual,
                    String(">="),
                    String((FormatString(help.c_str()) << "**greater than or equal to**").Str())});
            }
            if (condition._AllowOperator(kBetween)) {
                values.push_back(AllowedValue<int>{
                    kBetween,
                    String("Between"),
                    String((FormatString("Return true when **value (a) <= %s < value (b)**")
                            << condition._GetValueToCompareGeneralDescription())
                               .Str())});
            }
        }
        return values;
    });
    op.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<StateBase*>(args.mComponent)->_SendDisplayTextUpdated();
    };

    GroupMetadata& valueA = prototype.mValues[0].Register(
        registry,
        Symbol("value_a"),
        offsetof(StateConditionComparison, mValues),
        StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    valueA.mHelp = String("This value will be compared against.");
    valueA.mDisplayName = std::function<String(const TagArgs&)>([](const TagArgs& args) {
        const StateConditionComparison& condition = static_cast<const StateConditionComparison&>(args.mComponent);
        if (condition.mOperator == kBetween) {
            return String("value (a)");
        }
        return String(condition.IsRandomized() ? "value (a)" : "value");
    });
    valueA.mReadOnly = std::function<bool(const TagArgs&)>([](const TagArgs& args) {
        return !static_cast<const StateConditionComparison&>(args.mComponent)._HasValueToCompare();
    });

    GroupMetadata& valueB = prototype.mValues[1].Register(
        registry,
        Symbol("value_b"),
        offsetof(StateConditionComparison, mValues) + sizeof(DrivenProp<T>),
        StateBase::_GetContextEntityTagFunc());
    static_cast<void>(Symbol("driven_prop"));
    valueB.mHelp = String("A secondary value to be compared against.");
    valueB.mDisplayName = String("value (b)");
    valueB.mReadOnly = std::function<bool(const TagArgs&)>([](const TagArgs& args) {
        return !static_cast<const StateConditionComparison&>(args.mComponent)._HasValueToCompare();
    });
    // (b) shows for "Between" and for a random span.
    valueB.mIsHidden = std::function<bool(const TagArgs&)>([](const TagArgs& args) {
        const StateConditionComparison& condition = static_cast<const StateConditionComparison&>(args.mComponent);
        if (condition.mOperator == kBetween) {
            return false;
        }
        return !condition.IsRandomized();
    });

    BoolMetadata& randomize = TypeSpecificMetadata(
        registry.RegisterProp("randomize", offsetof(StateConditionComparison, mRandomize), kPropertyBool, 1),
        prototype.mRandomize);
    randomize.mDefault = prototype.mRandomize;
    static_cast<void>(Symbol("prop"));
    randomize.mHelp = String(
        "If true, the user can specify a pair of values. The actual value to compare against will be chosen at "
        "random between value (a) and value (b).");
    randomize.mIsHidden = std::function<bool(const TagArgs&)>([](const TagArgs& args) {
        const StateConditionComparison& condition = static_cast<const StateConditionComparison&>(args.mComponent);
        if (!condition._HasValueToCompare() || !IsNumber(DrivenPropType<T>::kType)) {
            return true;
        }
        return condition.mOperator == kBetween;
    });
    randomize.mOnChanged = [](const PropAccessorArgs& args) {
        static_cast<StateBase*>(args.mComponent)->_SendDisplayTextUpdated();
    };

    StateBase::_FixDrivenProp<T>(registry, PropPath(Symbol("value_a")));
    StateBase::_FixDrivenProp<T>(registry, PropPath(Symbol("value_b")));
    StateCondition::_InitAsSuperclass(registry, metadata);
    metadata.mDescription = "Compares a value against a specific value or another property.";
    metadata.mAuthor = "Matt Sharpe";
}

// Reconstructed from eboot.elf at 0x83A300.
template <class T>
void StateConditionComparison<T>::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("StateConditionComparison", sMetaData);
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

// Reconstructed from eboot.elf at 0x83B6E0. A value saved with its property
// is driven when the property is complete.
template <class T>
void StateConditionComparison<T>::_LoadOldValues(const Component& saved) {
    mValues[0].mValue = GetPropValue<T>(saved, PropPath(Symbol("comparison_value_a")));
    mValues[0].mProp = GetPropValue<PropRef>(saved, PropPath(Symbol("comparison_prop_a")));
    mValues[0].mDriven = mValues[0].mProp.IsComplete();
    mValues[1].mValue = GetPropValue<T>(saved, PropPath(Symbol("comparison_value_b")));
    mValues[1].mProp = GetPropValue<PropRef>(saved, PropPath(Symbol("comparison_prop_b")));
    mValues[1].mDriven = mValues[1].mProp.IsComplete();
}

// Reconstructed from eboot.elf at 0x80E8A0.
template <class T>
Symbol StateConditionComparison<T>::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x80E8B0.
template <class T>
Symbol StateConditionComparison<T>::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x80E8C0.
template <class T>
int StateConditionComparison<T>::CurrentRev() const {
    return const_cast<StateConditionComparison*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x80E8E0.
template <class T>
bool StateConditionComparison<T>::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x80E910.
template <class T>
Component* StateConditionComparison<T>::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x80E920. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
template <class T>
char* StateConditionComparison<T>::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateConditionComparison*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateConditionComparison(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return _ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x80EA60.
template <class T>
PropRegistry& StateConditionComparison<T>::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x80EA70.
template <class T>
ComMetaData& StateConditionComparison<T>::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x80EA80.
template <class T>
const char* StateConditionComparison<T>::_GetDescription() const {
    const char* op;
    switch (mOperator) {
    case kEqual:
        op = "==";
        break;
    case kNotEqual:
        op = "!=";
        break;
    case kLess:
        op = "<";
        break;
    case kLessEqual:
        op = "<=";
        break;
    case kGreater:
        op = ">";
        break;
    case kGreaterEqual:
        op = ">=";
        break;
    case kBetween:
        op = "is between";
        break;
    default:
        op = "";
        break;
    }
    const bool randomized = IsRandomized();
    const char* prefix = randomized ? "random value between " : "";
    const char* suffix = "";
    if (randomized || mOperator == kBetween) {
        const char* const b = _GetComparisonValueDescription(1, Symbol("value_b"));
        suffix = (FormatString(" and %s") << b).Str();
    }
    const char* const subject = _GetValueToCompareDescription();
    const char* const a = _GetComparisonValueDescription(0, Symbol("value_a"));
    return (FormatString("%s %s %s%s%s") << subject << op << prefix << a << suffix).Str();
}

// Reconstructed from eboot.elf at 0x80ECA0. The own value is shown through
// a reference to the condition's own property.
template <class T>
const char* StateConditionComparison<T>::_GetComparisonValueDescription(unsigned long index, Symbol name) const {
    if (const char* const description = _GetComparisonValueDescriptionOverride(index, name)) {
        return description;
    }
    const DrivenProp<T>& value = mValues[index];
    if (value.mDriven && value.mProp.IsComplete()) {
        return value.mProp.Description(_GetContextEntityPrototype());
    }
    PropPath::Node member;
    PropPath::Node field;
    const PropRef self(mObject->mId, GetId(), PropPath(member = name, field = Symbol("value")));
    return self.ValueToString(mObject->mEntity);
}

// Reconstructed from eboot.elf at 0x80D8F0.
template <class T>
Component* StateConditionComparison<T>::_Create() {
    return new StateConditionComparison();
}

// Reconstructed from eboot.elf at 0x809AE0. Emitted in
// StateConditionComparisonInit_A.o, which calls it.
template <class T>
void StateConditionComparison<T>::Init() {
    InitComponentClass<StateConditionComparison>();
}

// Reconstructed from eboot.elf at 0x83C8E0. Number comparisons may move
// (a) toward (b) by the instance's random fraction.
template <class T>
bool StateConditionComparison<T>::Instance::_IsMet(bool useCached) {
    static_cast<void>(useCached);
    T value;
    if (!_GetValueToCompare(value)) {
        return false;
    }
    const ComparisonOperator op = static_cast<ComparisonOperator>(mComparison->mOperator);
    const T a = _GetComparisonValue(0);
    const T b = _GetComparisonValue(1);
    const float* const random = mComparison->IsRandomized() ? &mRandom : nullptr;
    return Compare<T>(value, op, a, b, random);
}

// Reconstructed from eboot.elf at 0x855E30 (the Waveform<float> copy).
template <class T>
T StateConditionComparison<T>::Instance::_GetComparisonValue(unsigned long index) const {
    T value;
    if (!_OverrideComparisonValue(index, value)) {
        value = mComparison->mValues[index].GetValue(mGraph->mEntity);
    }
    return value;
}

// The registrations of the StateConditionComparison<T> condition types: each
// calls its types' Init.
void StateConditionComparisonInit_A();  // 0x809AA0
void StateConditionComparisonInit_B();  // 0x81E630
