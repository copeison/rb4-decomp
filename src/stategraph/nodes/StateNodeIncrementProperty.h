#pragma once

#include <cstddef>
#include <cstdio>
#include <limits>
#include <new>

#include "entity/core/ComMetaData.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropUtl.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/nodes/StateNodeSetProperty.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

// The class id of the node that changes properties of the type,
// "StateNodeIncrementProperty_<kType>" with the type's constant name
// (TypeName), in one buffer the objects share (0x1AB6C30). Name not in the
// reference map.
inline const char* MakeIncrementPropertyClassName(PropertyType type) {
    static char sName[64];
    sprintf(sName, "StateNodeIncrementProperty_%s", TypeName(type, true));
    return sName;
}

// The bounds of the amount: the type's own for integers, +-1e30 for float.
// Inlined into _Init. Names not in the reference map.
template <class T>
inline T IncrementAmountMin() {
    return std::numeric_limits<T>::min();
}
template <class T>
inline T IncrementAmountMax() {
    return std::numeric_limits<T>::max();
}
template <>
inline float IncrementAmountMin<float>() {
    return -1e30F;
}
template <>
inline float IncrementAmountMax<float>() {
    return 1e30F;
}

// A node that changes a number property by an amount: adds it, subtracts
// it, or multiplies or divides by it ("operator"); the value of the set
// property node is the amount, which starts at one. Newer than the
// reference map, which has neither the class nor its objects; the name
// follows the class ids ("StateNodeIncrementProperty_kFloat") and the
// objects' names are inferred from the property nodes'. The binary
// registers the numbers in two groups, StateNodeIncrementPropertyInit_A
// (float to unsigned char) and _B (unsigned short to unsigned long); a
// third group is empty. The float copy's vtable at 0x193CD08 has 53 slots.
// The object is 672 bytes: the operator lives in the property node's tail
// padding.
template <class T>
class StateNodeIncrementProperty : public StateNodeSetProperty<T> {
public:
    // "operator". The labels are the allowed values'; the enumerator names
    // are not in the reference map.
    enum Operator : int {
        kOperatorAdd = 0,
        kOperatorSubtract = 1,
        kOperatorMultiply = 2,
        kOperatorDivide = 3,
    };

    // The instance that applies the target's value changed by the amount.
    // Name not in the reference map. The float copy's vtable at 0x193CEC0
    // has 20 slots. The object is 88 bytes for float, from the small-block
    // pool.
    class Instance : public StateNodeSetProperty<T>::Instance {
    public:
        POOL_OVERLOAD(Instance)

        // Inlined into _CreateInstance (0x72B610).
        Instance(const StateNodeIncrementProperty& node, StateGraphInstance& graph)
            : StateNodeSetProperty<T>::Instance(node, graph), mIncrementNode(&node) {}
        // Slots 0-1: 0x72B710, 0x72B740.
        ~Instance() override {}

        // Slot 16: the target's current value changed by the amount; the
        // current value when it cannot be read, or for a division by zero.
        T _GetValueToApply() const override;  // 0x72B790

        // Name not in the reference map.
        const StateNodeIncrementProperty* mIncrementNode;
    };

    // Inlined into the prototypes and the factory. The operator is left
    // unset; _PostCreate sets the amount.
    StateNodeIncrementProperty() {}
    // Copies the node for an imprint. Inlined into _Imprint (0x72AF70).
    StateNodeIncrementProperty(const StateNodeIncrementProperty& other) = default;
    // Slots 0-1: 0x722D40, 0x72AE20.
    ~StateNodeIncrementProperty() override {}

