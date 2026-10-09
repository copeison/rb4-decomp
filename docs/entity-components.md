# Components and properties

The component base, the component class registration and the property
system of the map's `entity` module. Names follow [naming.md](naming.md);
the entity objects that hold the components are in
[entity-resources.md](entity-resources.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `entity/ComMetaData.o` (part) | `0xE56C0`-`0xE6870` | `src/entity/core/ComMetaData.cpp` |
| `entity/Component.o` (part) | `0xE7190`-`0xE9060`, `0xAEA0`, `0xAED0` | `src/entity/core/Component.cpp` |
| `entity/EditorCom.o` (part) | `0xEABC0`-`0xEABD0` | `src/entity/core/EditorCom.cpp` |
| `entity/PropInfo.o` | `0x12DBE0`-`0x12DD00` | `src/entity/props/PropInfo.cpp` |
| `entity/PropRegistry.o` (part) | `0x1804C0`-`0x1813D0` | `src/entity/props/PropRegistry.cpp` |
| `PropArrayBase` | `0xA660`, `0x2ABE0` | `src/entity/props/PropUtl.cpp` |

`PropPath`, `PropMetadata` and the `PropUtl.o` helpers are declared in
`src/entity/props`; their bodies are not reconstructed.

## Component

`Component` is 24 bytes: the vtable, the owning `GameObject`, and seven
flags (imprinted, props synced, resources requested, resources ready,
entered, a flag nothing reads, enabled). Subclasses place their first
member in the tail padding at 23, as `RndOverlayOptionsCom` does.

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
| 24-30 | `_PostLoadStorage`, `_PreLoadResources`, `_PostLoadResources`, `_GetPollDeps`, `_CanHaveParentPollDep`, `_OnResourcesLoaded`, `_AreResourcesReady` |
| 31-35 | game mode `_Enter`, `_Exit`, `_Poll`; `_OnDeactivate`, `_OnActivate` |
| 36-40 | edit mode `_EditEnter`, `_EditExit`, `_EditPoll`; `_HasPostPoll`, `_PostPoll` |

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
(`0xE5B80`, not reconstructed) fills the dependency lists of the classes it
names and adds the class to `sComMetaDataList` (`0x19E28B8`).

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

## Weak evidence

- The slot names 3, 5, 6, 9, 12-14, 17, 18, 24-26 and 28-30 rest on their
  bodies and callers. `_CanHaveParentPollDep` and the resource-load pair are
  map names of `InstanceCom` methods.
- `ComMetaData::mLightweight`, `mEditorRestrictions` and `mClassName`, and
  `Component::mPropsSynced`, are named from their few readers.
- `GroupMetadata` as the struct metadata class, and `LoadPropResources`
  (`0x1C9A90`), are inferred.
