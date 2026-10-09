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
| `entity/EntityResource.o` (part) | `0xFD5D0`-`0x102708` | `src/entity/core/EntityResource.cpp` |
| load-progress listeners (not in the map) | `0x12D640`-`0x12DBD6` | `src/entity/progress/LoadProgress.cpp` |
| `entity/TransEntityResource.o` | `0x1BAAA0`-`0x1BB9F2` | `src/entity/core/TransEntityResource.cpp` |
| `entity/Entity.o` (part) | `0xEBB70`-`0xF7960` | `src/entity/core/Entity.cpp` |
| `entity/GameObject.o` (part) | `0x115F60`-`0x117A40` | `src/entity/core/GameObject.cpp` |
| `entity/TransCom.o` (part) | `0x127730`, `0x1B3A90` | `src/entity/core/TransCom.cpp` |
| `entity/EntityConstants.o` | data only | `src/entity/core/EntityConstants.cpp` |
| `utl/DataUtl.o` (part) | `0x23C2E0`, `0x23F780` | `src/utl/data/DataUtl.cpp` |
| `utl/FileUtl.o` (part) | `0x244860`-`0x245866` | `src/utl/files/FileUtl.cpp` |
| `utl/PollMgr.o` (part) | thread-local data | `src/utl/threading/PollMgr.cpp` |
| `utl/BinStream.o` (part) | `0x219F10` | `src/utl/streams/BinStream.cpp` |

The component base and the property system are in
[entity-components.md](entity-components.md).

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

`_LoadRoot` (slot 32) passes the layer paths before the root data, unlike
the map's signature. `Entity::_LoadRoot` reads the root header from
revisions 12 to `kEntityResourceRev` (18), the entity revision (from
`kEntityRev`, 33), the layer count and the root object.
`TransEntityResource` reads the root data of revisions 12 and 13 itself
and, before revision 12, copies it from the root's `InstanceCom` icon data.
Its `_LoadEntity` resets the root's transform unless the instance
component drives the parent, and `_PostLoad` takes the instance component's
class from the first root component whose class serves as one.

`Load` (slot 4) reads the entity through `_LoadEntity` (slot 14), loads its
resources and calls `_PostLoad`; a source file has its stale ids fixed
around the resource load (`_FixupStaleIds`, declared). `_LoadEntity` drops
the old entity and its inlined resources, reads the layer files (revisions
12 to 18) and the root data (from 16), and then the entity. A source file
before revision 17 gains the root's `EditorCom` (before 13) and
`InstanceCom`. A cached file fails when a layer file is missing or not
older than the resource, and then reads each layer's inlined resources
(`_LoadInlineResources`, declared). `Save` writes revision 18, the layer
files, the root data (empty in a cached file while precaching), the load
step count and the entity; a cached file adds each layer's inlined
resources and loads the entity's resources. While either runs,
`gEntityLoadDir` (`0x19B02E0`) holds the file's folder. Slot 31 asks for
the per-entity perf timers when it returns true, so its name is now
`_WantsPerfTimers`.

The load-progress listeners are an object of their own between PollSort
and PropInfo, which the map does not have; all their names are inferred.
`LoadProgressListener` has five pure slots (begin, set, step, end and a
message); the list (`0x19E3CC0`) and its `CritSec` (`0x19E3CE0`) are
global, and each report stops at the first listener that returns true.
Nothing registers a listener in this build. The list's size is the
"profiling" check: `IsProfilingLoad` is true while listeners exist and the
thread's `gProfilingLoad` (TLS `0x19B02E8`) is the resource, which
`_BeginLoadProgress` sets when the path is `sProfileLoadPath`
(`SetProfileLoadPath`, `0x19E3000`).

## Entities and objects

`Entity` keeps its layers in a `PropArray<Entity::Layer>` at +168, its
poll and post-poll orders as `PropArray<GameObjectId>`s, its owning
resource at +288 and its state flags at +300. `GameObject` (128 bytes)
keeps its components in a `PropArray<GameObject::ComIndex>` and their poll
order as indices.

