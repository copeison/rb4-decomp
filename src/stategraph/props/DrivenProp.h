#pragma once

#include <cstddef>
#include <functional>

#include "entity/core/Component.h"
#include "entity/core/Entity.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropMetadata.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRef.h"
#include "entity/props/PropRegistry.h"
#include "entity/props/PropUtl.h"
#include "entity/waveform/Waveform.h"
#include "math/color/Color.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

// The property type a DrivenProp<T> reads its driving property as, and the
// value it starts with: the value-initialized T. Names not in the reference
// map.
template <class T, PropertyType type>
struct DrivenPropTypeBase {
    static constexpr PropertyType kType = type;
    static T Default() {
        return T{};
    }
};

template <class T>
struct DrivenPropType;
template <>
struct DrivenPropType<float> : DrivenPropTypeBase<float, kPropertyFloat> {};
template <>
struct DrivenPropType<signed char> : DrivenPropTypeBase<signed char, kPropertyInt8> {};
template <>
struct DrivenPropType<short> : DrivenPropTypeBase<short, kPropertyInt16> {};
template <>
struct DrivenPropType<int> : DrivenPropTypeBase<int, kPropertyInt> {};
template <>
struct DrivenPropType<long> : DrivenPropTypeBase<long, kPropertyInt64> {};
template <>
struct DrivenPropType<unsigned char> : DrivenPropTypeBase<unsigned char, kPropertyUInt8> {};
template <>
struct DrivenPropType<unsigned short> : DrivenPropTypeBase<unsigned short, kPropertyUInt16> {};
template <>
struct DrivenPropType<unsigned int> : DrivenPropTypeBase<unsigned int, kPropertyUInt> {};
template <>
struct DrivenPropType<unsigned long> : DrivenPropTypeBase<unsigned long, kPropertyUInt64> {};
template <>
struct DrivenPropType<bool> : DrivenPropTypeBase<bool, kPropertyBool> {};
template <>
struct DrivenPropType<GameObjectId> : DrivenPropTypeBase<GameObjectId, kPropertyGameObjectId> {};
template <>
struct DrivenPropType<Symbol> : DrivenPropTypeBase<Symbol, kPropertySymbol> {};
template <>
struct DrivenPropType<ResourcePath> : DrivenPropTypeBase<ResourcePath, kPropertyResourcePath> {};
template <>
struct DrivenPropType<PropRef> : DrivenPropTypeBase<PropRef, kPropertyPropRef> {};
template <>
struct DrivenPropType<Waveform<float>> : DrivenPropTypeBase<Waveform<float>, kPropertyWaveformFloat> {};
template <>
struct DrivenPropType<Waveform<Hmx::Color>> : DrivenPropTypeBase<Waveform<Hmx::Color>, kPropertyWaveformColor> {};

// A colour starts opaque black, as the binary's prototypes (0x410EF0) and
// reads (0x19F410) do; the reconstruction's Hmx::Color default constructor
// leaves the channels zero.
template <>
struct DrivenPropType<Hmx::Color> {
    static constexpr PropertyType kType = kPropertyColor;
    static Hmx::Color Default() {
        return Hmx::Color(0.0F, 0.0F, 0.0F, 1.0F);
    }
};

// The property types a property of type T may be driven by: the type
// itself by default. The map has the template (with the waveform copies
// out of line); its body is inlined into the colour Register.
template <class T>
eastl::vector<PropertyType> GetCompatibleTypes() {
    return eastl::vector<PropertyType>{DrivenPropType<T>::kType};
}

// A float may be driven by any number. RndSceneCom.o's copy of the
// specialization.
// Reconstructed from eboot.elf at 0x414210.
template <>
inline eastl::vector<PropertyType> GetCompatibleTypes<float>() {
    eastl::vector<PropertyType> types;
    types.reserve(9);
    types = eastl::vector<PropertyType>{
        kPropertyInt8,
        kPropertyInt16,
        kPropertyInt,
        kPropertyInt64,
        kPropertyUInt8,
        kPropertyUInt16,
        kPropertyUInt,
        kPropertyUInt64};
    types.push_back(kPropertyFloat);
    return types;
}

