# Meta overshell

`rb_meta/RBOvershellCom.o` (`0xBE6F60`-`0xBF727F`, static initializer
`0xBF7180`) holds `RBOvershellCom`, the component of the overshell scene
("Total control of the overshell scene.", author Sheila Porter, category UI,
allowed resource `RndSceneResource`). It owns the four `RBOvershellSlotCom`s
named by "overshell_slots", joins the band's users into them, resolves the
slots' states, runs the override flows that take every slot through one
screen, and keeps the slots' online state in a SOMP session. The
constructor publishes it as `theOvershell`.

The overshell, its slot component and the slot states are reconstructed
except the functions listed under "Not reconstructed", which need members
of BandUser, the game or the SOMP session that are not modelled yet.

## Files

| Object | Range | Source |
| --- | --- | --- |
| `rb_meta/RBOvershellCom.o` | `0xBE6F60`-`0xBF727F` | `src/rb_meta/overshell/RBOvershellCom.{h,cpp}` |
| `rb_meta/RBOvershellDefines.o` (declared) | | `src/rb_meta/overshell/RBOvershellDefines.h` |
| `rb_meta/RBOvershellListProviderCom.o` (declared) | | `src/rb_meta/overshell/RBOvershellListProviderCom.h` |
| `rb_meta/RBOvershellSlotCom.o` | `0xC05910`-`0xC24AFF` | `src/rb_meta/overshell/RBOvershellSlotCom.{h,cpp}` |
| `rb_meta/RBOvershellSlotState.o` | `0xC24B00`-`0xC2906F` | `src/rb_meta/overshell/RBOvershellSlotState.{h,cpp}` |

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
| `0x1B03ED0` | `RBOvershellSlotCom::sId` | required class of "overshell_slots" (see below) |

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
The slot com's field names and about 150 functions newer than the map
(the SOMP code especially, `OvershellSlotConnectionState`),
`RBOvershellRemoteSlotState` and the state slots 19-36 (Has/Get pairs for
the remote record), `_IsNoOnlineSlotUserReady`, `_RemoveUsersOverLimit`,
the local Game, GameMode, RBBTMMgr and joypad colour declarations in
`RBOvershellCom.cpp`.

## RBOvershellSlotCom (760 bytes)

The component of each overshell slot; vtable `0x19722A8` (41 slots:
destructors `0xC0A5C0`/`0xC0A810`, `Handle` `0xC1E680`, identity slots
`0xC24010`-`0xC24090`, `0xC24230`/`0xC24240`, slot 29 `0xC0F420`, `_Enter`
`0xC0F440`, `_Poll` `0xC0FC30`, `_HasPostPoll` `0xC24250`, `_PostPoll`
`0xC10900`). `RuntimeData` (680 bytes at 24, constructor `0xC05910`) holds
the overshell, the band slot (+0xA8, -1 for none), the state and the state
manager (+0x78, +0x88), the potential users, the queued messages (an
`eastl::deque`, `src/utl/containers/Deque.h`) and the SOMP request state;
then `list` (704), `confirm_action_text`, the heights (712, 716) and
`state_blenders` (720). `_Init` registers the slot's properties (list,
slot, views, online state, tooltips, mic monitors, button states) and six
exported events. Statics: `sId` `0x1B03ED0`, `sClassName` `0x1B03ED8`,
`sPropRegistry` `0x1B03EE0`, `sMetaData` `0x1B03F80`, the event symbols
`0x1B03EA8`-`0x1B03EC8`; static initializer `0xC24990`; `Init` `0xB88970`,
`_Create` `0xBABA30`.

The slot's user is `theBandUserMgr->GetUserFromSlot(band slot or 0)`.
`ShowState` sets the user's overshell slot state, runs `UpdateState` (the
states' exit and enter messages) and `UpdateView` and queues a SOMP update.
The `OvershellSlotStateID` enumerators (`kState_*`) come from the name table
at `0x1971C70`; ids 2, 22 and 58-60 are unnamed.

## Slot states

`RBOvershellSlotState` (88 bytes, vtable `0x1972468`, 49 slots: every query
is virtual in this build; the map's had three) holds the state's
`slot_states.dta` entry, its id, the slot and a `map<Symbol, DataNode>` of
properties; `GetStateID` is `0xC24C80`. `RBOvershellRemoteSlotState`
(vtable `0x19725F0`, newer than the map) serves entries with `(remote
TRUE)`. `RBOvershellSlotStateMgr` (96 bytes) reads `slot_states.dta` and
`state_transitions.dta` (`(from (to seconds)...)`, default 0.3 s, 0 from no
state). Handlers run with the slot as the object and `$overshell` set.

## Behaviour notes

- `_FindSlots`: each slot is the root of the entity its "overshell_slots"
  object instances; it gets its game and band slot, the input component and
  the overshell; the valid controllers are guitar, drums, vocals, guitar;
  the join order 0, 3, 2, 1.
- `GetOverrideFlow` is disabled while inactive, during a BTM event in game
  mode 8, during a SOMP session's song in kLayoutGame, and in the Winning,
  Losing, WatchingCutscene and Exiting gameplay states; it is
  SompTransition during the 2.5 s SOMP layout transition.
- A pending online state ends after 11 s, after 1.5 s once the users have
  settled, or when the slots are ready. A SOMP controller loss drops out to
  kLayoutSompGreenRoom with "somp_error_controller_disconnected".
- 0xBF2670 forwards a remote member's emote (0xD3BAF0); the third
  std::function (vtable `0x1971C00`) is `_ShowErrorZeroPlayersDialog`'s
  condition (`0xBF7020`); the wrap constructors inlined into `Handle` are
  `0xBF6860`, `0xBF6970` and `0xBF6A80` (`SystemOverlayStatusChangedMsg`,
  now in `PlatformMgr.h`).

## Not reconstructed

- RBOvershellCom (17 functions): `_CheckSompJoin`,
  `_ResolvePartWaitStates`, `_NeedsSompControllerReconnect`,
  `_RefreshJoinableUsers`, `_PollRemoteUsers`, `CheckAndAutoPauseGame`,
  `CheckAndAutoUnpauseGame`, `_PollSystemOverlay`, `OnMsg(GameStartedMsg)`
  (undeclared RBBandUserMgr members), `_HandleControllerDisconnect`,
  `_ShowSlotReconnectController`, `_ShowErrorZeroPlayersDialog`,
  `_DropInRemoteUser` (Game fields), `_AreRemoteSlotsConnected`,
  `_PollSompDropIn` (SOMP member records), `_Enter` (the game messages'
  event statics, the joypad subscription, GameMode as a MsgSource) and
  `Handle` (the game messages' wrap constructors).
- RBOvershellSlotCom (58 of about 309): the user joins and removals, the
  profile and character functions, part and difficulty selection and the
  SOMP part, score type and difficulty results (undeclared RBBandUser
  members); the in-song menu actions (the game); the song options, the SOMP
  record view and the emote menus (unmodelled tables).
- RBOvershellSlotState: the base `GetView` and `HandleMsg`, and the remote
  state's getters (RemoteBandUser's session record).
