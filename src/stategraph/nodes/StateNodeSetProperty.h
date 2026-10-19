#pragma once

#include <cstddef>
#include <cstdio>
#include <new>

#include "entity/core/ComMetaData.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropUtl.h"
#include "os/memory/PoolAlloc.h"
#include "stategraph/drivers/StateGraphDriverCom.h"
#include "stategraph/nodes/StateNodeSetValue.h"
#include "utl/text/MakeString.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

// The class id of the node that sets properties of the type,
// "StateNodeSetProperty_<kType>" with the type's constant name (TypeName),
// in one buffer the property node objects share (0x1AB6B70). The guarded
// statics' initializers and _ChangeValueType inline it. Name not in the
// reference map.
inline const char* MakeSetPropertyClassName(PropertyType type) {
    static char sName[64];
    sprintf(sName, "StateNodeSetProperty_%s", TypeName(type, true));
    return sName;
}

// A node that sets a property of type T of an object of the graph's
// entity (the map's StateNodeSetProperty<T>): the target property and, for
// resource paths, whether to load the resource first. Its instance applies
// the value once on entering or every frame, blending from the value an
// earlier node set. The binary registers the types in three groups,
// StateNodeSetPropertyInit_A (float to unsigned char), _B (unsigned short
// to Symbol) and _C (ResourcePath to PropXfm); each copy's code is emitted
// with the first object that uses it, the float one in
// stategraph/StateNodeIncrementPropertyInit_A.o (vtable 0x193C310 with 53
// slots). The object is 672 bytes for the types whose value node is 448,
// 680 for colours, vectors and rotations, 704 for waveforms and transforms
// and 880 for property references.
template <class T>
class StateNodeSetProperty : public StateNodeSetValue<T> {
public:
    // The instance that applies the value to the target property (the
    // map's StateNodeSetProperty<T>::Instance). The float copy's vtable at
    // 0x193C4C8 has 20 slots. The object is 80 bytes for float, from the
    // small-block pool.
    class Instance : public StateNodeSetValue<T>::Instance {
    public:
        POOL_OVERLOAD(Instance)

        // Inlined into _CreateInstance (0x723F90).
        Instance(const StateNodeSetProperty& node, StateGraphInstance& graph)
            : StateNodeSetValue<T>::Instance(node, graph), mSetPropertyNode(&node) {}
        // Slots 0-1: 0x724280, 0x7242B0.
        ~Instance() override {}

        // Slot 15: the target property's value.
        bool _GetCurrentValue(T& value) const override;  // 0x7244F0
        // Slot 17: sets the target property. A resource path may be loaded
        // first.
        void _ApplyValue(const T& value) const override;  // 0x7245D0
        // Slot 18: the other instance's node targets the same property.
        bool _HasSameTarget(const StateNodeInstance& other) const override;  // 0x724890

        // Name not in the reference map.
        const StateNodeSetProperty* mSetPropertyNode;
    };

    // Inlined into the prototypes' and the factory's; out of line at
    // 0x7239A0 for float.
    StateNodeSetProperty();
    // Copies the node for an imprint. Inlined into _Imprint (0x723C30).
    StateNodeSetProperty(const StateNodeSetProperty& other) = default;
    // Slots 0-1: 0x7238E0, 0x723AE0.
    ~StateNodeSetProperty() override {}