// The value of the component's property at the path, read as T; the type's
// start value when the property cannot be found or read, which a
// compiled-out report notes. The binary emits the colour copy at 0x19F410
// and inlines the others. Name not in the reference map.
template <class T>
T GetPropValue(const Component& component, const PropPath& path) {
    T value = DrivenPropType<T>::Default();
    Component& com = const_cast<Component&>(component);
    void* storage = &com;
    const PropInfo* const info = com._GetPropRegistry().FindProp(path, &storage);
    if (info == nullptr || !_GetProp(com, path, *info, storage, &value, DrivenPropType<T>::kType)) {
        // The report is compiled out; its arguments remain.
        static_cast<void>(com.MakeErrorName());
        char text[512] = {};
        path.ToString(text, sizeof(text));
    }
    return value;
}

// Sets the default of the "value" member's metadata: a reference keeps it
// in its own tag. Name not in the reference map.
template <class Metadata, class T>
void SetDrivenPropDefault(Metadata& metadata, const T& value) {
    metadata.mDefault = value;
}
inline void SetDrivenPropDefault(PropRefMetadata& metadata, const PropRef& value) {
    metadata.mRefDefault = value;
}

// A value a state graph object either holds itself or reads from a
// property of another component (the map's DrivenProp<T>, instantiated in
// the stategraph objects: StateConditionComparisonInit_A.o,
// StateNodeAnimate.o and others). Its "value", "prop" and "is_driven"
// properties are registered by Register. Field names are not in the
// reference map.
template <class T>
class DrivenProp {
public:
    DrivenProp() : mValue(DrivenPropType<T>::Default()), mProp(), mDriven(false) {}
    // An undriven value. Inlined into the owners' constructors, for example
    // StateTransition's at 0x7F5220.
    explicit DrivenProp(const T& value) : mValue(value), mProp(), mDriven(false) {}

    // The value: the driving property's, read through its component, when
    // the value is driven; the own value otherwise. A driving property that
    // cannot be found or read gives the type's default. The map has it out
    // of line in StateNodeAnimate.o; this build inlines it, for example into
    // StateTransitionInstance::Poll at 0x7F84B0.
    T GetValue(Entity* entity) const;
    // Copies the DrivenProp at the path of the component's properties,
    // member by member. The map has the float copy in StateNodeAnimate.o;
    // this build emits it at 0x716F10, with the reference read by
    // GetPropValue<PropRef> (0xFA090).
    void CopyFromProperty(const Component& component, const PropPath& path);

    // Registers the type as the struct property `name` at `offset`:
    // "value", "prop" (a property reference into the entity `entity`
    // gives, of a type GetCompatibleTypes allows) and the hidden
    // "is_driven", with a ">" button that switches between them, and
    // returns the struct's metadata. The editor hides the half that does
    // not apply. A value saved before the struct's revision 1 was a plain
    // "raw_value". The binary emits a copy per user; RndSceneCom.o holds
    // the float and colour ones (0x4107C0, 0x410EF0).
    GroupMetadata& Register(
        PropRegistry& registry,
        Symbol name,
        int offset,
        const std::function<Entity*(const TagArgs&)>& entity);

    // The DrivenProp whose member the arguments' path names: the path
    // without its last node, found from the component. Inlined into
    // Register's tag functions.
    static DrivenProp& _GetThisRef(const TagArgs& args);

    T mValue;
    PropRef mProp;
    bool mDriven;
};

