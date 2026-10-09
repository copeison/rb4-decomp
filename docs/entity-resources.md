# Resources and the entity core

The engine's resource system and the parts of its entity module that the
rest of the reconstruction calls. Names follow [naming.md](naming.md); the
map puts all of this in its `entity` module.

## Files

| Object | Range | Source |
| --- | --- | --- |
| `entity/Resource.o` | `0x1AB390`-`0x1AF10A` | `src/entity/resources/Resource.cpp` |
| `entity/ResourceMetaData.o` | `0x1AF110`-`0x1AF940` | `src/entity/resources/ResourceMetaData.cpp` |
| `ResourcePath` methods | `0x1AF950`-`0x1AFFAB` | `src/entity/resources/ResourcePath.cpp` |
| `entity/EntityResource.o` (part) | `0xFD610`-`0x102708` | `src/entity/core/EntityResource.cpp` |
| `entity/TransEntityResource.o` (part) | `0x1BAAA0`-`0x1BB9F2` | `src/entity/core/TransEntityResource.cpp` |
| `entity/Entity.o` (part) | `0xEF180`-`0xF3060` | `src/entity/core/Entity.cpp` |
| `entity/GameObject.o` (part) | `0x115FC0`-`0x117752` | `src/entity/core/GameObject.cpp` |
| `entity/TransCom.o` (part) | `0x127730`, `0x1B3A90` | `src/entity/core/TransCom.cpp` |
| `entity/EntityConstants.o` | data only | `src/entity/core/EntityConstants.cpp` |
| `utl/DataUtl.o` (part) | `0x23C2E0` | `src/utl/data/DataUtl.cpp` |
| `utl/FileUtl.o` (part) | `0x244860`-`0x245866` | `src/utl/files/FileUtl.cpp` |
| `utl/PollMgr.o` (part) | thread-local data | `src/utl/threading/PollMgr.cpp` |

The map has the `ResourcePath` methods in `Resource.o`. This build places
them after `ResourceMetaData.o`'s static initializer, in an object the map
does not name. `GfxApiSymbol` sits in `Resource.o`'s range and moved there
from `PlatformMgr.cpp`.

## Resource

Every resource with a path is in `Resource::sResources`, a map from path to
resource. The `CritSec` at `0x19E4560` guards the map and every reference
count. The `CritSec` at `0x19E4570` is held while resources load.
`ReleaseRef` deletes a resource with its last reference, but does nothing
once `TheDebug.mExiting` is set.

`GetOrLoad` returns a loaded resource, unless it was marked by
`FileChangedOnDisk` or its slot-12 `NeedsReload` asks to be reloaded. A
load goes through `LoadUnique`:

- A path with an extension and no `::` sub-object is created as the class
  registered for its extension (`ResourceMetaData::sExtToId`). That class
  must be the requested class or derive from it.
- Any other path is created as the requested class.

`New` uses the class's factory in `sFactory`. When the class has none, it
uses the first factory whose class derives from it. A reload that fails
keeps the old resource. Any other result replaces the map entry.

`LoadFile` (slot 3) reads the cached file that `CheckCache` finds under
`../build/data` (or `../build/data_precached` while precaching). The cached
file name gains the platform (`_ps4`) unless the type's metadata sets
`mTypeFlags[6]`; shader sources get the platform and the graphics API. A
missing or stale cache is rebuilt from the source through `Load(stream,
false)` and `Save(stream, true)` into `<cache>_temp`, which then replaces
the cache. Precached mode (`gFileArchiveMode`, the map's
`Resource::sPrecached`) trusts the caches and never rebuilds them.

`_SetFileAndLoad` stores the bytes `LoadFile` allocates (`MemGetStats`) in
`mLoadSize`. The thread-local `Resource::LoadContext` names the component
whose resources the thread is loading, and a failed load is reported
against it.

## Entity resources

`EntityResource` holds an `Entity` and its layer files, and keeps the
resources inlined into each layer. An inlined resource has `mInlined` set.
The vtable has 33 slots, checked against `0x18E6AF8` and `0x18E9B68` with
`orbis-objdump -r`. Slots 13-32 are the entity's. The map names
`CreateEntity`, the enter/poll virtuals, `CreateRequiredComponents`,
`DestroyDependentComponents`, `InitObject`, `GetInstanceComId` and
`_LoadRoot`; the other names are inferred. The non-virtual `EnterEntity`,
`ExitEntity` and `PollEntity` check for a null entity before calling slots
16-18.

`TransEntityResource` is the code-created entity resource. Its
`InitObject` adds a `TransCom` to every object. A resource with
`mSuppressPoll` set enters its entity with the thread's
`ThreadPollContext::mEnterImmediately` cleared. `RndSceneResource` derives
from it through an unmodelled intermediate class.

The loaders (`Load`, `_LoadEntity`, `_LoadRoot` and `Save`) are declared but
not reconstructed. They need the property system (`PropRegistry`,
`PropPath`, `ComMetaData`) and the `Component` vtable, neither of which is
modelled yet.

## Entities and objects

`Entity` keeps its layers in a `PropArray<Entity::Layer>` at +168, its
owning resource at +288 and its state flags at +300. `GameObject`
(128 bytes) keeps its components in a `PropArray<GameObject::ComIndex>`.
`PropArrayBase` declares the element-operation slots: `CreateObject` grows
the component table through slot 6, the element move.

`GameObject::CreateComponent` wraps `_CreateComponent`, the map's
`ObjPtr::CreateComponent`. It re-sorts the components after an addition
unless the caller defers the sort. `GetDataCom` looks the command's object
up in `gDataThread.mDefaultEntity` and its component by class or base
class.

`Component` is still a byte layout. Its 41-slot vtable (`0x18E6518`) is not
modelled, so `Component::MakeErrorName` and the other component methods
remain declarations.

## Weak evidence

- `ResourcePathRecorder` (`0x1AEB20`-`0x1AEC26`), `ResourceFileIsLocal` and
  `CopyToCache` have no callers. Their names rest on their bodies and their
  place in the object.
- The EntityResource slot names `_PostLoad`, `_OnEntityReady`,
  `_DestroyLayerEntity`, `IsTransient`, `_IsEditorEntity` and
  `_SkipPerfTimers` come from their bodies and callers only.
- `GameObject::mImprinted` and `Debug::mDisabled` are named from their
  writers only.
- The global `0x19E3280` is named `gEntityInstanceComId`. It is the class id
  of the component registered at `0x118E60`, whose class name is not
  recovered.