    Symbol GetId() const override;          // slot 4: 0x723BB0
    Symbol GetClassName() const override;   // slot 5: 0x723BC0
    int CurrentRev() const override;        // slot 7: 0x723BD0
    bool IsA(Symbol type) const override;   // slot 8: 0x723BF0
    Component* AsComponent() override;      // slot 9: 0x723C20
    // Slot 10. The map's _Imprint(char*, Component*&, bool).
    char* _Imprint(char* buffer, Component** imprint) override;  // 0x723C30
    PropRegistry& _GetPropRegistry() override;  // slot 22: 0x723DD0
    ComMetaData& _GetMetaData() override;       // slot 23: 0x723DE0
    // Slot 42: "<target> =\n<value>" once the target is set.
    const char* GetEditorText() const override;  // 0x723DF0
    StateNodeInstance* _CreateInstance(StateGraphInstance& graph) const override;  // slot 45: 0x723F90
    // Slot 50: whether the target is set.
    bool _HasValueToSet() const override;  // 0x723FF0
    // Slot 51: the target property's range in the graph's prototype
    // entity.
    bool _GetTargetRange(float* min, float* max) const override;  // 0x724000
    // Slot 52: replaces the node on the object with the property node of
    // the type: destroys the component of the class and creates
    // "StateNodeSetProperty_<type>". No caller in this build's code. Name
    // not in the reference map.
    virtual Component* _ChangeValueType(Symbol className, GameObject* object, PropertyType type);  // 0x724040

    // Registers the class (InitComponentClass). Emitted in the group
    // objects, at 0x791430 for float.
    static void Init();
    // The class factory. Emitted in the group objects, at 0x793170 for
    // float.
    static Component* _Create();
    static void _Init(PropRegistry& registry, ComMetaData& metadata);  // 0x722EB0
    // Makes the node the superclass of a subclass's metadata, as
    // StateNode::_InitAsSuperclass does. Name not in the reference map.
    static void _InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata);  // 0x722E00

    // Guarded template statics; for float sId at 0x1AB4498 and sClassName
    // at 0x1AB43C8, both "StateNodeSetProperty_kFloat".
    static Symbol sId;
    // Name not in the reference map.
    static Symbol sClassName;
    static PropRegistry sPropRegistry;  // 0x1AB44B0 for float
    static ComMetaData sMetaData;       // 0x1AB4218 for float

    // Field names are not in the reference map; they follow the
    // properties.
    // "target_prop": the property to set.
    PropRef mTargetProp;
    // "load_resource_first", for resource paths.
    bool mLoadResourceFirst;
};

template <class T>
Symbol StateNodeSetProperty<T>::sId(MakeSetPropertyClassName(DrivenPropType<T>::kType));
template <class T>
Symbol StateNodeSetProperty<T>::sClassName = Symbol(MakeSetPropertyClassName(DrivenPropType<T>::kType));
template <class T>
PropRegistry StateNodeSetProperty<T>::sPropRegistry;
template <class T>
ComMetaData StateNodeSetProperty<T>::sMetaData;

static_assert(offsetof(StateNodeSetProperty<float>, mTargetProp) == 448);
static_assert(offsetof(StateNodeSetProperty<float>, mLoadResourceFirst) == 664);
static_assert(sizeof(StateNodeSetProperty<float>) == 672);
static_assert(sizeof(StateNodeSetProperty<ResourcePath>) == 672);
static_assert(sizeof(StateNodeSetProperty<Hmx::Color>) == 680);
static_assert(sizeof(StateNodeSetProperty<PropRef>) == 880);
static_assert(sizeof(StateNodeSetProperty<Waveform<float>>) == 704);
static_assert(sizeof(StateNodeSetProperty<Waveform<Hmx::Color>>) == 704);
static_assert(sizeof(StateNodeSetProperty<Vector3>) == 680);
static_assert(sizeof(StateNodeSetProperty<PropRotation>) == 680);
static_assert(sizeof(StateNodeSetProperty<PropXfm>) == 704);
static_assert(offsetof(StateNodeSetProperty<float>::Instance, mSetPropertyNode) == 72);
static_assert(sizeof(StateNodeSetProperty<float>::Instance) == 80);
static_assert(sizeof(StateNodeSetProperty<Symbol>::Instance) == 96);

