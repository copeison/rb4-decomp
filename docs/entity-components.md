# Components and properties

The component base, the component class registration and the property
system of the map's `entity` module. Names follow [naming.md](naming.md);
the entity objects that hold the components are in
[entity-resources.md](entity-resources.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `entity/ComMetaData.o` (part) | `0xE56C0`-`0xE6870`, `0xE69F0` | `src/entity/core/ComMetaData.cpp` |
| `entity/Component.o` (part) | `0xE7190`-`0xE9060`, `0xAEA0`, `0xAED0` | `src/entity/core/Component.cpp` |
| `entity/EditorCom.o` (part) | `0xEABC0`-`0xEABD0` | `src/entity/core/EditorCom.cpp` |
| `entity/PropInfo.o` | `0x12DBE0`-`0x12DD00` | `src/entity/props/PropInfo.cpp` |
| `entity/PropRegistry.o` (part) | `0x1804C0`-`0x1813D0` | `src/entity/props/PropRegistry.cpp` |
| `PropArrayBase` | `0xA660`, `0x2ABE0` | `src/entity/props/PropUtl.cpp` |
| `entity/PropMetadata.o` (part) | `0x12DD40`-`0x12E610`, `0x140A20`-`0x143C20` | `src/entity/props/PropMetadata.cpp` |
| `entity/InstanceCom.o` (registry) | `0x118E60`, `0x11B8A0`-`0x11B8E0` | `src/entity/core/InstanceCom.cpp` |

`PropPath` and the `PropUtl.o` helpers are declared in `src/entity/props`;
their bodies are not reconstructed.

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
order-dependency CRC and `PropRegistry::UpdateCRC`, `0x180D20`, declared),
sets `mHasFlaggedProps` when a property has an instance access, through
`PropRegistry::VisitPropInfo` (its visitor copy is `0xE69F0`), and adds
the class to `sComMetaDataList` (`0x19E28B8`). The release build keeps the
loops of its stripped duplicate-id checks; the source leaves them out.
`AddExportedEvent` reserves room for 32 events and returns the new event's
parameter registry.

The class CRC takes `Component::GetClassCRC` (`0xE72A0`): a component of
the class, created through its factory, hashes its poll-order and
component-order dependencies (`_GetOrderDepsCRC`, `0xE7380`). Each list is
sorted without regard to case and hashed with FNV-1a, with `'-'` between
lists; the hash of non-empty lists starts from `671913016` rather than the
basis. Both names are inferred. `Dump` and `_ImprintRegistry` remain
declared: they walk the registry through property visitors (`0xE9930`,
`0xE9C30`) and `PropUtl` helpers that are not modelled. Slots 12 and 13
(`0x107220`, `0x107300`) belong to the prop-storage serializer outside
`Component.o` and remain declared.

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
the class for a value type (`0x140A20`, `0x141B90`-`0x141D70`).
`PropertyType` names the 23 types after `PropertySize` and
`PropMetadata::Create`.

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

## Weak evidence

- The slot names 3, 5, 6, 9, 12-14, 17, 18, 24-26 and 28-30 rest on their
  bodies and callers. `_CanHaveParentPollDep` and the resource-load pair are
  map names of `InstanceCom` methods; this build's `InstanceCom` keeps its
  own parent poll dependency in slot 42 (`_HasParentPollDep`).
- `ComMetaData::mLightweight`, `mEditorRestrictions` and `mClassName`, and
  `Component::mPropsSynced`, are named from their few readers.
- `GroupMetadata` as the struct metadata class is inferred.
- Attribute names that `Dump` does not write: the three set-by-default
  `PropMetadata` tags (`mExtraFlag0`-`2`, no reader), `mInstanceFile`,
  the number tags `mReserved` and `mEnabled`, `GameObjectIdMetadata`'s
  `mEntity`, `mAllowedObjects`, `mAllowedEntities`, `mRequiredInterfaces`
  and `mFilterClass`, `ResourcePathMetadata`'s `mDefaultExtension` and
  flags, `PropRefMetadata`'s `mArrayIndex`, `mAllowedComs`,
  `mAllowedProps` and `mAllowsArrays`, `ArrayMetadata`'s hooks and
  `mFixedSize`, and `IntMetadata::mBitfield` (which `Dump` writes as
  "prefix").
- `PropTypeSizes`, `PropTypeName`, `InitComponentClass`,
  `RegisterPropStruct` and `RevisionArgs`' fields are inferred.
  `LoadWaveformResources` (`0x1C9A90`), which `LoadResources` calls, takes
  the map's name: it lies in `entity/Waveform.o`'s range and visits the
  waveform property types 18 and 19.