    Symbol GetId() const override;          // slot 4: 0x72AEF0
    Symbol GetClassName() const override;   // slot 5: 0x72AF00
    int CurrentRev() const override;        // slot 7: 0x72AF10
    bool IsA(Symbol type) const override;   // slot 8: 0x72AF30
    Component* AsComponent() override;      // slot 9: 0x72AF60
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x72AF70
    // Slot 19: a new node increments by one.
    void _PostCreate() override;  // 0x72B130
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x72B450
    ComMetaData& _GetMetaData() override;       // slot 23: 0x72B460
    // Slot 42: "<target> +=\n<amount>" once the target is set.
    const char* GetEditorText() const override;  // 0x72B470
    StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 45: 0x72B610
    // Slot 52: as the property node's, with
    // "StateNodeIncrementProperty_<type>".
    Component* _ChangeValueType(Symbol className, GameObject* object, PropertyType type) override;  // 0x72B680

    // Registers the class (InitComponentClass). Emitted in the group
    // objects, at 0x71E6A0 for float.
    static void Init();
    // The class factory. Emitted in the group objects, at 0x722CD0 for
    // float.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x721ED0

    // Guarded template statics; for float sId at 0x1AB4558 and sClassName
    // at 0x1AB4058, both "StateNodeIncrementProperty_kFloat". Names not in
    // the reference map.
    static Symbol sId;
    static Symbol sClassName;
    static PropRegistry sPropRegistry;  // 0x1AB43E0 for float
    static ComMetaData sMetaData;       // 0x1AB4068 for float

    // "operator" (Operator). Name not in the reference map.
    int mOperator;
};

template <class T>
Symbol StateNodeIncrementProperty<T>::sId(MakeIncrementPropertyClassName(DrivenPropType<T>::kType));
template <class T>
Symbol StateNodeIncrementProperty<T>::sClassName = Symbol(MakeIncrementPropertyClassName(DrivenPropType<T>::kType));
template <class T>
PropRegistry StateNodeIncrementProperty<T>::sPropRegistry;
template <class T>
ComMetaData StateNodeIncrementProperty<T>::sMetaData;

static_assert(offsetof(StateNodeIncrementProperty<float>, mOperator) == 668);
static_assert(sizeof(StateNodeIncrementProperty<float>) == 672);
static_assert(sizeof(StateNodeIncrementProperty<unsigned long>) == 672);
static_assert(offsetof(StateNodeIncrementProperty<float>::Instance, mIncrementNode) == 80);
static_assert(sizeof(StateNodeIncrementProperty<float>::Instance) == 88);