// Points the metadata of the node's "value" at the target property's, so
// that the editor offers the target's range, default and allowed values.
// The waveforms' value keeps its own. Inlined into _Init. Name not in the
// reference map.
template <class T>
struct SetPropertyValueRemap {
    static void Apply(PropRegistry& registry) {
        PropPath::Node member;
        PropPath::Node value;
        const PropPath path((member = Symbol("value")), (value = Symbol("value")));
        PropInfo& info = const_cast<PropInfo&>(*registry.FindProp(path, nullptr));
        const TagRemapFunc remap = [](TagRemapArgs& args) {
            const StateNodeSetProperty<T>& node = static_cast<const StateNodeSetProperty<T>&>(args.mComponent);
            args.mEntity = node._GetContextEntityPrototype();
            args.mTargetCom = node.mTargetProp.Com(args.mEntity);
            args.mTargetPath = node.mTargetProp.mPath;
            args.mTargetInfo = node.mTargetProp.Find(args.mEntity, nullptr);
        };
        RemapPropMetadata(TypeSpecificMetadata(info, T()), remap);
    }
};
template <>
struct SetPropertyValueRemap<Waveform<float>> {
    static void Apply(PropRegistry& registry) {
        static_cast<void>(registry);
    }
};
template <>
struct SetPropertyValueRemap<Waveform<Hmx::Color>> {
    static void Apply(PropRegistry& registry) {
        static_cast<void>(registry);
    }
};

// Reconstructed from eboot.elf at 0x7239A0.
template <class T>
StateNodeSetProperty<T>::StateNodeSetProperty() : mTargetProp(), mLoadResourceFirst(true) {}

// Reconstructed from eboot.elf at 0x723BB0.
template <class T>
Symbol StateNodeSetProperty<T>::GetId() const {
    return sId;
}

// Reconstructed from eboot.elf at 0x723BC0.
template <class T>
Symbol StateNodeSetProperty<T>::GetClassName() const {
    return sClassName;
}

// Reconstructed from eboot.elf at 0x723BD0.
template <class T>
int StateNodeSetProperty<T>::CurrentRev() const {
    return const_cast<StateNodeSetProperty*>(this)->_GetPropRegistry().mCurrentRev;
}

// Reconstructed from eboot.elf at 0x723BF0.
template <class T>
bool StateNodeSetProperty<T>::IsA(Symbol type) const {
    for (const ComMetaData* metadata = &sMetaData; metadata != nullptr; metadata = metadata->mSuperclass) {
        if (metadata->mId == type) {
            return true;
        }
    }
    return false;
}

// Reconstructed from eboot.elf at 0x723C20.
template <class T>
Component* StateNodeSetProperty<T>::AsComponent() {
    return this;
}

// Reconstructed from eboot.elf at 0x723C30. The copy goes at the buffer's
// next 8-byte boundary, and its properties follow it.
template <class T>
char* StateNodeSetProperty<T>::_Imprint(char* buffer, Component** imprint) {
    ScopedImprint scope;
    auto* copy = reinterpret_cast<StateNodeSetProperty*>((reinterpret_cast<unsigned long>(buffer) + 7) & ~7UL);
    Component* result = nullptr;
    if (imprint != nullptr) {
        new (copy) StateNodeSetProperty(*this);
        copy->mImprinted = true;
        *imprint = copy;
        result = copy;
    }
    return this->_ImprintProps(reinterpret_cast<char*>(copy + 1), result);
}

// Reconstructed from eboot.elf at 0x723DD0.
template <class T>
PropRegistry& StateNodeSetProperty<T>::_GetPropRegistry() {
    return sPropRegistry;
}

// Reconstructed from eboot.elf at 0x723DE0.
template <class T>
ComMetaData& StateNodeSetProperty<T>::_GetMetaData() {
    return sMetaData;
}

// Reconstructed from eboot.elf at 0x723DF0.
template <class T>
const char* StateNodeSetProperty<T>::GetEditorText() const {
    if (!mTargetProp.IsComplete()) {
        return "";
    }
    char path[512] = {};
    mTargetProp.mPath.ToString(path, sizeof(path));
    const char* target;
    {
        FormatString text("%s");
        text << path;
        target = text.Str();
    }
    const char* const value = this->_GetValueDescription();
    FormatString text("%s =\n%s");
    text << target << value;
    return text.Str();
}

