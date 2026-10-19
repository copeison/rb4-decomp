# Meta profile manager

The game's profile manager from the map's `rb_meta/RBProfileMgr.o`, built
on the persistence `ProfileMgr` ([persistence-profiles.md](persistence-profiles.md)).
Names follow [naming.md](naming.md); the profiles themselves are in
[rb-meta-profiles.md](rb-meta-profiles.md) and the system options in
[rb-meta-options-and-layouts.md](rb-meta-options-and-layouts.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `rb_meta/RBProfileMgr.o` | `0xD5B320`-`0xD62D4F` | `src/rb_meta/profiles/RBProfileMgr.{h,cpp}` |
| `ProfileMgr::CreateProfile<RBProfile>` | `0xBB02E0` | `src/rb_meta/profiles/RBProfileMgr.cpp` (declared in `ProfileMgr.h`) |

The EASTL helpers of the slot map, the critical profiles and the sets
(`0xD62580`-`0xD62B40`), and the `std::function` of the solo warning's
empty callback (vtable `0x197DF40`), are the compiler's. The object is
followed by `RBRockShopPersistentData.o` (`0xD62D50`).

## RBProfileMgr (1144 bytes)

`Init` (`0xD5BB00`, called by the game's startup before
`SaveLoadMgr::Init`) allocates the manager from the "profile" heap (a class
`operator new`, `MemAlloc(size, "RBProfileMgr", 0)`), sets
`theRBProfileMgr` (`0x1B1F7F0`) and the "profile_mgr" global, subscribes to
the events (`_RegisterForEvents`, `0xD5BBF0`) and starts the
"rbprofile_signed_in_poll" thread (`0xD5C070`, priority 701).

The vtable at `0x197DEB0` has 16 slots:

| Slot | Address | Member |
| ---: | --- | --- |
| 0-1 | `0xD5B6C0`, `0xD5BAE0` | destructors |
| 2 | `0xD61F20` | `Handle` |
| 7 | `0xD62560` | `GetSystemOptions` |
| 8 | `0xD62570` | `GetSystemSaveFileName` ("rb_system_settings.bin") |
| 9 | `0xD5F0E0` | `_OnProfileAdded`: rich presence |
| 10 | `0xD5F780` | `_OnProfileAboutToBeRemoved` |
| 11 | `0xD5F9E0` | `_OnProfileFailedToLoad` |
| 12 | `0xD5FF80` | `_OnProfileFailedToSave` |
| 13 | `0x3B4190` | `_UsesSaveLoadMgr` (inherited) |
| 14 | `0xD5F070` | `_OnUserLoggedIn`: `CreateProfile<RBProfile>` |
| 15 | `0xD60010` | `_OnAppResumed`: resets the profiles' server state |

| Offset | Field | Notes |
| ---: | --- | --- |
| 344 | `mSystemOptions` | `RBSystemOptions` |
| 680 | `mProfileData` | `eastl::map<unsigned long, ProfileData>`, slots 0-3 |
| 736 | `mCriticalProfiles` | `vector<CriticalProfile>` (profile, count) |
| 768-770 | `mShowCriticalDropOutDialog`, `mShowLoadFailedDialog`, `mShowSaveFailedDialog` | postponed dialogs |
| 776 | `mAddedProfilePads` | pads checked for a finished load |
| 808 | `mDestructionQueue` | profiles whose load failed |
| 840 | `mMode` | 0 none, 1 menus, 2 game |
| 844 | `mLastLayout` | -1 at start |
| 848-851 | `mBootToMainMenuFromGame`, `mShowLostConnectionDialog`, `mServerConnected` (true), `mShowSoloNoServerWarning` (true) | |
| 856 | `mSignedInPollThread` | `NamedThread*` |
| 864-1104 | seven `EventSinkElem`s | game created, game started, pre game ended (`theGameData`), song cache dirty (`theSongMgr`), system options dirty, connection status (`ThePlatformMgr`), transition done (`theUI`) |

`ProfileData` (24 bytes) is the slot's profile, the profile waiting for it
during a song and whether the slot owns the profile's settings (the first
slot to hold it).

Statics: the lock `gRBProfileMgrCritSec` (`0x1B1F7E0`, "hx crit sec") and
the thread's run flag (`0x1B1F808`); both are newer than the map and their
names are inferred.

