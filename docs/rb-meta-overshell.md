# Meta overshell

`rb_meta/RBOvershellCom.o` (`0xBE6F60`-`0xBF727F`, static initializer
`0xBF7180`) holds `RBOvershellCom`, the component of the overshell scene
("Total control of the overshell scene.", author Sheila Porter, category UI,
allowed resource `RndSceneResource`). It owns the four `RBOvershellSlotCom`s
named by "overshell_slots", joins the band's users into them, resolves the
slots' states, runs the override flows that take every slot through one
screen, and keeps the slots' online state in a SOMP session. The
constructor publishes it as `theOvershell`.

This pass declares the whole class (every function of the object, with its
address) and reconstructs the state, the registration, the identity slots
and the self-contained members. The rest is declared only (see "Not
reconstructed").

## Files

| Object | Source |
| --- | --- |
| `rb_meta/RBOvershellCom.o` | `src/rb_meta/overshell/RBOvershellCom.{h,cpp}` |
| `rb_meta/RBOvershellDefines.o` (declared) | `src/rb_meta/overshell/RBOvershellDefines.h` |
| `rb_meta/RBOvershellSlotCom.o` (declared: `sId`) | `src/rb_meta/overshell/RBOvershellSlotCom.h` |

## Globals

| Address | Name | Notes |
| ---: | --- | --- |
| `0x1B02988` | `gNullObjectId` | -1; the other shared header statics (`0x1B0298C`-`0x1B029D8`: a SOMP "null member" record) are not modelled |
| `0x1B029E0` | `RBOvershellCom::sId` | "RBOvershellCom" |
| `0x1B029E8` | `RBOvershellCom::sClassName` | "RBOvershellCom" |
| `0x1B029F0` | `RBOvershellCom::sPropRegistry` | |
| `0x1B02A90` | `RBOvershellCom::sMetaData` | |
| `0x1B02C38` | `theOvershell` | set by the constructor, cleared by the destructor |
| `0x1971C40` | (table) | `OvershellOnlineStateToString`'s labels, indexed by state + 1 |
| `0x1B03ED0` | `RBOvershellSlotCom::sId` | required class of "overshell_slots" |

Registration `Init` is emitted at `0xB87E00`, the factory `_Create` at
`0xBAB940` (`new(0x428)`).

## Properties

| Name | Storage | Notes |
| --- | --- | --- |
| `overshell_slots` | `mOvershellSlots` (1024), `PropArray<GameObjectId>` of 4 | fixed size; items require `RBOvershellSlotCom` |
| `active_status` | accessors | symbol: kOvershellInactive / kOvershellInMeta / kOvershellInSong; not saved; the setter writes the field directly |
| `should_be_showing` | accessor | read-only bool: active and the override flow is not disabled |

## RBOvershellCom (1064 bytes)

`Component` (24), `RuntimeData` (24-1023), `mOvershellSlots` (1024).

| Component offset | Field | Notes |
| ---: | --- | --- |
| 24 | `mNeedsFindSlots` | `_EditPoll` runs `_FindSlots` once |
| 25, 26 | `mSystemOverlayOpen`, `mSystemOverlayClosed` | |
| 32 | `mSlots` | `vector<RBOvershellSlotCom*>`, sized 4 |
| 64 | `mJoinOrder` | slot order 0, 3, 2, 1 |
| 96 | `mInputCom` | the object's `RBUIInputCom` |
| 104 | `mLostConnectionToServer` | |
| 108 | `mOverrideFlow` | |
| 112 | `mOverrideFlowRequests` | 12-byte {begin, flow, cancelled} |
| 144 | `mActiveStatus` | default kOvershellInMeta |
| 148-151 | venue meta, song options required, extended mic arrows, button pulse (default true) | |
| 152 | `mQueuedJoinUsers` | |
| 184, 192 | `mPartRand`, `mRandomSeed` | |
| 200-232 | idle master guid, pad, profile, slot | |
| 240 | `mDifficultySelectMs` | gig/config.dta show + grace ms |
| 244, 245 | cached blocking flags | |
| 248 | `mOnlineState` | Offline 0, PendingOffline 1, Online 2, PendingOnline 3 |
| 256 | `mOnlineSlots` | `eastl::set<unsigned long>` |
| 312-336 | online/dialog/timer flags, `mLightLayout` | `mJoinBlocked` (314) and `mMovingUser` (324) are written by other objects |
| 344-1023 | 17 `MsgSource::EventSinkElem` subscriptions | made by `_Enter` and `OnMsg(GameCreatedMsg)` |

The vtable at `0x19719E8` has 41 slots and overrides 0-1 (`0xBE88D0`,
`0xBE8970`), 2 `Handle` (`0xBF4700`), 4, 5, 7-10 (`0xBF6370`-`0xBF63F0`),
22-23 (`0xBF6580`, `0xBF6590`), 29 `_OnResourcesLoaded` (`0xBF2860`, the
map's `_LoadResources`), 31 `_Enter` (`0xBF2A40`), 33 `_Poll` (`0xBF36A0`),
38 `_EditPoll` (`0xBF3C70`), 39 `_HasPostPoll` (`0xBF65A0`) and 40
`_PostPoll` (`0xBF3CA0`). The built object's vtable matches slot for slot.
`OvershellOverrideEndedMsg` (`Message<2>`, "override_ended") has its vtable
at `0x1971B40`, constructor `0xBF6610`.

## Functions

Each declaration in the header carries its address; the main groups:

| Range | Functions |
| --- | --- |
| `0xBE6F60`-`0xBE6F80` | online state labels and allowed values |
| `0xBE72D0`-`0xBE8970` | RuntimeData, `_Init`, constructor, destructors |
| `0xBE8A10`-`0xBE9EE0` | slot lookup, active status, venue meta mode, idle master, joining users |
| `0xBE9F70`-`0xBEB060` | `Reset`, override flows and their request queue, input checks |
| `0xBEB1C0`-`0xBED090` | resolving the slot states, controller disconnects |
| `0xBED0A0`-`0xBF2830` | SOMP: reconnect dialog, remote users, online transitions, client message forwarders |
| `0xBF2860`-`0xBF45C0` | resources, `_Enter`, `_FindSlots`, `_Poll`, post-poll, `OnMsg` handlers |
| `0xBF4700` | `Handle` |
| `0xBF6370`-`0xBF7170` | identity slots, property lambdas, message and container helpers |

## Weak names

Almost every name of the SOMP code (newer than the map), the field names,
`GetOverrideFlow`, `_CanChangeOverrideFlow`, `Reset`, `IsOnline`,
`IsUserAllowingInputToShell`, the `OvershellOverrideFlow` enumerator names
(flow 1 also starts the drop-in), and the OnSomp forwarders' meaning.

## Not reconstructed

Declared only: the functions that call into classes not yet modelled —
`BandUser` (in this build a chain User, BandUser, an unnamed base and the
local/remote user classes), `RBOvershellSlotCom` and `RBOvershellSlotState`
(49-slot vtable at `0x1972468`), `GameMode`, `Game`, the band user manager
subclass at `0x1ADFB18`, the SOMP session manager's members and client
(`0x1B156C8`), and the game messages. These are `UpdateAll`, the override
flow and online transition logic, `_Enter`, `_Poll`, `_FindSlots`,
`Handle` and the `OnMsg` handlers. They should land with the slot, slot
state and user classes.