// Reconstructed from eboot.elf at 0x723F90.
template <class T>
StateNodeInstance* StateNodeSetProperty<T>::_CreateInstance(StateGraphInstance& graph) const {
    return new Instance(*this, graph);
}

// Reconstructed from eboot.elf at 0x723FF0.
template <class T>
bool StateNodeSetProperty<T>::_HasValueToSet() const {
    return mTargetProp.IsComplete();
}

// Reconstructed from eboot.elf at 0x724000.
template <class T>
bool StateNodeSetProperty<T>::_GetTargetRange(float* min, float* max) const {
    GetPropRefRange(this->_GetContextEntityPrototype(), mTargetProp, min, max);
    return true;
}

// Reconstructed from eboot.elf at 0x724040.
template <class T>
Component* StateNodeSetProperty<T>::_ChangeValueType(Symbol className, GameObject* object, PropertyType type) {
    object->DestroyComponent(className, false);
    return object->CreateComponent(Symbol(MakeSetPropertyClassName(type)), false);
}

// Reconstructed from eboot.elf at 0x722EB0.
template <class T>
void StateNodeSetProperty<T>::_Init(PropRegistry& registry, ComMetaData& metadata) {
    StateNodeSetProperty prototype;
    PropRegistry& own = prototype._GetPropRegistry();
    if (&registry != &own) {
        registry.AddInheritedRegistry(sClassName, own);
    }

    // A change to a target of the node's type redraws the node.
    PropRefMetadata& target = TypeSpecificMetadata(
        registry.RegisterProp(
            "target_prop", offsetof(StateNodeSetProperty, mTargetProp), kPropertyPropRef, sizeof(PropRef)),
        prototype.mTargetProp);
    target.mRefDefault = prototype.mTargetProp;
    static_cast<void>(Symbol("prop"));
    target.mHelp = String("The property this node will modify.");
    target.mEntity.mMode = TagBase<Entity*>::kModeFunction;
    target.mEntity.mFunction = StateBase::_GetContextEntityTagFunc();
    target.mAllowsReadOnly = false;
    target.mAllowsNonAnimatable = false;
    target.mExcludedTypes = {kPropertyAction};
    target.mRefOnChanged = [](const PropAccessorArgs& args) {
        const PropRef& ref = *static_cast<const PropRef*>(args.mValue);
        StateNodeSetProperty* const node = static_cast<StateNodeSetProperty*>(args.mComponent);
        const PropInfo* const info = ref.Find(node->_GetContextEntityPrototype(), nullptr);
        if (info == nullptr || info->mType == DrivenPropType<T>::kType) {
            node->_SendDisplayTextUpdated();
        }
    };

    // Revision 0 kept the value in "value" and "value_prop".
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision = [](const PropRegistry::RevisionArgs& args) {
        if (args.mSavedRev <= 0) {
            static_cast<StateNodeSetProperty*>(args.mComponent)->_LoadOldValue(*args.mSavedProps);
        }
    };
    if (&own == &registry) {
        registry.SetCurrentRev(1);
        registry.mRevisionFunc = onRevision;
    }

    StateNodeSetValue<T>::_InitAsSuperclass(registry, metadata);

    BoolMetadata& loadFirst = TypeSpecificMetadata(
        registry.RegisterProp(
            "load_resource_first", offsetof(StateNodeSetProperty, mLoadResourceFirst), kPropertyBool, 1),
        prototype.mLoadResourceFirst);
    loadFirst.mDefault = prototype.mLoadResourceFirst;
    static_cast<void>(Symbol("prop"));
    loadFirst.mHelp = String(
        "If true, this node will ensure the specified resource is loaded before setting the property. If it is "
        "not already loaded, it will load the resource asynchronously and apply the value as soon as it is "
        "ready. In general, it is best to preload resources when possible. Occasionally, the specified resource "
        "does not actually need to be loaded (or its loading will be managed by the target component), and this "
        "property allows that case to be managed cleanly.");
    loadFirst.mIsHidden._Set([](const TagArgs& args) {
        static_cast<void>(args);
        return DrivenPropType<T>::kType != kPropertyResourcePath;
    });

    SetPropertyValueRemap<T>::Apply(registry);

    metadata.mEditorAttributes[Symbol("state_creatable_node")] = Symbol("true");
    metadata.mEditorAttributes[Symbol("hotkey")] = Symbol("s");
    metadata.mDescription =
        "A state that sets the value of the targeted property to some specified value (or to the value of "
        "another property). It can set the property once upon activation, or persistently once per frame.";
}

