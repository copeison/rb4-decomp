# Components and properties

The component base, the component class registration and the property
system of the map's `entity` module. Names follow [naming.md](naming.md);
the entity objects that hold the components are in
[entity-resources.md](entity-resources.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `entity/ComMetaData.o` (part) | `0xE56C0`-`0xE6870`, `0xE69F0` | `src/entity/core/ComMetaData.cpp` |
| `entity/Component.o` (part) | `0xE7190`-`0xE9060`, `0xE9930`, `0xE9C30`, `0xAEA0`, `0xAED0` | `src/entity/core/Component.cpp` |
| `entity/EditorCom.o` (part) | `0xEABC0`-`0xEABD0` | `src/entity/core/EditorCom.cpp` |
| `entity/PropInfo.o` | `0x12DBE0`-`0x12DD00` | `src/entity/props/PropInfo.cpp` |
| `entity/PropRegistry.o` (part) | `0x1804C0`-`0x1813D0` | `src/entity/props/PropRegistry.cpp` |
| `PropArrayBase`, `entity/PropUtl.o` (part) | `0xA660`, `0x2ABE0`, `0x189380` | `src/entity/props/PropUtl.cpp` |
| `entity/PropMetadata.o` (part) | `0x12DD40`-`0x12ED70`, `0x134E50`-`0x13F620`, `0x140A20`-`0x143C20`, `0x14DA30`-`0x171C30`, `0x17E790`, `0x183640`-`0x183700` | `src/entity/props/PropMetadata.cpp` |
| `entity/InstanceCom.o` (registry) | `0x118E60`, `0x11B8A0`-`0x11B8E0` | `src/entity/core/InstanceCom.cpp` |
| `entity/TransCom.o` (registry) | `0x1AFFB0`, `0x1B4EF0`-`0x1BA960` | `src/entity/core/TransCom.cpp` |
| entity header (registry) | `0x1D21D0`, `0x1D5A40`-`0x1D8B80` | `src/entity/core/EntityHeaderCom.cpp` |

`PropPath` and the `PropUtl.o` helpers (`_GetProp`, `DumpProp`,
`PropertySize`, ...) are declared in `src/entity/props`; of the helpers only
`IsPropTestable` is reconstructed.

## Component

`Component` is 24 bytes: the vtable, the owning `GameObject`, and six
flags (imprinted, props synced, resources requested, resources ready,
entered, a flag nothing reads). Subclasses place their first member in the
tail padding at 22: `AudioEmitterCom`'s `is_named_emitter`, the "Options"
base of `RndOverlayOptionsCom` (constructor `0x46A870`), and the flag
`RndDefaults` writes on light components.

The vtable at `0x18E6518` has 41 slots; the emitted vtable matches it slot
for slot (`orbis-objdump -r`). The pure slots 4, 5, 7-10, 22 and 23 are each
class's identity, which the component macros generate.

| Slots | Members |
| ---: | --- |
| 0-3 | destructors, `Handle`, `GetEntity` |
| 4-9 | `GetId`, `GetClassName`, `GetInterfaceId`, `CurrentRev`, `IsA`, `AsComponent` |
| 10-13 | `_Imprint`, `_ImprintProps`, `_SaveStorage`, `_LoadStorage` |
| 14-18 | `_ResetEntered`, `_PostLoad`, `_Save`, `_GetPollOrderDeps`, `_GetComponentOrderDeps` |
| 19-23 | `_PostCreate`, `_PreDestroy`, `_LoadResources`, `_GetPropRegistry`, `_GetMetaData` |
| 24-30 | `_PostLoadStorage`, `_PreLoadResources`, `_PostLoadResources`, `_GetPollDeps`, `_CanHaveParentPollDep`, `_OnResourcesLoaded`, `_AreResourcesReady(pass)` |
| 31-35 | game mode `_Enter`, `_Exit`, `_Poll`; `_OnDeactivate`, `_OnActivate` |
| 36-40 | edit mode `_EditEnter`, `_EditExit`, `_EditPoll`; `_HasPostPoll`, `_PostPoll` |

Slot 30 receives the load pass (0 to 9) that `Entity::_LoadResources`
passes through `GameObject::AreResourcesReady` and
`Component::AreResourcesReady`; the light manager is ready only on pass 1.

An entity enters in the game mode (its flags' bits 1-2 are 1) or the edit
mode (2). The game mode polls through the poll window; the edit mode polls
every object in order. `Component::LoadResources` exits an entered
component in its mode, loads, and re-enters it unless slot 29 refuses.

`Component::sFactory` (`0x19E2900`) maps class ids to factories.
`sRegressionTesting` (`0x19E2970`) lets root-only classes be created on any
object.

## Component classes

`ComMetaData` (424 bytes) describes a class: its description and author,
its interface (the `mBaseId` of `GameObject::ComIndex`), category, the
entity resource classes it may live in, its aliases, required, default and
dependent classes, exported events and property registry. `Init`
(`0xE5B80`) fills the dependency lists of the classes it names, hashes
the class CRC (the class and interface names, the post-poll flag, the
order-dependency CRC and `PropRegistry::UpdateCRC`, `0x180D20`),
sets `mHasFlaggedProps` when a property has an instance access, through
`PropRegistry::VisitPropInfo` (its visitor copy is `0xE69F0`), and adds
the class to `sComMetaDataList` (`0x19E28B8`). The release build keeps the
loops of its stripped duplicate-id checks; the source leaves them out.
`AddExportedEvent` reserves room for 32 events and returns the new event's
parameter registry. `mReserved` (offset 83) has no identified reader and
three writers: `CharClipCom::_Init` (`0x2E6360`),
`CharIkBoneInterp::_Init` (`0x2F1580`) and the "PathMesh" registration
(`RndPathMeshCom::_Init` in the map, `0x5E45B0`), which also sets
`mLightweight`.

The class CRC takes `Component::GetClassCRC` (`0xE72A0`): a component of
the class, created through its factory, hashes its poll-order and
component-order dependencies (`_GetOrderDepsCRC`, `0xE7380`). Each list is
sorted without regard to case and hashed with FNV-1a, with `'-'` between
lists; the hash of non-empty lists starts from `671913016` rather than the
basis. Both names are inferred.

`Dump` (`0xE8330`) and `_ImprintRegistry` (`0xE8D90`) walk the registry
with `PropRegistry::VisitProps`, the template this build emits for each
visitor in place of the map's `VisitProps`/`_VisitProp` over
`std::function`s (`0xE9930`, `0xE9C30`). `Dump` prints "name: value" lines
indented by depth through `DumpProp` (`0x1A2610`, declared); its flag keeps
only the properties `IsPropTestable` allows. `_ImprintRegistry` places each
saved array's elements in the imprint buffer and copies the waveforms that
live in arrays or dynamic storage. Slots 12 and 13 (`0x107220`,
`0x107300`) hand the component to the prop-storage serializer
(`0x1073A0`, `0x108590`, with per-property writers `0x10F520` and
`0x112EB0`); that serializer is newer than the map and not modelled, so
they remain declared.

`PropRegistry::UpdateCRC` hashes the registry's revision, then, through
`VisitPropInfo`, each saved property's offsets, type, count and struct
revision, byte by byte with sign extension. Offsets of properties in the
component are hashed only when negative; with its flag only properties in
dynamic storage count. `RegisterAction` (`0x180910`) and
`RegisterDynamicProp` (`0x180980`) are reconstructed.

`GameObject::_CreateComponent` keeps one component per interface and, unless
the class sets `mAllowMultiple`, one per class. It refuses classes the
resource does not allow, editor and debug classes in archive mode, and
root-only classes off the root. It creates the required classes first and
the default classes after.

## Properties

A `PropRegistry` (160 bytes, aligned for its `std::function`) lists 40-byte
entries of a name and a `PropInfo`: the offset, the dynamic-storage offset
(`-1` for a member of the component), the type (`0x100` marks arrays) and
a reference-counted `PropMetadata`. `FindProp` walks a `PropPath` of up to
twelve name and index nodes and returns the address of the property.

`PropArray<T>` is a template over `PropArrayBase` whose element operations
are virtual. `Resize` and `_Insert` work through slots 5-7; `_Insert`
takes an element from the array itself before the storage moves.

A component's `_Imprint` (slot 10) copies it into an imprint buffer inside a
`ScopedImprint` (`0xB4D0`). Entities imprint the same way
(`Entity::_Imprint`, see [entity-resources.md](entity-resources.md)). The scope sets the thread's
`PropArrayBase::sImprinting`, which the binary reaches through the
thread-local descriptor at `0x19B0310` and the source models as
`thread_local`; `PropArrayBase::_Copy` (`0xBA70`) skips its copy while it is
set. The class copies are reconstructed for the render components.

## Property metadata

`PropMetadata` (vtable `0x18E7960`, 8 slots: the destructors,
`ValueToString`, `IsValueAllowed`, `Dump`, `GetAllowedValuesHelp`,
`DumpAllowedValuesHelp` and `GetStructRegistry`) is a list of attributes,
each a `TagBase<T>`: a mode (unset, constant, `TagArgs` function, script
`DataNode`, optional function, static function), the constant, the two
`std::function`s, the static function pointer, the script and the function
that aggregates the values of several selected components (the map's
`Agg*` functions). `Tag<T>` starts constant and `OptionalTag<T>` unset.
`TagArgs` holds the component and the property's path; `StaticTagArgs` the
entity and the path. The attributes are named by the keys `Dump`
(`0x133860`) and the data-driven registries (`0x202760`, `0x202F80`) use:
`display_name`, `group`, `is_hidden`, `read_only`, `debug`, `animatable`,
`no_test`, `saves`, `history`, `help`, an owned inline action and the
instance access (`set_on_instance`/`view_on_instance`). The object is 1528
bytes; subclasses start in its 16-byte tail padding.

The typed classes add five accessor pointers (setter, change callback,
getter and two unread ones) and a `mDefault`; `NumberMetadata<T>` adds
`allowed_values`, `min`, `max`, `slider`, `slider_log_resolution`,
`prefix`, `units` and `increment`, `IntMetadata<T>` a bitfield flag and
`FloatMetadata` the decimal places. `SymbolMetadata`,
`GameObjectIdMetadata`, `ResourcePathMetadata` (`resource_id`,
`custom_label`, `custom_extensions`, `is_relative_to_entity`),
`ColorMetadata`, `PropRefMetadata`, `WaveformMetadata<T>`,
`StructMetadata`/`GroupMetadata` (`summary`, the member registry) and
`ArrayMetadata` (the element `PropInfo`, the insert hooks, `summary`) are
modelled with `static_assert`ed layouts. `TypeSpecificMetadata` returns
the class for a value type (`0x140A20`, `0x141410` for `Vector3`,
`0x141B90`-`0x141D70`); the Euler angle overload (`0x141420`) has no
modelled value type.
`PropertyType` names the 23 types after `PropertySize` and
`PropMetadata::Create`.

`Create` (`0x12ED70`) makes the class for a type, constructing most of them
inline. Types 20 to 22, newer than the map, get `Vector3Metadata`,
`EulerMetadata` (radians shown in degrees) and `XfmMetadata` (a 36-byte
scale, rotation and translation, `PropXfm`); their `GetStructRegistry`
returns a shared member registry that the object's static initializer
(`0x17E790`) constructs and `InitRegistry` (`0x1406B0`, `0x140BA0`,
`0x1410A0`) fills: "x", "y", "z"; "pitch", "roll", "yaw" in degrees;
"scale", "rotation", "translation". Each member's accessors read and
write the whole struct through the parent's path (`_GetProp`/`_SetProp`,
or the struct behind the member's storage), and its default is a function
of the parent's default. A float waveform gets `FloatWaveformMetadata`, which adds a
number's range; actions get `ActionMetadata` from `RegisterAction`, whose
`std::function` the editor runs. The constructors' defaults: a number's
range is its type's limits (+-1e30 for float), a color is white, an object
id the null id. Each attribute's aggregator is one of the map's `Agg*`
templates: `AggUseFirstResult`, `AggUseMin`/`AggUseMax` for the bounds,
`AggUseNullIfConflicted`, `AggVectorIntersection` for allowed values and
`AggVectorUnion` for required classes, sorting by `AggCompare` (symbols
without regard to case). `TagBase<T>::Get` evaluates an attribute for a
component (`0x171380` for `String`, `0x1893F0` for `bool`).

`Dump` returns a `DataArray` of `(key value)` arrays, `(key value
"bool")` for flags (`MakeDataArray`, `0x82E00`/`0x83540`), for the
attributes that are constant and differ from their defaults, followed by
`DumpAllowedValuesHelp`'s `("allowed_values_help" text)`; subclasses add
their own keys (`min`/`max` when narrower than the type, `fixed_size`,
`summary`, `allowed_types` by `TypeName`, waveform types and blend modes
by name, ...). The typed `ValueToString`/`IsValueAllowed` overloads (map
names, `T const&`) sit behind thunks in the slots: a value is named by its
allowed value's label when asked, a number gets its prefix, units and
decimal places, a bitfield integer joins its set flags with "|", a color
shows its sRGB channels and an object its name; a value is allowed when
it is one of the allowed values, else within the range (numbers), of a
valid extension (paths, through `GetValidExtensions`), among
`GetAllowedObjects` (objects) or passing the reference's type, component
class and read-only/animatable/debug restrictions (`PropRef`).
`_GetAllowedValuesHelp<T>`/`_DumpAllowedValuesHelp<T>` (one copy per
class) list the allowed values as "- label: description" lines, the
labels' punctuation escaped. `OptionalTag<T>::operator()` gives a value
for one component or aggregates several, reporting conflicts.

`DefaultDisplayName` (`0x19E3D00`, call operator `0x14D1A0`) makes a name
from the path's last node: an index as a number; a name lower-cased with
spaces for underscores, its words capitalized for a struct or array and
"?" appended for a bool, cached by symbol. `StaticDefaultDisplayName`
(`0x19E3D30`, `0x1437A0`) returns the node as it is.

## Class registration

A class's inline `Init` is `InitComponentClass<T>` (`ComMetaData.h`): in
the "metadata" heap it sets `mCreatable` and `mSuperclassRoot` (false and
true for an abstract base such as `RndPostProcStageCom`), runs `_Init`,
records `{sizeof, alignof}` under the class id in the guarded size map
(`PropTypeSizes`, `0x19C5F20`) and names the type
(`PropTypeName<T>::sName`), installs the factory and the placement
constructor (`Component::sVtables`, `0x19E2938`) and calls
`ComMetaData::Init` with a prototype's `_HasPostPoll`.
`RegisterPropStruct<T>` names a struct property type for its arrays
(`PropArray<T>::sType`) and records its size. A builder (`_Init`)
constructs a prototype for the defaults, inherits the class's own
registry, registers each property with `RegisterProp` (building a "prop"
symbol it does not use) and fills its metadata, and sets the class
description; a revision handler (`PropRegistry::RevisionArgs`) upgrades
older saved properties.

`TransCom::_Init` (revision 2) saves the local transform through the
floats "x" to "z_scale" (the angles edited in degrees, the scales kept
off zero) and adds unsaved views: "position", "scale" and "rotation"
(types 20 and 21, for PropAnim), "local_transform" (its axes computed
through `MakeRotMatrix`) and "world_transform". The root object's
transform is read-only and not animatable. "parent" lists the objects
`GetValidTransParentsFromObject` (`0x1141C0`, declared) gives, and
"utilities" holds the "reset" action and "copy_world_xfm", which uses
`TransUtl::CalcNewLocalXfm` (`0x1C8220`, declared). The revision handler
converts the rotation matrix "mxx" to "mzz" of revisions 0 and 1, read
through `_GetProp` from the component in `RevisionArgs::mSavedProps`. The
local transform has `PropXfm`'s layout, which `TransCom::LocalXfm`
converts from. `EntityHeaderCom::_Init` (revision 1) hides the instancing,
statistics and icon camera properties outside a TransEntityResource and
adds the debug "poll_order" view: the poll order's size and ten objects
from a shared start index (`0x19E4CE8`). Its revision handler clears a
non-finite icon camera translation.

## Weak evidence

- The slot names 3, 5, 6, 9, 12-14, 17, 18, 24-26 and 28-30 rest on their
  bodies and callers. `_CanHaveParentPollDep` and the resource-load pair are
  map names of `InstanceCom` methods; this build's `InstanceCom` keeps its
  own parent poll dependency in slot 42 (`_HasParentPollDep`).
- `ComMetaData::mLightweight`, `mEditorRestrictions` and `mClassName`, and
  `Component::mPropsSynced`, are named from their few readers.
- `GroupMetadata` as the struct metadata class is inferred. The map puts
  `Dump` on `GroupMetadata` and `ValueToString` on `StructMetadata`, the
  reverse of the slots of the classes so named here, which suggests the
  two names are swapped.
- `Vector3Metadata`, `EulerMetadata`, `XfmMetadata`, `PropXfm`,
  `FloatWaveformMetadata`, `ColorWaveformMetadata` and
  `EulerMetadata::sZeroAngles` (`0x19E6630`, zero data in the math
  module's globals) are inferred. `GameObjectIdMetadata::mExcludedObjects`
  and `PropRefMetadata::mExcludedTypes` are named from their union
  aggregators, `PropRefMetadata::mObject` from its type.
- Attribute names that `Dump` does not write: the three set-by-default
  `PropMetadata` tags (`mExtraFlag0`-`2`, no reader), `mInstanceFile`,
  the number tags `mReserved` and `mEnabled`, `GameObjectIdMetadata`'s
  `mEntity`, `mAllowedObjects`, `mRequiredInterfaces` and `mFilterClass`,
  `ResourcePathMetadata`'s `mAllowsInlined`, `mEntity`,
  `mDefaultExtension`, `mAllowsUntyped` and `mFlag1`, `PropRefMetadata`'s
  `mAllowedComs` and `mAllowsDebug` (named from `IsValueAllowed`),
  `ArrayMetadata`'s hooks and `mSummary`, and `IntMetadata::mBitfield`
  (which `Dump` writes as "prefix").
- Names of the helpers absent from the map: `GetAllowedValueName`,
  `IntMetadata::FormatValue`, `InitRegistry` and the member accessors,
  `OptionalTagArgs`' fields and `TagValueEquals`. `TypeName` takes a
  second flag (the "k" prefix) the map's signature lacks.
- `PropTypeSizes`, `PropTypeName`, `InitComponentClass`,
  `RegisterPropStruct` and `RevisionArgs`' fields are inferred.
  `LoadWaveformResources` (`0x1C9A90`), which `LoadResources` calls, takes
  the map's name: it lies in `entity/Waveform.o`'s range and visits the
  waveform property types 18 and 19.