`Entity` derives from `MsgSource` (at 0) and `PollDepBase` (at 24); see
[messages-and-polls.md](messages-and-polls.md). Its vtable `0x18E6818` has
twelve slots: `MsgSource`'s seven, then `ThreadPoll`, `PostPoll`,
`IsPollEnabled`, `_OnAddPollDep` and `_OnRemovePollDep`, whose thunks fill
the `PollDepBase` vtable `0x18E6888`. Both match the emitted vtables slot
for slot. `Handle` (slot 2, `0xF6DB0`) remains declared.

`Enter` sets the mode (game, or edit with flag bit 0), enters the objects
in poll order unless bit 1 is set, with the entity as the default script
entity, and in the game mode sets the poll weight to a tenth of the
component count (`GetNumComs`, declared) and flag `0x8000` when the root's
`InstanceCom` drives the parent. `IsPollEnabled` then follows such
entities up to their parents. `Exit` and `_Destroy` run under the
thread-local destroy type (`TLSValue<DestroyType>` at `0x19B02B8`, starting
at -1): a top-level entity uses `kDestroyRoot` (0), an instanced one 1 or 2
by its editor flag, and nested entities keep the lowest. `Exit` leaves the
post-poll queue and exits the objects in reverse poll order, then the
inactive objects still entered. `_Poll` polls the poll window in the game
mode (`GameObject::_PollComponents`, which releases the object's
early-free job) and every object in the edit mode. `_Destroy` drops the
event sinks, destroys every object and then the root, leaves the poll
manager's resource-load queue and frees the entity unless it was built in
place.

`Entity::_LoadResources` loads every object's components in poll order,
restarting an object whose components a load added to, then waits up to
ten passes for them to report ready. `_CreateAndInsertNewGameObject` gives
the object the layer's next serial and first free index. `SafeGetObject`
rejects stale ids.

`GameObject::CreateComponent` wraps `_CreateComponent`, the map's
`ObjPtr::CreateComponent`. It re-sorts the components after an addition
unless the caller defers the sort. `GetDataCom` looks the command's object
up in `gDataThread.mDefaultEntity` and its component by class or base
class.

`GameObject::_SortComponents` orders the table by the classes' component
dependencies and rebuilds the poll order; the ordering itself (`0x117A90`)
is declared. `DestroyComponent` exits the component in the entity's mode,
lets the resource destroy its dependants, removes it and clears the
entity's references to it.

## Weak evidence

- `ResourcePathRecorder` (`0x1AEB20`-`0x1AEC26`), `ResourceFileIsLocal` and
  `CopyToCache` have no callers. Their names rest on their bodies and their
  place in the object.
- The EntityResource slot names `_PostLoad`, `_OnEntityReady`,
  `_DestroyLayerEntity`, `IsTransient`, `_IsEditorEntity` and
  `_WantsPerfTimers` come from their bodies and callers only.
- Every load-progress name, and the enumerator names `kDestroyRoot`,
  `kDestroyEditorInstance` and `kDestroyInstance`, are inferred.
  `LoadProgressMessage`'s argument types rest on its register use alone.
- `Entity::mPollWindowStart` and `mPollWindowEnd` (+296) and
  `GameObject::mEarlyFreeDep` (+112) are named from their readers; their
  writers are not identified.
- `GameObject::_EnterComponents` (`0xF4FF0`) and `_PollComponents`
  (`0xF6290`) are emitted in `Entity.o`; their names are inferred.
- `GameObject::mImprinted` and `Debug::mDisabled` are named from their
  writers only.
- The global `0x19E3280` is named `gEntityInstanceComId`. It is the class id
  of the component registered at `0x118E60`, whose class name is not
  recovered.
- `InstanceCom::mPollRequested` (+520) has no identified writer.