// Reconstructed from eboot.elf at 0x72AEF0.
template <class T>
Symbol StateNodeIncrementProperty<T>::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x72AF00.
template <class T>
Symbol StateNodeIncrementProperty<T>::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x72AF10.
template <class T>
int StateNodeIncrementProperty<T>::CurrentRev() const {
    return const_cast<StateNodeIncrementProperty*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x72AF30.
template <class T>
bool StateNodeIncrementProperty<T>::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x72AF60.
template <class T>
Component* StateNodeIncrementProperty<T>::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x72AF70. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
template <class T>
char* StateNodeIncrementProperty<T>::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodeIncrementProperty*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeIncrementProperty(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return this->_ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x72B130. A failure is reported; the
// report is compiled out and its arguments remain.
template <class T>
void StateNodeIncrementProperty<T>::_PostCreate() {
    StateNode::_PostCreate();
    static Symbol value;
    if (value == Symbol()) {
        value = Symbol("value");
    }
    PropPath::Node member;
    PropPath::Node amount;
    const PropPath path((member = value), (amount = value));
    const T one = 1;
    void* storage = this;
    const PropInfo* const info = _GetPropRegistry().FindProp(path, &storage);
    if (info == nullptr || !_SetProp(*this, path, *info, storage, &one, DrivenPropType<T>::kType, false)) {
        static_cast<void>(this->MakeErrorName());
        char text[512] = {};
        path.ToString(text, sizeof(text));
    }
}

// Reconstructed from eboot.elf at 0x72B450.
template <class T>
PropRegistry& StateNodeIncrementProperty<T>::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x72B460.
template <class T>
ComMetaData& StateNodeIncrementProperty<T>::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x72B470.
template <class T>
const char* StateNodeIncrementProperty<T>::GetEditorText() const {
    if (!this->mTargetProp.IsComplete()) {
        return gNullStr;
    }
    char path[512] = {};
    this->mTargetProp.mPath.ToString(path, sizeof(path));
    const char* target;
    {
        FormatString text("%s");
        text << path;
        target = text.Str();
    }
    const char* const amount = this->_GetValueDescription();
    FormatString text("%s +=\n%s");
    text << target << amount;
    return text.Str();
}

// Reconstructed from eboot.elf at 0x72B610.
template <class T>
StateNodeInstance* StateNodeIncrementProperty<T>::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x72B680.
template <class T>
Component* StateNodeIncrementProperty<T>::_ChangeValueType(Symbol className, GameObject* object, PropertyType type) {
    object->DestroyComponent(className, false);
    return object->CreateComponent(Symbol(MakeIncrementPropertyClassName(type)), false);
}

// Reconstructed from eboot.elf at 0x721ED0. The amount is the property
// node's "value"; the conversion is hidden.
template <class T>
void StateNodeIncrementProperty<T>::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeIncrementProperty prototype;
    if (&registry != &sPropRegistry) {
        registry.AddInheritedRegistry(sClassName, sPropRegistry);
    }
    StateNodeSetProperty<T>::_InitAsSuperclass(registry, metadata);

    static Symbol value;
    if (value == Symbol()) {
        value = Symbol("value");
    }
    PropPath::Node node;
    PropPath valuePath;
    valuePath.PushNode(node = value);
    registry.FindProp(valuePath, nullptr)->mMetadata->mDisplayName = String("increment amount");

    PropPath::Node member;
    PropPath::Node amountNode;
    const PropPath amountPath((member = value), (amountNode = value));
    auto& amount = TypeSpecificMetadata(const_cast<PropInfo&>(*registry.FindProp(amountPath, nullptr)), T());
    amount.mMin = IncrementAmountMin<T>();
    amount.mMax = IncrementAmountMax<T>();

    PropPath targetPath;
    targetPath.PushNode(node = Symbol("target_prop"));
    registry.FindProp(targetPath, nullptr);
    PropPath targetPath2;
    targetPath2.PushNode(node = Symbol("target_prop"));
    PropRefMetadata& target =
        TypeSpecificMetadata(const_cast<PropInfo&>(*registry.FindProp(targetPath2, nullptr)), PropRef());
    static_cast<void>(Symbol("prop"));
    target.mAllowedTypes = GetCompatibleTypes<T>();

    PropPath conversionPath;
    conversionPath.PushNode(node = Symbol("conversion"));
    registry.FindProp(conversionPath, nullptr);
    PropPath conversionPath2;
    conversionPath2.PushNode(node = Symbol("conversion"));
    IntMetadata<int>& conversion =
        TypeSpecificMetadata(const_cast<PropInfo&>(*registry.FindProp(conversionPath2, nullptr)), 0);
    static_cast<void>(Symbol("prop"));
    conversion.mIsHidden = true;

    IntMetadata<int>& op = static_cast<IntMetadata<int>&>(*registry.RegisterProp(
        "operator", offsetof(StateNodeIncrementProperty, mOperator), kPropertyInt, 4).mMetadata.Get());
    op.mDefault = prototype.mOperator;
    static_cast<void>(Symbol("prop"));
    op.mHelp = String("The operation to perform.");
    op.mAllowedValues = eastl::vector<AllowedValue<int>>{
        AllowedValue<int>{
            kOperatorAdd, String("Add"), String("Increase the specified property by the specified value.")},
        AllowedValue<int>{
            kOperatorSubtract, String("Subtract"), String("Decrease specified property by the specified value.")},
        AllowedValue<int>{
            kOperatorMultiply, String("Multiply"), String("Multiply the specified property by the specified value.")},
        AllowedValue<int>{
            kOperatorDivide, String("Divide"), String("Divide the specified property by the specified value.")}};
    registry.MoveProp(Symbol("operator"), Symbol("persistent"));

    // Revision 0 kept the amount as the plain "increment_amount", revision
    // 1 as a driven "increment_amount".
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision = [](const PropRegistry::RevisionArgs& args) {
        static Symbol increment_amount;
        if (increment_amount == Symbol()) {
            increment_amount = Symbol("increment_amount");
        }
        if (args.mSavedRev <= 0) {
            Component& com = *args.mComponent;
            Component& saved = *args.mSavedProps;
            PropPath::Node member;
            PropPath::Node amount;
            const PropPath newPath((member = value), (amount = value));
            const PropPath oldPath(increment_amount);
            T old = T();
            void* storage = &saved;
            const PropInfo* info = saved._GetPropRegistry().FindProp(oldPath, &storage);
            if (info == nullptr || !_GetProp(saved, oldPath, *info, storage, &old, DrivenPropType<T>::kType)) {
                // The report is compiled out; its arguments remain.
                static_cast<void>(saved.MakeErrorName());
                char text[512] = {};
                oldPath.ToString(text, sizeof(text));
            }
            storage = &com;
            info = com._GetPropRegistry().FindProp(newPath, &storage);
            if (info == nullptr || !_SetProp(com, newPath, *info, storage, &old, DrivenPropType<T>::kType, false)) {
                static_cast<void>(com.MakeErrorName());
                char text[512] = {};
                newPath.ToString(text, sizeof(text));
            }
        } else if (args.mSavedRev == 1) {
            static_cast<StateNodeIncrementProperty*>(args.mComponent)
                ->mValue.CopyFromProperty(*args.mSavedProps, PropPath(increment_amount));
        }
    };
    if (&registry == &sPropRegistry) {
        sPropRegistry.SetCurrentRev(4);
        sPropRegistry.mRevisionFunc = onRevision;
    }

    metadata.mDescription = "Increments the specified property by the specified amount.";
    metadata.mAuthor = "Matt Sharpe";
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("i");
}

// Reconstructed from eboot.elf at 0x722CD0. The binary emits the factory
// in the group object with the class's Init.
template <class T>
Component* StateNodeIncrementProperty<T>::_Create() {
    return new StateNodeIncrementProperty();
}

// Reconstructed from eboot.elf at 0x71E6A0.
template <class T>
void StateNodeIncrementProperty<T>::Init() {
    InitComponentClass<StateNodeIncrementProperty>();
}

// Reconstructed from eboot.elf at 0x72B790. The amount is the property
// node's value, read as StateNodeSetValue<T>::Instance does.
template <class T>
T StateNodeIncrementProperty<T>::Instance::_GetValueToApply() const {
    T current = T();
    if (!this->_GetCurrentValue(current)) {
        return current;
    }
    const T amount = StateNodeSetValue<T>::Instance::_GetValueToApply();
    switch (mIncrementNode->mOperator) {
    case kOperatorAdd:
        return static_cast<T>(amount + current);
    case kOperatorSubtract:
        return static_cast<T>(current - amount);
    case kOperatorMultiply:
        return static_cast<T>(amount * current);
    case kOperatorDivide:
        if (amount == 0) {
            return current;
        }
        return static_cast<T>(current / amount);
    default:
        return current;
    }
}

// The registrations of the StateNodeIncrementProperty<T> types, each
// calling its types' Init: _A float to unsigned char, _B unsigned short to
// unsigned long; _C is empty. Names not in the reference map.
void StateNodeIncrementPropertyInit_A();  // 0x71E670
void StateNodeIncrementPropertyInit_B();  // 0x75F980
void StateNodeIncrementPropertyInit_C();  // 0x780B10
