#pragma once

#include "entity/props/PropInfo.h"

class BinStream;
class Component;
class PropArrayBase;
class PropPath;

// The property helpers of entity/PropUtl.o that the reconstructed code
// calls. None is reconstructed.

// The storage size and alignment of a value of the type.
unsigned long PropertySize(PropertyType type);  // 0x1894B0
int PropertyAlignment(PropertyType type);       // 0x1894E0

// Inserts a value into the array property at the path: `value` is copied in,
// or a default element is constructed when it is null; `type` is the
// value's type, or kPropertyNone for the element's own. False when the
// value cannot be converted.
bool _InsertProp(
    Component& component,
    const PropPath& path,
    const PropInfo& info,
    PropArrayBase& array,
    const void* value,
    PropertyType type);  // 0x198310
// Removes the element the path's last index names from the array property.
void _RemoveProp(
    Component& component,
    const PropPath& path,
    const PropInfo& info,
    PropArrayBase& array);  // 0x199300

// Writes the values of the properties under the path to the stream. The
// map's signature is _StorePropState(Component const&, PropPath const&,
// BinStream&); this build adds the registry to walk, the component's own
// when it is null.
bool _StorePropState(
    const Component& component,
    const PropPath& path,
    BinStream& stream,
    const PropRegistry* registry);  // 0x19A1B0
// Reads values that _StorePropState wrote back into the properties under
// the path. The map's signature starts with an ObjPtr const&, which this
// build drops; the meaning of the flag is not recovered.
bool _ApplyStoredPropState(
    Component& component,
    const PropPath& path,
    BinStream& stream,
    bool force);  // 0x19E0F0
