#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "entity/props/PropPath.h"
#include "utl/text/Symbol.h"

class Component;
class Entity;
class PropInfo;

// A reference to a property of a component of an object: the object, the
// component's class and the path to the property (entity/PropRef.o; the
// constructors are at 0x17F4F0-0x17F60F). Properties of type 16 store one
// in 216 bytes, for example EntityHeaderCom's "show_entity".
// Only the constructors are reconstructed. Field names are not in the
// reference map.
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
    // The map's ValueToString(EntityPtr) const: the reference as editor
    // text, through the static Description.
    const char* ValueToString(Entity* entity) const;  // 0x17FA50
    bool operator==(const PropRef& other) const;  // 0x1801E0
    // The map's Get<T>(EntityPtr): the referenced property's value in the
    // entity. Instantiated where it is used: Waveform<float>'s at 0x2D0470.
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
