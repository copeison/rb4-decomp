# Users and the game's shared definitions

The engine's users (map module `os`) and the first `rb_game` object. Names
follow [naming.md](naming.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `os/HxGuid.o` | `0x246960`-`0x247110` | `src/os/users/HxGuid.cpp` |
| `os/UserGuid.o` | `0x11B15D0` | `src/os/users/UserGuid.cpp` |
| `os/User.o` | `0x3A8050`-`0x3A8420` | `src/os/users/User.cpp` |
| `os/UserMgr.o` | `0x36AD30`-`0x36B865` | `src/os/users/UserMgr.cpp` |
| `rb_game/Defines.o` (part) | `0xA43F20`-`0xA459E8` | `src/rb_game/core/Defines.cpp` |

## HxGuid and UserGuid

`HxGuid` is four signed 32-bit chunks. `Generate` hashes the date
(`GetDateAndTime`, `0x242700`, which returns a 6-byte `DateTime` in this
build), the next `gRand` value and the cycle counter with `CSHA1`, and
keeps 16 bytes of the digest. `operator=(String)` parses eight digits per
chunk and counts a character as a letter only above `'A'`. `NULL_GUID`'s
static is at `0x19E7C88`. `UserGuid` adds only its `operator=(String)`.

## User

32 bytes: the vtable (`0x18FC990`, eight slots), the `UserGuid` (+8), the
platform user id (+24, -1 when none) and a flag set by the first
`SetPlatformUserId` (+28). The constructor generates the id.

| Slot | Member |
| ---: | --- |
| 0-1 | destructors (`0x3A83D0`, `0x3A83E0`) |
| 2 | `Handle`: "get_player_name", "comes_before" |
| 3 | `MsgSink::GetSinkObject` |
| 4 | `GetPlatformUserId` (the map's `BandUser` method, moved up) |
| 5 | `IsJoypadConnected` |
| 6 | `_OnProfileLoaded` (inferred, empty) |
| 7 | `_OnPlatformUserIdChanged` (inferred, empty) |

`UserName` uses `PlatformMgr` slot 21 (`GetName`), or "Player <pad>".

## UserMgr

A `SyncMsgSource` with its own `CritSec` (128 bytes, vtable `0x18FB530`,
ten slots). Slots 7-8 (`GetUsers`, `GetUser`) are pure; slot 9 (`OnLoadEnd`,
inferred) tells the pad's users a profile load ended. The constructor sets
the "user_mgr" global and subscribes to the platform's "user_signed_out";
`PostInit` (inferred, called after `ProfileMgr::PostInit`) subscribes to the
save manager's "load_end". `TheUserMgr` is at `0x19FE748`.

## Band users

| Object | Range | Source |
| --- | --- | --- |
| `rb_game/BandUser.o` | `0xA3ED10`-`0xA40130` | `src/rb_game/users/BandUser.{h,cpp}` |
| `rb_game/BandUserMgr.o` | `0xA40130`-`0xA41BBC` | `src/rb_game/users/BandUserMgr.{h,cpp}` |
| (not in the map) | `0x9A9A00`-`0x9AC060` | `src/rb_game/users/RBBandUser.{h,cpp}` |
| (not in the map) | `0x92AB60`-`0x92C580` | `src/rb_game/users/LocalBandUser.{h,cpp}` |
| (not in the map) | `0x9A48C0`-`0x9A5920` | `src/rb_game/users/RemoteBandUser.{h,cpp}` |
| (not in the map) | `0x923190`-`0x926610` | `src/rb_game/users/RBBandUserMgr.{h,cpp}` |

The chain is `User` -> `BandUser` -> `RBBandUser` -> `LocalBandUser` /
`RemoteBandUser`. The three derived names are inferred (the older game's
`LocalBandUser` and `RemoteBandUser` had the same roles); the evidence is
weak. The intermediate class holds what the map's `BandUser` kept about the
character (`SetCurrentCharName`, `GetPreviewCharName`...).

### BandUser

72 bytes, vtable `0x1968498`, 24 slots. Every function of the object is
reconstructed except the static initializer (`0xA400F0`), which builds
header statics nothing reads.

| Slot | Member | Notes |
| ---: | --- | --- |
| 0-7 | `User`'s | |
| 8 | `GetControllerType` | |
| 9-11 | `IsSompHost`, `IsSompLeader`, `IsSompReady` | pure; session flags, weak names (slot 10 is set by the update that exports "somp_leader_changed") |
| 12 | `IsRemoteJoined` | pure; false for local users, a session flag for remote ones; weak |
| 13 | `GetPlatformName` | pure; profile name or localized "guest" |
| 14-17 | `IsOvershellSlotOpen`, `GetOvershellSlotState` (inline, `0x92B8B0`), `SetOvershellSlotState`, `IsOvershellSlotClosed` | open/closed are weak; closed means `kState_JoinedDefault`, `kState_Idle` or `kState_Disabled` |
| 18 | `_Reset` | pure; called last by `Reset` |
| 19 | `GetGameplayOptions` | pure; returns `IGameplayOptions*` |
| 20 | `GetAchievementData` | pure; a base of the profile's achievement data, weak |
| 21 | `GetProfile(bool any)` | pure |
| 22 | `_ApplyTrackVolumeSettings` | pure; called by `SetTrackType`, weak |
| 23 | `_GetOnlineIDWithoutProfile` | unset id; `GetOnlineID` (`0xA3F520`) prefers the profile's |

Fields: difficulty (+32), track (+36), controller (+40), preferred score
type (+44), two flags the reset clears (+48, named after the map's
`SetHasButtonGuitar`, weak), `RBPlayer*` (+56), overshell slot state (+64),
`mAutoplay` (+68, weak), +69 (unused), difficulty chosen (+70),
participating (+71). The `Handle` messages, `OnSet*` handlers, conversions
and the inline debug-log calls kept by the release build are as in the
binary. `GetActiveScoreType` (`Defines.o`) is now reconstructed on top.

### BandUserMgr

256 bytes, vtable `0x1968568`, 11 slots (slot 10 `_CreateUsers` pure).
Fields: users (+0x80), four slot ids (+0xA0), `BandSlotsReplica*` (+0xE0,
`new(0x1DC8)`, ctor `0xA84AF0`) and `BandUserDataReplica*` (+0xF0,
`new(0x9048)`, ctor `0xA8F1B0`); +0xE8 and +0xF8 are never written. The
replicas are RakNet replicas whose names are inferred and only declared.
`theBandUserMgr` is at `0x1AEF5D8`.

`_GetBandUsers(int)` (`0xA41110`) and `CountBandUsers(int)` (`0xA41770`)
filter on `BandUserFlags`: 4 not participating, 8 participating, 0x10
excludes everyone in this build, 0x800 signed in, 0x1000 logged in
(`PlatformMgr` slot 15), 0x2000 `IsSompLeader`. `GetBandUsers()`
(`0xA40DC0`) passes 8; `GetNumSignedInUsers` inlines `CountBandUsers(0x800)`.
`SetSlot` exports "user_slot_set"/"user_slot_cleared" (slot, user). The
map's `ForEachUser`, `Handle`, FTUE, overshell and debug members are not in
this build. Inferred names: `Init`, `_CreateUsers`, `GetNumUsersInSlots`,
`IsAnyUserSignedIn`, the flag names.

### RBBandUser, LocalBandUser, RemoteBandUser

`RBBandUser` (232 bytes, vtable `0x1962360`, 43 slots; pure 12, 19, 21,
24-26, 29, 41, 42) adds slot 24 `IsLocal` (true for `LocalBandUser`, false
for `RemoteBandUser`) and the preference and session slots 25-42, whose
names follow `RBUserPreferencesPersistentData`'s values and are weak. The
inline slots 9-11 and the empty defaults of 27-40 are emitted next to the
local and remote users. Reconstructed: the constructor, destructor,
`GetPlatformUserId`, `GetPlatformName`, `_Reset`, `_ApplyTrackVolumeSettings`
("vca:/IGS" on the guitar part), `IsUsingOtherController` and the two session
preference helpers (`0x9ABEB0`, `0x9ABF00`). Declared: `SetCurrentCharName`
(`0x9A9BB0`), `ApplyVocalistVolumeSettings` (`0x9A9D00`), `GetAchievementData`.

`LocalBandUser` (552 bytes, vtable `0x1960620`) keeps an `RBGameplayOptions`
(+0xE8), a session view of the options (`SessionGameplayOptions`, vtable
`0x1960788`, +0x1B8), an `FTUEPersistentData` (+0x1C8) and the preference
values for use without a profile. Reconstructed: constructor, destructor,
`_Reset`, the preference setters, the picked getters, `_ResetPreferences`,
slots 12, 24 and 42. The getters that read the profile manager
(`0xD5D060`, `0xD5CEB0`) or `RBUserPreferencesPersistentData`'s accessors
(`0xD68C50`-`0xD68F30`), and the SOMP calls, are declared only.

`RemoteBandUser` (1352 bytes, vtable `0x19621C0`) keeps the session's
member record (opaque), a `GameplayOptions` (+0x340), the name (+0x418) and
online id text (+0x420). Only the small slots are reconstructed.

### RBBandUserMgr

The 352-byte subclass (vtable `0x1960498`) created by `RBBandUserMgrInit`
(`0x9231C0`), which sets `TheRBBandUserMgr` (`0x1ADFB18`) and
`theBandUserMgr`, calls `Init`, `SetTheUserMgr` and registers
`RBBandUserMgrTerminate` (`0x923190`) as an exit callback. `_CreateUsers`
(`0x923520`) makes eight `LocalBandUser`s and three `RemoteBandUser`s. It
also keeps the local (+0x100) and remote (+0x120) lists and the session
leader's member id (+0x140) and user id (+0x150). Reconstructed: the
creator, terminator, constructor, destructor, `_CreateUsers`,
`HasProfileAndController` (`0x923300`), `GetRBBandUserFromSlot`
(`0x9249B0`), `GetRemoteUserFromSlot` (`0x9249C0`) and
`GetLocalUserFromSlot` (`0x924A20`). Not reconstructed: the session-filtered
user lists (`0x923AF0`-`0x924970`) and the session updates
(`0x925680`-`0x926610`, called from `RBOvershellCom::_PollRemoteUsers`).

### Gameplay options

`IGameplayOptions` (32 pure slots, no destructor) and `GameplayOptions`
(184 bytes, vtable `0x1968880`, 38 slots: the 32, the destructors and four
change hooks) are declared in `src/rb_game/options/GameplayOptions.h`; the
method names are the map's `RBGameplayOptions` names where they fit. The
band users' slot 19 returns the interface.

## Defines

Every function from `0xA43F20` to `0xA459E8` is reconstructed;
`GetActiveScoreType` (`0xA44330`) reads the participating band users.
The enumerator names of `GameSlot` and `ScoreType` are the binary's own
strings; `ControllerType`'s and the new `CheatedReason` helpers' are
inferred. The predicates at `0xA43EF0` and `0xA43F10` classify a session
value at +9000 of the object at `0x1AE1818` that is not identified, and are
left out. `GetUILayoutEntity` (`0x9A39E0`) stays declared in `Defines.h`
but lies elsewhere.

## Engine declarations added

`PlatformMgr` slots 15-21 (`GetName` and `GetOnlineID` are the map's
names, the others inferred), the joypad user functions in `Joypad.h`,
`CSHA1::Update`/`Final`, `DateTime`/`GetDateAndTime`, and
`TrackTypeToSym`, `SymToTrackType` and `kTrackKeys`, `kTrackRealKeys`,
`kTrackNone` in `TrackType.h`.

The band users added: `Difficulty` and its conversions in
`src/rb_beatmatch/core/Defines.h` (`rb_beatmatch/Defines.o`, enumerator
names inferred), `RBPlayer::ChangeDifficulty` (`0xA5F1B0`, weak match) in
`src/rb_game/players/RBPlayer.h`, and `MicManagerInterface` and three
`MicClientMapper` members with `theMicClientMapper` (`0x1ADF358`) in
`src/mic/core/MicClientMapper.h` (see [rb-game-mics.md](rb-game-mics.md)).
