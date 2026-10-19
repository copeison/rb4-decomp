#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "entity/core/GameObject.h"
#include "entity/props/PropInfo.h"
#include "entity/props/PropPath.h"
#include "entity/props/PropRegistry.h"
#include "entity/props/PropUtl.h"
#include "utl/text/Symbol.h"

class Entity;

// A reference to a property of a component of an object: the object, the
// component's class and the path to the property (entity/PropRef.o; the
// constructors are at 0x17F4F0-0x17F60F). Properties of type 16 store one
// in 216 bytes, for example EntityHeaderCom's "show_entity".
// Only the constructors and the comparisons are reconstructed. Field names
// are not in the reference map.
class PropRef {
public:
    // No object, no component and an empty path.
    PropRef();  // 0x17F4F0
    PropRef(GameObjectId object, Symbol com, const PropPath& path);  // 0x17F550
    // A reference to a component of no particular object.
    PropRef(Symbol com, const PropPath& path);  // 0x17F5B0

    // Whether the reference names an object, a component and a non-empty
    // path.
    bool IsComplete() const;  // 0x17F730
    // The map's Obj(EntityPtr) const: the referenced object in the entity,
    // or null.
    GameObject* Obj(Entity* entity) const;  // 0x17F610
    // The map's Com(EntityPtr) const: the referenced component in the
    // entity, or null.
    Component* Com(Entity* entity) const;  // 0x17F650
    // The map's Find(EntityPtr, void**) const: the referenced property's
    // info, with its storage in `storage` when it is given, or null.
    const PropInfo* Find(Entity* entity, void** storage) const;  // 0x17F850
    // Declared for the state graph nodes; not reconstructed. The
    // referenced object, component and property's info in the entity;
    // false when one is missing. Name not in the reference map; the
    // evidence is weak.
    bool Resolve(Entity* entity, GameObject*& object, Component*& com, const PropInfo*& info) const;  // 0x17FCB0
    // The map's Description(EntityPtr) const: the reference as editor
    // text, through the static Description (0x17FDA0).
    const char* Description(Entity* entity) const;  // 0x17FA50
    // The map's ValueToString(EntityPtr) const: the referenced property's
    // value as text, or empty when the object or component is missing.
    // Not reconstructed.
    const char* ValueToString(Entity* entity) const;  // 0x17FF20
    bool operator==(const PropRef& other) const;  // 0x1801E0
    // Orders references by object, component class and path, as
    // eastl::map<PropRef, ...> compares them (ClipBlendTreeCom.o's edit
    // values).
    bool operator<(const PropRef& other) const;  // 0x180200
    // The map's Exists(EntityPtr) const: whether the entity has the object,
    // its component and the property. Not reconstructed.
    bool Exists(Entity* entity) const;  // 0x17F760
    // The map's Get<T>(EntityPtr): the referenced property's value in the
    // entity, read as PropValueType<T>, or the type's default when the
    // object, component or property is missing. Instantiated where it is
    // used: bool's at 0x302750, Waveform<float>'s at 0x2D0470.
    template <class T>
    T Get(Entity* entity) const;

    // The object, or the invalid id (-1).
    GameObjectId mObject;
    // The component's class symbol.
    Symbol mCom;
    PropPath mPath;
};

static_assert(offsetof(PropRef, mCom) == 8);
static_assert(offsetof(PropRef, mPath) == 16);
static_assert(sizeof(PropRef) == 216);

// The property type a value type is read and written as, which PropRef::Get
// and the key components pass to _GetProp and _SetProp, specialized below
// for the property value types. Name not in the reference map.
template <class T>
struct PropValueType;

class ResourcePath;
class Vector3;
struct PropRotation;
struct PropXfm;
template <class T>
class Waveform;
namespace Hmx {
class Color;
}

template <>
struct PropValueType<bool> {
    static constexpr PropertyType kType = kPropertyBool;
};
template <>
struct PropValueType<signed char> {
    static constexpr PropertyType kType = kPropertyInt8;
};
template <>
struct PropValueType<short> {
    static constexpr PropertyType kType = kPropertyInt16;
};
template <>
struct PropValueType<int> {
    static constexpr PropertyType kType = kPropertyInt;
};
template <>
struct PropValueType<long> {
    static constexpr PropertyType kType = kPropertyInt64;
};
template <>
struct PropValueType<unsigned char> {
    static constexpr PropertyType kType = kPropertyUInt8;
};
template <>
struct PropValueType<unsigned short> {
    static constexpr PropertyType kType = kPropertyUInt16;
};
template <>
struct PropValueType<unsigned int> {
    static constexpr PropertyType kType = kPropertyUInt;
};
template <>
struct PropValueType<unsigned long> {
    static constexpr PropertyType kType = kPropertyUInt64;
};
template <>
struct PropValueType<float> {
    static constexpr PropertyType kType = kPropertyFloat;
};
template <>
struct PropValueType<GameObjectId> {
    static constexpr PropertyType kType = kPropertyGameObjectId;
};
template <>
struct PropValueType<Symbol> {
    static constexpr PropertyType kType = kPropertySymbol;
};
template <>
struct PropValueType<ResourcePath> {
    static constexpr PropertyType kType = kPropertyResourcePath;
};
template <>
struct PropValueType<Hmx::Color> {
    static constexpr PropertyType kType = kPropertyColor;
};
template <>
struct PropValueType<PropRef> {
    static constexpr PropertyType kType = kPropertyPropRef;
};
template <>
struct PropValueType<Vector3> {
    static constexpr PropertyType kType = kPropertyVector3;
};
template <>
struct PropValueType<PropRotation> {
    static constexpr PropertyType kType = kPropertyEuler;
};
template <>
struct PropValueType<PropXfm> {
    static constexpr PropertyType kType = kPropertyXfm;
};
template <>
struct PropValueType<Waveform<float>> {
    static constexpr PropertyType kType = kPropertyWaveformFloat;
};
template <>
struct PropValueType<Waveform<Hmx::Color>> {
    static constexpr PropertyType kType = kPropertyWaveformColor;
};

template <class T>
T PropRef::Get(Entity* entity) const {
    GameObject* const object = Obj(entity);
    if (object == nullptr) {
        return T();
    }
    Component* const component = object->FindCom(mCom);
    if (component == nullptr) {
        return T();
    }
    T value = T();
    void* storage = component;
    const PropInfo* const info = component->_GetPropRegistry().FindProp(mPath, &storage);
    if (info == nullptr || !_GetProp(*component, mPath, *info, storage, &value, PropValueType<T>::kType)) {
        static_cast<void>(component->MakeErrorName());
        char text[512] = {};
        mPath.ToString(text, sizeof(text));
    }
    return value;
}