template <class T>
T DrivenProp<T>::GetValue(Entity* entity) const {
    if (!mDriven || !mProp.IsComplete()) {
        return mValue;
    }
    T value = DrivenPropType<T>::Default();
    GameObject* const object = mProp.Obj(entity);
    if (object == nullptr) {
        return value;
    }
    Component* const com = object->FindCom(mProp.mCom);
    if (com == nullptr) {
        return value;
    }
    void* storage = com;
    const PropInfo* const info = com->_GetPropRegistry().FindProp(mProp.mPath, &storage);
    if (info == nullptr || !_GetProp(*com, mProp.mPath, *info, storage, &value, DrivenPropType<T>::kType)) {
        // The report is compiled out; its arguments remain.
        static_cast<void>(com->MakeErrorName());
        char path[512] = {};
        mProp.mPath.ToString(path, sizeof(path));
    }
    return value;
}

// A driven waveform is read whole from the property
// (PropRef::Get<Waveform<float>>, 0x2D0470); an undriven one is copied,
// which draws a new evaluation state. Inlined into
// StateTransitionInstance::Poll (0x7F84B0).
template <>
inline Waveform<float> DrivenProp<Waveform<float>>::GetValue(Entity* entity) const {
    if (mDriven && mProp.IsComplete()) {
        return mProp.Get<Waveform<float>>(entity);
    }
    return mValue;
}

// The colour waveform reads the same way, through
// PropRef::Get<Waveform<Hmx::Color>>.
template <>
inline Waveform<Hmx::Color> DrivenProp<Waveform<Hmx::Color>>::GetValue(Entity* entity) const {
    if (mDriven && mProp.IsComplete()) {
        return mProp.Get<Waveform<Hmx::Color>>(entity);
    }
    return mValue;
}

// Reconstructed from eboot.elf at 0x716F10.
template <class T>
void DrivenProp<T>::CopyFromProperty(const Component& component, const PropPath& path) {
    static Symbol value;
    if (value == Symbol()) {
        value = Symbol("value");
    }
    static Symbol prop;
    if (prop == Symbol()) {
        prop = Symbol("prop");
    }
    static Symbol is_driven;
    if (is_driven == Symbol()) {
        is_driven = Symbol("is_driven");
    }
    PropPath::Node node;
    PropPath valuePath = path;
    valuePath.PushNode(node = value);
    mValue = GetPropValue<T>(component, valuePath);
    PropPath propPath = path;
    propPath.PushNode(node = prop);
    mProp = GetPropValue<PropRef>(component, propPath);
    PropPath drivenPath = path;
    drivenPath.PushNode(node = is_driven);
    mDriven = GetPropValue<bool>(component, drivenPath);
}

template <class T>
DrivenProp<T>& DrivenProp<T>::_GetThisRef(const TagArgs& args) {
    PropPath path = args.mPath;
    path.PopNode();
    Component& com = const_cast<Component&>(args.mComponent);
    void* storage = &com;
    com._GetPropRegistry().FindProp(path, &storage);
    return *static_cast<DrivenProp*>(storage);
}