## Behaviour

- `Poll` (`0xD5C230`) holds the lock while it polls the profiles, exports
  "profile_dropped_in" for added pads whose profile finished loading,
  exports "profile_failed_to_saveload" and destroys the queued profiles,
  shows the postponed dialogs on the main menu, returns the mode to the
  menus when the layout leaves the game, ends a game interrupted by a boot,
  shows "error_lost_connection" and starts the profiles' server jobs.
- A failed load or save shows its dialog on the title screen or boots to
  the main menu (`_BootToMainMenu` waits for "transition_done") and shows
  it there; failures are ignored without a layout and on the startup
  layout. A failed load also queues the profile for destruction. Removing a
  critical profile clears the critical list and boots with
  "critical_user_drop_out".
- `AssignProfileToSlot` parks the profile as pending while the game is on
  (outside the results screen); "game_started" and the return to the menus
  (`_UpdateMode`) resolve the pending profiles and export
  "pending_profiles_resolved". `RemoveProfileFromSlot` exports
  "profile_about_to_drop_out" for a profile no other slot holds.
- The worker thread asks `ThePlatformMgr->IsSignedIntoLive` for each pad
  every half second, outside the lock, and stores the answers in the
  profiles under it.
- `Handle` takes "game_created" (mode 2, forced), "game_started",
  "pre_game_ended", "connection_status_changed",
  "platform_entitlements_retrieved", then `ProfileMgr`'s messages, then
  "on_song_cache_dirty" (profiles and options dirty),
  "on_system_options_dirty" (profiles dirty) and "on_transition_done".

- In the menus, with the server and the network up
  (`PlatformMgr::IsEthernetCableConnected`, `0x363760`), each loaded,
  signed-in profile gets an `RBNetEntitlementsJob` ("client_entitlements/get/")
  and an `RBNetUpdateSongsJob` ("leaderboards/player/update_songs/", the
  `SongMgr::GetPlaybleSongSet`), queued with `theRBNetMgr.ManageJob`; the
  jobs report back through `OnEntitlementsJobDone` (`0xD61C40`) and
  `OnSongUploadJobDone` (`0xD61D50`). Guests drop their entitlements and
  count as uploaded. A reconnection re-requests both.

The game messages come from `rb_game/RBGameData.o` and are declared in
`RBProfileMgr.cpp` (with `theGameData`, `0x1AE1818`) until that object
exists; `GamePreEndedMsg` has two arguments in this build (the map's has
one). `ConnectionStatusChangedMsg` and `PlatformEntitlementsRetrievedMsg`
are in `os/platform/PlatformMgr.h`. `RBNetMgr` and the two job classes are
declared only, in `src/rb_meta/net/`.

## Not reconstructed

These are declared and called but not defined, because they read objects
that are not modelled:

- `_UpdateRichPresence` (`0xD5F150`): the platform's presence slot
  (`PlatformMgr` vtable +440) and the game's song.
- `_UpdateProfileDataFromGameState` (`0xD60180`): the game's players,
  the campaign and the net manager.
- `_CheckForMainMenuBootFromGame` (`0xD5C9B0`): the game object at
  `0x1ADF7D0`.
- The entitlement handler (`0xD618A0`) and the uncalled platform request
  (`0xD61520`, `PlatformMgr` vtable +464): the entitlement table at
  `0x1AF4E38` and the platform slot.
- `GetCampaignParticipatingProfiles` (`0xD5E270`), the session variant
  (`0xD5E400`) and `GetProfileFromOnlineId` (`0xD5CF30`).

## Weak evidence

`ProfileMgrMode`'s and `ProfileMgrDialogType`'s enumerators, the field
names, `GetSlotProfileFromPadNum`, `GetSettingsSlotIdxFromProfile`,
`GetProfiles`, the XP widget exports, `ShowSoloNoServerWarning`,
`AnyParticipatingProfileSubmittingScores`, `_AnyParticipatingProfileSignedIn`,
`_AnyParticipatingProfileOnline`, `_CheckForLostConnection`,
`GamePreEndedMsg::GetValue`, `CreateProfile`, the server-job helpers,
`RBNetEntitlementsJob`, `RBNetUpdateSongsJob` and
`PlatformMgr::IsEthernetCableConnected` (the map's name for a function
that refreshes and returns a cached link flag).
