#pragma once

#include <cstddef>

#include "entity/core/GameObject.h"
#include "entity/props/PropPath.h"
#include "utl/text/Symbol.h"

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

    // The object, or the invalid id (-1).
    GameObjectId mObject;
    // The component's class symbol.
    Symbol mCom;
    PropPath mPath;
};

static_assert(offsetof(PropRef, mCom) == 8);
static_assert(offsetof(PropRef, mPath) == 16);
static_assert(sizeof(PropRef) == 216);
