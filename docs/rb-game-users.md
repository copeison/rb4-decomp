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

## Band users (not yet reconstructed)

- `BandUser` (`0xA3ED10`-`0xA40130`) is abstract in this build: 72 bytes,
  vtable `0x1968498` with 24 slots (slots 9-13 and 18-22 pure). Its
  layout: difficulty (+32), track type (+36), controller type (+40),
  preferred score type (+44), +48 (cleared on reset), the `RBPlayer`
  (+56), the overshell slot state (+64), flags at +68, +70 (difficulty
  chosen) and +71 (participating). `Handle` answers the get/set difficulty,
  track, score type and controller messages, "get_slot_num" and
  "is_participating".
- Two derived classes hold the rest of the map's character code: an
  abstract one (vtable `0x1962360`, constructor `0x9A9A00`) and the local
  user (vtable `0x1960620`). **Slot 24, which `RBMetaState::_UpdateJoinedSlots`
  calls, belongs to these derived classes, not to `User` or `BandUser`**:
  the local user's returns true (likely `IsLocal`; the map has
  `LocalSysUser::IsLocal`).
- `BandUserMgr` (`0xA40130`-`0xA41BBC`, vtable `0x1968568`, 11 slots, slot
  10 pure) derives from `UserMgr`: users at +0x80, four slot `UserGuid`s at
  +0xA0, and two RakNet replicas at +0xE0 and +0xF0 (constructors
  `0xA84AF0`, `0xA8F1B0`). The concrete manager is a 352-byte subclass
  (vtable `0x1960498`) created at `0x9231C0`, which sets both
  `theBandUserMgr` and `0x1ADFB18`.
- `0xA41BC0`-`0xA43EE8` is the next object (unison phrase tracking).

## Defines

Every function from `0xA43F20` to `0xA459E8` is reconstructed except
`GetActiveScoreType` (`0xA44330`, declared: it needs `BandUser`'s virtuals).
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