// The colour copy is at 0x410EF0. The ">" button's lambda (0x414610), the
// tag functions (0x414BF0, 0x414DC0) and the revision handler (0x415010)
// follow the float copy.
// Reconstructed from eboot.elf at 0x4107C0.
template <class T>
GroupMetadata& DrivenProp<T>::Register(
    PropRegistry& registry,
    Symbol name,
    int offset,
    const std::function<Entity*(const TagArgs&)>& entity) {
    GroupMetadata& group = static_cast<GroupMetadata&>(
        *registry.RegisterProp(name.Str(), offset, kPropertyStruct, sizeof(DrivenProp)).mMetadata.Get());
    group.mKind = 2;
    DrivenProp prototype;

    // The map's lambda takes TriggerArgs; the button flips "is_driven".
    group.mInlineAction = new PropMetadata::InlineAction();
    group.mInlineAction->mLabel = ">";
    group.mInlineAction->mEnabled = true;
    group.mInlineAction->mAction = [](const TagArgs& args) {
        static Symbol is_driven;
        if (is_driven == Symbol()) {
            is_driven = Symbol("is_driven");
        }
        PropPath::Node node;
        PropPath path = args.mPath;
        path.PushNode(node = is_driven);
        Component& com = const_cast<Component&>(args.mComponent);
        bool driven = false;
        void* storage = &com;
        const PropInfo* info = com._GetPropRegistry().FindProp(path, &storage);
        if (info == nullptr || !_GetProp(com, path, *info, storage, &driven, kPropertyBool)) {
            // The report is compiled out; its arguments remain.
            static_cast<void>(com.MakeErrorName());
            char text[512] = {};
            path.ToString(text, sizeof(text));
        }
        storage = &com;
        const bool toggled = !driven;
        info = com._GetPropRegistry().FindProp(path, &storage);
        if (info == nullptr || !_SetProp(com, path, *info, storage, &toggled, kPropertyBool, false)) {
            static_cast<void>(com.MakeErrorName());
            char text[512] = {};
            path.ToString(text, sizeof(text));
        }
    };

    PropRegistry& members = group.mRegistry;
    auto& value = TypeSpecificMetadata(
        members.RegisterProp("value", offsetof(DrivenProp, mValue), DrivenPropType<T>::kType, sizeof(T)),
        prototype.mValue);
    SetDrivenPropDefault(value, prototype.mValue);
    static_cast<void>(Symbol("prop"));
    value.mHelp =
        String("The value of this property. Click '>' to drive this property by the value of another property.");
    value.mIsHidden._Set([](const TagArgs& args) { return _GetThisRef(args).mDriven; });

    PropRefMetadata& prop = TypeSpecificMetadata(
        members.RegisterProp("prop", offsetof(DrivenProp, mProp), kPropertyPropRef, sizeof(PropRef)),
        prototype.mProp);
    prop.mRefDefault = prototype.mProp;
    static_cast<void>(Symbol("prop"));
    prop.mHelp = String(
        "The property that drives this property's value. Click '>' to specify this property's value directly.");
    prop.mIsHidden._Set([](const TagArgs& args) { return !_GetThisRef(args).mDriven; });
    prop.mAllowedTypes = GetCompatibleTypes<T>();
    if (entity) {
        prop.mEntity.mMode = TagBase<Entity*>::kModeFunction;
        prop.mEntity.mFunction = entity;
    }

    BoolMetadata& driven = TypeSpecificMetadata(
        members.RegisterProp("is_driven", offsetof(DrivenProp, mDriven), kPropertyBool, 1), prototype.mDriven);
    driven.mDefault = prototype.mDriven;
    static_cast<void>(Symbol("prop"));
    driven.mIsHidden = true;

    // Revision 0 saved the value alone, as "raw_value".
    std::function<void(const PropRegistry::RevisionArgs&)> onRevision =
        [](const PropRegistry::RevisionArgs& args) {
            if (args.mSavedRev > 0 || args.mStorage == nullptr) {
                return;
            }
            PropPath::Node node;
            PropPath path = *args.mPath;
            path.PushNode(node = Symbol("raw_value"));
            static_cast<DrivenProp*>(args.mStorage)->mValue = GetPropValue<T>(*args.mSavedProps, path);
        };
    members.SetCurrentRev(1);
    members.mRevisionFunc = onRevision;
    return group;
}

static_assert(offsetof(DrivenProp<float>, mProp) == 8);
static_assert(offsetof(DrivenProp<float>, mDriven) == 224);
static_assert(sizeof(DrivenProp<float>) == 232);
static_assert(offsetof(DrivenProp<Hmx::Color>, mProp) == 16);
static_assert(offsetof(DrivenProp<Hmx::Color>, mDriven) == 232);
static_assert(sizeof(DrivenProp<Hmx::Color>) == 240);
static_assert(offsetof(DrivenProp<Waveform<float>>, mProp) == 40);
static_assert(offsetof(DrivenProp<Waveform<float>>, mDriven) == 256);
static_assert(sizeof(DrivenProp<Waveform<float>>) == 264);
