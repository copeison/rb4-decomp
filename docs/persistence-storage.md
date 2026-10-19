# Storage devices and save files

The PS4 save data layer of the map's `persistence` module: the storage
manager, the save file reference and the callback interface the save
clients implement. Names follow [naming.md](naming.md); the clients and the
save manager are in [persistence-saveload.md](persistence-saveload.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `persistence/StorageMgr.o` | `0x11B2C80`-`0x11B2CCE` | `src/persistence/storage/StorageMgr.cpp` |
| `persistence/StorageMgr_PS4.o` | `0x3B0AA0`-`0x3B1137` | `src/persistence/storage/StorageMgr_PS4.cpp` |
| `persistence/StorageRef_PS4.o` | `0x3B1140`-`0x3B28E0` | `src/persistence/storage/StorageRef_PS4.cpp` |

`StorageCallback`, `StorageRef` and `StorageMgr` are header-only apart from
`StorageMgr::Init` and `StorageRef::ResultToString`, which sit in the second
persistence block with `Profile` and `SystemOptions`
([persistence-profiles.md](persistence-profiles.md)). The map's
`StorageMgr::IsFakingSaveFailures` and `ToggleFakingSaveFailures` were not
found in this binary.

## Storage manager

`StorageMgr::Init` creates `theStorageMgr` (`0x1A6E2B0`), a
`StorageMgr_PS4` (40 bytes, vtable `0x18FCFE0`). Its constructor initializes
the user service and the save data library (`sceSaveDataInitialize3`) and
sizes a vector of 16 mount point flags, which `AquireMountPoint` (the map's
spelling) and `ReleaseMountPoint` take and free under a file-static
`CritSec` (`0x1A6E2B8`).

The five slots are the destructors, `Find`, `PromptForDevice` (empty: the
PS4 has one device and never answers) and `DeleteContainer`, an empty
inline body in this build (`0x3B1090`; the map's had a static
`sDisabled`). `Find` builds a `StorageRef_PS4` for the user and name, titles
it with the localized "save_file_name", and reports it through
`OnFindResult` when its save directory can be created and the file is not
empty; the result is `kResultAccessDenied` when the directory cannot be
created. It then deletes the listed references (a reference that was not
listed leaks, as in the binary). The device and prompt arguments are
unused, as is `FindInDirectory`, which returns 0.

The results are `StorageRef::Result`: `kResultSuccess`,
`kResultGeneralError` (-1), `kResultAccessDenied`, `kResultInvalidDevice`
and `kResultDeviceFull` (-4), named by `ResultToString`'s table at
`0x19A5AF0`. `StorageRef::kAnyUser` and `kAnyDevice` are both -1
(`0x127B374`, `0x127B378`); their users read them from memory.

## Callbacks

`StorageCallback`'s slots are empty inline defaults emitted with
`SaveLoadMgr` (`0x3AEDA0`-`0x3AEDE0`, `0x3AEE20`): `OnFindResult` (2),
`OnDevicePromptResult` (3), `OnReadResult` (4, with the file size on
success), `OnWriteResult` (5), `OnDeleteResult` (6) and a seventh slot with
no override or caller, named `OnDeleteContainerResult` here. Slot 3 sits
before the read and write results: `SaveLoadMgr` overrides it and its
secondary vtable keeps the base's find result in slot 2.

## Save file references

A `StorageRef_PS4` (144 bytes, vtable `0x18FD018`, `ThreadCallback` at
offset 8 with `0x18FD0B0`) names a file inside the title's save directory,
"slot017" (`snprintf("%s%03d", "slot", 17)` into 17 bytes). It has a state
(idle, reading, writing), the owner, the name, the display title, the path,
a cached size, the pending operation's context, callback, buffer and size,
the mount point index and the mount point. Its slots follow the map's
virtuals: `Name`, `SetName`, `Size`, `Device` (-1), `SetDevice` (ignored),
`Owner`, `SetOwner`, `Write`, `Read`, `Delete`, `Exists`, `IsValid` (an
owner and a name), `IsBusy` (not idle), then `ThreadStart` and `ThreadDone`.
The copy constructor copies the owner, size and names; `operator=` copies
the owner, size, the mount flag and the name, and clears the title.

Every file operation runs inside `_WithMountedSaveDir` (inferred name; an
unreferenced out-of-line copy is at `0x3B1640`), which takes a
`std::function<void()>`:

- `InitMount` creates the directory with 1280 blocks in mode
  `RDWR|CREATE|COPY_ICON` and, when it is new, sets its title from the
  display name (`_SetSaveDataParams`, inlined) and unmounts it. An existing
  or busy directory succeeds. Broken data and a full disk show
  `PlatformMgr::ShowSaveDataDialog` (error types 6 and 4, with the required
  blocks) through `PlatformMgr_PS4.o`'s copy of the manager pointer
  (`0x19FE988`) and fail. The flag at +80 would skip it; nothing sets it.
- `_MountSaveDir` fails when `ContentMgr::AreAllMountsGoingToFailDueToCheat`
  (inlined in the binary as the flag at +0x30 of `TheContentMgr`) is set.
  Otherwise it takes a mount point, sets the mount point to "savedataN:"
  and the path to "savedataN:/<name>", and mounts the directory with 320
  blocks read-only for reads and read-write otherwise.
- `_UnmountSaveDir` unmounts "/savedataN", frees the mount point and clears
  both strings.

`Read` and `Write` store the request and queue the reference on the
ThreadCall thread; `ThreadStart` runs `_ThreadRead` (a `FileStream` read
of the buffer) or `_ThreadWrite` (creates the mount point directory and
writes with file mode 5, then flushes), each `kResultGeneralError` unless
the stream succeeded. `ThreadDone` reports `OnReadResult` or
`OnWriteResult` and goes idle. `Size` measures the file once
(`_DetermineSize`), `Exists` checks it with `FileExists` (mode 2), and
`Delete` deletes it at once in write mode without calling the callback.
The lambdas' `std::function` vtables are at `0x18FD0E0`-`0x18FD220`.

## Weak evidence

- `StorageCallback::OnDeleteContainerResult`, the `State` enumerators,
  `mMountInitialized`, `_WithMountedSaveDir` and the file-static lock are
  inferred.
- `PlatformMgr`'s slots 10 and 11 take the map's `GetDefaultUser` and
  `GetAnyUser` from their bodies; slot 14, empty on the PS4, is named
  `RefreshSignedInUsers` from its one caller.
- `Localize` takes a third argument its body does not read; it is declared
  as an `int`.
- The object's static initializer (`0x3B10A0`) also sets a shared header's
  global (`0x1A6E2A8`) to -1; it is not modelled.
