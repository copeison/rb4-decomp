# Meta state

The meta game's UI state from the map's `rb_meta/RBMetaStateCom.o`: the
UI layout IDs, `RBMetaState` (the one instance is `theRBMetaState`), which
goes to layouts and keeps the meta game's global state, and
`RBMetaStateCom`, the component that shows that state to the UI. Names
follow [naming.md](naming.md); the component machinery is described in
[entity-components.md](entity-components.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `rb_meta/RBMetaStateCom.o` | `0xBB0690`-`0xBC5810` | `src/rb_meta/state/RBMetaStateCom.cpp` |
| registration and factory | `0xB840D0`, `0xBAA980`, `0xBAA9B0` | `src/rb_meta/state/RBMetaStateCom.cpp` |

`RBUILayout.inc` lists the 108 layouts with the file `Goto` loads for each.

Declaration-only headers for what the object calls:
`src/rb_meta/somp/RBSompSessionMgr.h`, `src/rb_meta/clans/RBClanMgr.h`,
`src/rb_meta/character/RBCharacterMenuMgr.h`,
`src/rb_meta/onboarding/RBIGSOnboarding.h`, and outside the module
`src/rb_game/core/Defines.h` (`GameSlot`, `GetUILayoutEntity`),
`src/rb_game/mics/GameMicManager.h`, `src/mic/core/MicClientMapper.h`
(`MicClientID`) and `src/rb_world/characters/RBCharOutfitDesc.h`
(`RBChar::gCharGenderAllowedValues`).

## Layouts

`RBUILayoutToSym` (`0xBB06A0`) and `InitRBLayoutDefines` (`0xBC4760`)
name the layouts; the defines are one-node arrays set once with
`DataSetMacro`, `kLayoutInvalid` included. Data macros classify layouts:
`ROCK_SHOP_LAYOUTS`, `RIVALS_LAYOUTS`, `POTENTIAL_SOMP_LAYOUTS`,
`META_BACKGROUND_LAYOUTS` and `LAYOUTS_WITH_DUCKED_MUSIC` list layouts
(`0xBB0F40`-`0xBB15A0`, `0xBB1AE0`); `LAYOUT_MUSIC_STATES` and
`LAYOUT_AMBIENT_SOUND_STATES` hold `(state (layouts...))` entries
(`0xBB17C0`, `0xBB1D00`). A symbol in a list is skipped. The music and
ambient state of a rock shop layout during a character session come from
the layout the session started from (`0xBC4140`, `0xBC4180`).

`GetObjectUILayout` (`0xBB2020`) and `GetEntityUILayout` (`0xBB2110`) find
the `RBUILayoutCom` (sId `0x1B0FC60`, layout at +80) on the root object,
walking out through instancing parent objects; without one they return the
layout being gone to, or the current layout.

## RBMetaState

The object is 392 bytes (the map's is 304); its vtable at `0x1970888`
keeps `MsgSource`'s slots but for `Handle` (`0xBC48F0`: `"goto"` and
`"set_calibration_user"`). The static initializer inlines the
constructor. Fields, with their properties:

| Offset | Field | Notes |
| ---: | --- | --- |
| 24-36 | `mCurrentLayout`, `mPendingLayout`, `mReturnLayout`, `mPreviousLayout` | -1 at start |
| 48 | `mCalibrationUser` | `"set_calibration_user"` |
| 56 | `mFirstTimeCalibration` | `is_first_time_calibration` |
| 60-68 | `mFocusEntity` (4), `mFocusBodyPart` (-1), `mFocusGender` | character screen focus |
| 72 | `mHelpbarOverride` | `helpbar_override_config` |
| 80 | `mButtonOverrides[15]` | label and icon per joypad action |
| 320-326 | `mBackPressed`, `mLayoutChanged`, `mAnySlotJoinable`, `mSlotJoined[4]` | |
| 328 | `mFocusSlot` | `kNumGameSlots` for every slot |
| 332, 336 | `mSongListIndex`, `mPracticeSection` | written by other screens |
| 340-348 | `mSfxVolume`, `mCrowdVolume`, `mApplyVolumeOptions` | VCA volumes last applied |
| 352, 356 | `mNumMicsConnected`, `mMetaGameplayState` | |
| 360-362 | `mGoDirectlyToEditBand`, `mShellMusicStarted`, `mLoadingHeapStrategy` | |
| 368-388 | `mMainHeap`, `mGpuHeap` and their saved strategies | |

`Goto` (`0xBB5D40`) records the layout as pending, switches the "main"
and "gpu" heaps to strategy 3 for the difficulty select, game, loading
and BTM challenge layouts (`_SetLoadingHeapStrategy`, `0xBB6EC0`), and
calls `UIMgr::Goto` with the layout's file. The deprecated layouts without
a file do nothing; five dropped layouts and the developer menu fall back to
the title; the SOMP create and browse layouts go through the layouts after
them. It then marks the layout change and clears the dialog queue
(`DialogMgrCom::ClearAllDialogs`). `_SetCurrentLayout` (`0xBC3D00`) makes
the pending layout current.

`Poll` (`0xBB58C0`) attaches the shell music bus once
`audio_layers/shell_banks.scene` is loaded, applies the SFX and crowd
volume options to `vca:/SFX` and `vca:/Crowd` when enabled, plays
`event:/Shell/UI/os_back` when a back press and a layout change happened
in the same frame, updates the joined slots and counts the microphones
`TheGameMicManager` has for the four slots.

`Init` (`0xBB5590`) publishes the state as the script global
`rb_meta_state`, reads `use_authored_anim_timelines` and installs
`AnimDriverCom::sProjectTimelineCallback`: animations started in a UI
layout run on the UI clock unless the layout is listed or the animation is
a musician's.

The helpbar overrides export `helpbar_override_msg` and
`helpbar_button_override_msg` (action label icon) when they change
(`0xBB70A0`-`0xBB7420`); `ExportBootingPlayer` (`0xBC3BE0`) exports
`booting_player`.

## RBMetaStateCom

A 24-byte component (vtable `0x1970730`, 41 slots, checked with
`orbis-objdump -r`) whose `_Init` (`0xBB2300`, category UI, author "Chris
McCarthy") registers accessor-only properties: `current_layout` (setting
it goes to the layout), `current_layout_is_rock_shop`,
`current_layout_uses_meta_background`, the focus entity, body part and
gender, the background music, ambient sound and ducking states,
`current_meta_gameplay_state`, `helpbar_override_config`,
`is_dialog_active`, `is_first_time_calibration`, `game_version_text`,
`current_band_name`, `is_any_slot_joinable`,
`show_guitar_solo_tutorial_invite`, `is_autosaving`,
`num_microphones_connected`, the `joined_slots` struct (`guitar0`,
`drum`, `vocals`, `guitar1`, `either_guitar_joined`), `is_somp`,
`is_library_filtered_for_challenge` and `go_directly_to_edit_band`.

Statics: `sId` `0x1AFEF48`, `sClassName` `0x1AFEF50`, `sPropRegistry`
`0x1AFEF60`, `sMetaData` `0x1AFF000`; `theRBMetaState` `0x1AFF588`. The
authored-timeline layouts are at `0x1AFEF20` and "__no_button_override"
at `0x1AFEF40`.

## Not reconstructed

- The business-intelligence reports (`0xBB7550`-`0xBC3BB0`), which read
  most of the game's state.
- `_UpdateJoinedSlots` (`0xBB5C60`), which calls a `User` virtual (slot
  24) that is not modelled, and `GetCurrentBandName` (`0xBC4080`), which
  reads rb_game managers that are not modelled.

## Weak evidence

- Inferred names: `IsRockShopLayout` and the other macro lookups,
  `GetObjectUILayout`, `GetEntityUILayout`, `MetaGameplayState`,
  `BGMusicState`, `BGAmbientSoundState`, `BGDuckMusicState`,
  `_SetLoadingHeapStrategy`, `ClearHelpbarButtonOverrides`,
  `ExportBootingPlayer`, `IsWatchingCutsceneOrVoting`, `GetCurrentPatch`,
  `GetGameVersionText`, the message classes and `ButtonOverride`'s fields.
- `mReturnLayout`, `mSongListIndex` and `mPracticeSection` are named from
  single writers; `mApplyVolumeOptions` has no writer in this build.
- `RBSompSessionMgr`, `RBClanMgr` and their flags are named from the SOMP
  and clan strings; `FmodBusInterface::SetVcaVolumeDb` (`0xBF9F0`) is
  placed in that class because `Poll` calls it on `gFmodBusInterface`.
  `GetUILayoutEntity` (`0x9A39E0`) is placed in `rb_game/Defines.o` from its
  neighbours.