// Reconstructed from eboot.elf at 0x722E00.
template <class T>
void StateNodeSetProperty<T>::_InitAsSuperclass(PropRegistry& registry, ComMetaData& metadata) {
    metadata.InitSuperclass("StateNodeSetProperty", sMetaData);
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

// Reconstructed from eboot.elf at 0x793170. The binary emits the factory
// in the group object with the class's Init.
template <class T>
Component* StateNodeSetProperty<T>::_Create() {
    return new StateNodeSetProperty();
}

// Reconstructed from eboot.elf at 0x791430.
template <class T>
void StateNodeSetProperty<T>::Init() {
    InitComponentClass<StateNodeSetProperty>();
}

// Reconstructed from eboot.elf at 0x7244F0.
template <class T>
bool StateNodeSetProperty<T>::Instance::_GetCurrentValue(T& value) const {
    value = mSetPropertyNode->mTargetProp.template Get<T>(this->_GetContextEntity());
    return true;
}

// Reconstructed from eboot.elf at 0x7245D0. A failure is reported; the
// report is compiled out and its arguments remain.
template <class T>
void StateNodeSetProperty<T>::Instance::_ApplyValue(const T& value) const {
    const PropRef& target = mSetPropertyNode->mTargetProp;
    if (!target.IsComplete()) {
        return;
    }
    GameObject* const object = target.Obj(this->_GetContextEntity());
    if (object == nullptr) {
        return;
    }
    Component* const com = object->FindCom(target.mCom);
    if (com == nullptr) {
        return;
    }
    void* storage = com;
    const PropInfo* const info = com->_GetPropRegistry().FindProp(target.mPath, &storage);
    if (info == nullptr || !_SetProp(*com, target.mPath, *info, storage, &value, DrivenPropType<T>::kType, false)) {
        static_cast<void>(com->MakeErrorName());
        char path[512] = {};
        target.mPath.ToString(path, sizeof(path));
    }
}

// A resource path is set through the graph's driver, which loads the
// resource first when the node asks for it.
// Reconstructed from eboot.elf at 0x7B3C10.
template <>
inline void StateNodeSetProperty<ResourcePath>::Instance::_ApplyValue(const ResourcePath& value) const {
    const bool loadFirst = mSetPropertyNode->mLoadResourceFirst;
    Entity* const entity = _GetContextEntity();
    if (loadFirst) {
        entity->GetRoot()->GetCom<StateGraphDriverCom>()->LoadAndSetResourcePathProperty(
            mSetPropertyNode->mTargetProp, value);
    } else {
        DynamicSetResourcePathProperty(entity, mSetPropertyNode->mTargetProp, value);
    }
}

// Reconstructed from eboot.elf at 0x724890.
template <class T>
bool StateNodeSetProperty<T>::Instance::_HasSameTarget(const StateNodeInstance& other) const {
    return mSetPropertyNode->mTargetProp.IsComplete() &&
           mSetPropertyNode->mTargetProp == static_cast<const Instance&>(other).mSetPropertyNode->mTargetProp;
}

// The registrations of the StateNodeSetProperty<T> types, each calling its
// types' Init (stategraph/StateNodeSetPropertyInit_A.o to _C.o): _A float
// to unsigned char, _B unsigned short to Symbol and _C ResourcePath to
// PropXfm, as the map's groups.
void StateNodeSetPropertyInit_A();  // 0x791400
void StateNodeSetPropertyInit_B();  // 0x793350
void StateNodeSetPropertyInit_C();  // 0x7A9AC0
