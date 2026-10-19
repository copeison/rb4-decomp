# Save manager and storage clients

The save and load flow of the map's `persistence` module: the save manager
scripts talk to and the two clients that serialize a profile or the system
options and move the bytes through a `StorageRef_PS4`
([persistence-storage.md](persistence-storage.md)). Names follow
[naming.md](naming.md); the saved objects are in
[persistence-profiles.md](persistence-profiles.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `persistence/SaveLoadMgr.o` | `0x3ABB80`-`0x3AEED6` | `src/persistence/saveload/SaveLoadMgr.cpp` |
| `persistence/ProfileStorageClient.o` | `0x3AEEE0`-`0x3AFBD3` | `src/persistence/saveload/ProfileStorageClient.cpp` |
| `persistence/SystemOptionsStorageClient.o` | `0x3AFBE0`-`0x3B0A93` | `src/persistence/saveload/SystemOptionsStorageClient.cpp` |

[progress.md](progress.md) starts the range at `0x3AB600`; the code from
there to `0x3ABB7F` belongs to the memory tracker before it.

## Results and main-thread calls

`SaveLoad::Error` names the results: `kNoError`, `kNoActiveProfile`,
`kNoDevice`, `kMultipleDevices`, `kNoSaveNeeded`, `kIncompatibleVersion`,
`kDeviceFull` and `kUnknownError` (15), from `SaveLoad::ErrorToString`'s
table (`0x18FCE60`). A storage write result maps -4 to `kDeviceFull`, -3 to
`kNoDevice`, -2 to `kNoActiveProfile`, -1 to `kUnknownError` and anything
else to `kNoError` (a switch table per client, `0x127B2EC` and
`0x127B330`).

Every result reaches its receiver on the main thread through
`Hmx::CallMemberFunctionOnMain` (`os/threading/CallOnMainThread.h`): on the
main thread, or before it is known, the member is called at once;
otherwise a `MainFunctionCaller<T, P>` from the small-block pool
("MainFunctionCallerParam") carries the object, the member pointer and a
copy of the argument, calls it from `CallOnMainThreadPoll` and deletes
itself. The template is inlined everywhere; each object emits its
instantiation's destructors and `OnMainThread`.

## Profile client

A `ProfileStorageClient` (120 bytes, vtable `0x18FCEF0`) serializes its
profile into a `MemStream` scratch buffer inside a `RevisionStream` at the
profile's `GetSaveVersion`, encrypted with `BinStream`'s write cipher.
`Save` refuses a clean profile unless forced (`kNoSaveNeeded`), a pad that
is not signed in (`kNoActiveProfile`) and an invalid or busy store, then
counts the save and calls `_PerformSave` on the main thread. That pushes
the "profile" heap (a function-local cache of `MemTryFindHeap`), saves into
the scratch buffer, marks the profile clean and writes the bytes to the
store. The write result goes to the callback's `OnProfileSaveResult`.

`Load(true)` counts a find and asks `StorageMgr::Find` for the pad's save
file; `OnFindResult` reads the single file found into the scratch buffer,
reports `kNoDevice` when there is none (and marks the profile ready to
save, so a new profile can be written), `kMultipleDevices` for several,
`kDeviceFull` when the directory could not be created and `kUnknownError`
otherwise. `Load(false)` reads the store directly. The read result counts
a pending read and runs `_PerformLoad` on the main thread, which loads the
profile under the "profile" heap; a block that is too old or has no header
keeps the defaults and counts as loaded, a newer one is
`kIncompatibleVersion`. `Restore` reloads the last loaded save over a
dirty profile with dirtying disabled. `IsBusy` is a busy store or any
pending find, save or read.

Two trailing members have no caller: `_SaveProfile` (save and write, the
body of `_PerformSave`) and `_LoadProfile`, a tail call to
`_LoadProfileFromScratch`. `SystemOptionsStorageClient` has the same pair.

## System options client

`SystemOptionsStorageClient` (104 bytes, vtable `0x18FCF68`) does the same
for `ProfileMgr::GetSystemOptions` at revision 0, without encryption or
pending counters. Its store is the options' own `StorageRef_PS4`, named
after `GetSystemSaveFileName` with the localized "system_options_name"
(cached in a static) whenever it is idle. `Save` gives an invalid store the
profile manager's best device first. Its find, read, write and delete
results map as the profile client's do; a failed find is
`kUnknownError`.

## Save manager

`SaveLoadMgr::Init` creates `theSaveLoadMgr` (`0x1A6E0A8`, 464 bytes) and
sets the "saveload_mgr" global. It is a `SyncMsgSource` locked by its own
`CritSec` and implements `StorageCallback` (offset 32),
`ProfileStorageClientCallback` (40) and
`SystemOptionsStorageClientCallback` (48); the vtable at `0x18FCC80` adds
the device prompt and the six client results after `MsgSource`'s slots. The
constructor reads "viper_autosave" "enable" from the system config into the
autosave flag, reserves 64 queued saves and loads, and subscribes to the
platform's "storage_changed" (as "on_storage_changed") and the profile
manager's "profile_about_to_be_removed" (as
"on_profile_about_to_be_removed").

Status messages are `(type error pad)` exports, built on the main thread by
`ExportMsg` from a `MsgParams`; `ExportStatus` posts one. The pad is -1 for
the system options:

| Operation | Messages | Notes |
| --- | --- | --- |
| `Save`, `ExecuteSave`, `SaveAll` | `save_start`, `save_end` | Without autosave only a forced save runs; a profile not ready to save ends with `kNoSaveNeeded` |
| `Load` | `load_start`, `load_end` | |
| `DeleteSave` | `delete_start`, `delete_end` | |
| `SaveSystemOptions` | `saveopt_start`, `saveopt_end` | |
| `AutoloadSystemOptions` | `loadopt_start`, `loadopt_end` | |
| `ChooseDevice` | `choosedevice_start`, `choosedevice_end` | |
| device prompt | `confirm_overwrite` | |

A start is exported when an operation began; a failure also exports the
end. `kNoSaveNeeded` from a client exports nothing. While the profile
client is busy, `Save` and `Load` queue the profile once; `Poll` runs the
last queued load, else the last queued save, once both clients are idle.
`SaveSync` saves with a client and a recording callback of its own
(`0x18FCE10`) and spins until the write result arrives.

The device prompts pass their purpose as the context: choosing a device
(0), a new profile save (1), saving the options (2) and loading them (3).
`OnDevicePromptResult` exports `choosedevice_end` with `kNoDevice` for a
failed prompt, sets the client's device, and for a save over an existing
file keeps the device and exports `confirm_overwrite` for
`ConfirmSaveNewDevice` or `ConfirmOptionsOverwrite`. On the PS4
`PromptForDevice` never answers. `Handle` takes
"on_profile_about_to_be_removed", which forgets the profile in the client
and both queues, and "save_all". `DumpFiles` lists the pad's files through
a static `FileDumper`.

## Weak evidence

- `SaveLoadMgr::mPendingMsg` (an empty symbol nothing reads), the
  `DevicePrompt` names, `SaveSyncCallback`, the clients' counters and the
  four trailing helpers (`_SaveProfile`, `_LoadProfile`, `SaveOptions`,
  `LoadOptions`) are inferred.
- `SaveLoadMgr`'s destructor leaves the subscriptions in its body; the
  binary does so as each is destroyed, after the lock.
- The binary's `_PerformSave` and `_PerformLoad` open with a thread query
  whose result is unused, kept as such.
