#pragma once

#include "entity/props/PropInfo.h"

class BinStream;
class Component;
class Entity;
class PropArrayBase;
class PropPath;
class PropRef;
class ResourcePath;
class TextStream;

// The property helpers of entity/PropUtl.o that the reconstructed code
// calls. Only IsPropTestable is reconstructed.

// Whether a test may set the property: it is saved, recorded in the
// history, not marked "no_test", and neither hidden nor read-only for the
// component. The map's signature starts with an ObjPtr const&.
bool IsPropTestable(const Component& component, const PropPath& path, const PropInfo& info);  // 0x189380
// Prints the property's value to the stream. The map's signature starts
// with an ObjPtr const&. Not reconstructed.
void DumpProp(
    const Component& component,
    const PropInfo& info,
    const PropPath& path,
    const void* storage,
    TextStream* stream);  // 0x1A2610

// Reads the property at `storage` (or through its getter) into `value`,
// converted to `type`. False when the value cannot be converted. Not
// reconstructed.
bool _GetProp(
    const Component& component,
    const PropPath& path,
    const PropInfo& info,
    const void* storage,
    void* value,
    PropertyType type);  // 0x18B390
// Writes `value`, of `type`, to the property at `storage` (or through its
// setter), converting it to the property's type. False when the value
// cannot be converted. The meaning of the flag is not recovered. Not
// reconstructed.
bool _SetProp(
    Component& component,
    const PropPath& path,
    const PropInfo& info,
    void* storage,
    const void* value,
    PropertyType type,
    bool force);  // 0x18A440

// The type's name, such as "Float" or "ArrayStruct", or with a "k" prefix
// ("kFloat") when `constant` is set; empty for an unknown type. The map's
// signature is TypeName(PropertyType).
const char* TypeName(PropertyType type, bool constant);  // 0x189510

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

// The value at `value` of the property at the path as editor text, through
// its metadata. The map's ValueToString(ObjPtr const&, Component const&,
// PropInfo const&, PropPath const&, void const*); this build drops the
// object.
const char* ValueToString(
    const Component& component,
    const PropInfo& info,
    const PropPath& path,
    const void* value);  // 0x1A03F0

// Sets the resource path property the reference names in the entity, when
// it is one and differs. The map's signature starts with an EntityPtr
// const&. Not reconstructed.
void DynamicSetResourcePathProperty(Entity* entity, const PropRef& ref, const ResourcePath& path);  // 0x1A49E0
