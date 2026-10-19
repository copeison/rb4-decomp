# Profiles and system options

The saved data of the map's `persistence` module: the dirty-tracked data
base classes, the profile, the profile manager and the system options.
Names follow [naming.md](naming.md); saving and loading are in
[persistence-saveload.md](persistence-saveload.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `persistence/Profile.o` | `0x11B1AF0`-`0x11B1FD7`, `0xD59460`, `0xD59480` | `src/persistence/profiles/Profile.cpp` |
| `persistence/ProfileMgr.o` | `0x3B28F0`-`0x3B4496` | `src/persistence/profiles/ProfileMgr.cpp` |
| `persistence/SystemOptions.o` | `0x11B1FE0`-`0x11B2C73` | `src/persistence/options/SystemOptions.cpp` |
| `IDirty`, `PersistentData` (header-only) | `0x9D4110`-`0x9D4130` | `src/persistence/core/PersistentData.h` |

`Profile` and `SystemOptions` live in the second persistence block, where
the linker placed their objects; `Profile::IsDirty` and `GetSaveVersion`,
inline in the map's build, are folded into identical game copies.

## Dirty-tracked data

`IDirty` has the dirty flag (+8) and the `MakeDirty`, `MakeClean` and
`IsDirty` slots; `PersistentData` adds the parent (+16, which
`MakeDirty` marks) and the pure `Save(BinStream&) const` and
`Load(BinStream&)`. Their members are inline; the game code holds the
binary's copies.

## Profile

A `Profile` (200 bytes, vtable `0x19A59B0`, `MsgSink` at offset 24) is a
pad's saved game data; the game derives its profile from it. It keeps its
own dirty flag (+32): `MakeDirty` sets it and counts the change (+36, from
-1) while dirtying is enabled (+33), `MakeClean` resets both, and the
destructor disables dirtying. The slots after `PersistentData`'s are
`Handle` ("delete_all" calls `DeleteAll`, which marks it dirty), `Poll`,
`CanSave` (false), `DeleteAll`, `EnableDirtying`, `GetSaveFileName`
("game.save") and `GetSaveVersion` (0). The profile carries its
`StorageRef_PS4` (+48), the pad (+192) and the flag that lets
`SaveLoadMgr::ExecuteSave` write it (+40), set when no save exists.

## Profile manager

`ProfileMgr` (344 bytes, vtable `0x18FD270`) is a `SyncMsgSource` locked by
a file-static `CritSec` (`0x1A6E2E0`). The game's `RBProfileMgr` derives
from it (`0xD5B320` calls the constructor) and implements the pure slots:
`GetSystemOptions` (7), `GetSystemSaveFileName` (8) and slot 14, which
creates and loads a profile for a pad that signed in. Slots 9-12 are the
map's empty `_OnProfileAdded`, `_OnProfileAboutToBeRemoved`,
`_OnProfileFailedToLoad` and `_OnProfileFailedToSave`; slot 13 returns
true and slot 15 is empty.

The constructor subscribes to the platform's "user_signed_in",
"user_signed_out", "app_suspend_state_changed" and
"platform_identity_changed" and reserves 64 profiles. `Init` sets the
"profile_mgr" global; `PostInit` (a static that tail-calls the member)
subscribes, when slot 13 allows, to the save manager's "load_end",
"save_end" and "saveopt_end", the last as "on_system_options_loaded".
`Handle`:

- a signed-in user calls slot 14 with the pad;
- a signed-out user's profile is destroyed;
- on resuming, the profiles of users no longer signed in
  (`PlatformMgr::IsSignedIn`) are destroyed, then the platform's slot 14
  and the manager's slot 15 run;
- a changed identity destroys every profile;
- "load_end" reports `kUnknownError` and `kDeviceFull` as a failed load
  and exports "profile_loaded"; "save_end" reports `kUnknownError` as a
  failed save;
- "on_system_options_loaded" goes to the system options.

`DestroyProfile` removes the profile, calls the hook, exports
"profile_about_to_be_removed" with the profile's `MsgSink` and deletes it.
The message classes are inline in `os/platform/PlatformMgr.h`; the binary
emits `AppSuspendStateChangedMsg`'s and `PlatformIdentityChangedMsg`'s
array constructors with this object.

## System options

`SystemOptions` (216 bytes, from the small-block pool, vtable `0x19A5A60`,
`MsgSource` at offset 24) holds the overscan (1.0), the audio and video
offsets (-110 and -75 ms, limited to 300 either way), the dialog volume
(70), the gamma (0.5), the loaded flag and whether the player calibrated,
then its `StorageRef_PS4` (+72). The constructor sets the values and calls
`Reset`, which forces the defaults. Each setter marks the options dirty
when the value moves by more than 0.0001; `MakeDirty` exports
"system_options_dirty". `Save` writes the five floats and the flag in a
`RevisionStream` at revision 5. `Load` reads each value through a
`Revision::AsOfOp` (floats from revision 1, the flag from 2), restores the
default offsets for uncalibrated saves and for revisions 2 and older
(which also clear the flag), skips the two strings of revision 4 and marks
the options loaded even when the block cannot be read. `Handle` sets the
loaded flag on "on_system_options_loaded". `SetOverscan` is not emitted;
`Load` and `Reset` inline it.

## Weak evidence

- `PersistentData::mParent`, `Profile::mDirtyCount`, `mReadyToSave` and the
  system options' field names (taken from the map's setters, in the map's
  order) are inferred.
- `ProfileMgr`'s slots 13-15 (`_UsesSaveLoadMgr`, `_OnUserLoggedIn`,
  `_OnAppResumed`) are named from their bodies and the game's overrides;
  `_OnUserLoggedIn` is the override's map name.
- `PlatformIdentityChangedMsg` is newer than the map and named after its
  event.
- The static initializer at `0x3B4400` also sets a shared header's global
  (`0x1A6E2D8`) to -1; it is not modelled.
