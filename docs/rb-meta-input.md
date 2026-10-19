# Meta input and character layout tracking

The game's UI input component from the map's `rb_meta/RBUIInputCom.o` and
the layout's character tracker from `rb_meta/RBCharacterLayoutTrackerCom.o`.
Names follow [naming.md](naming.md); the component machinery is described
in [entity-components.md](entity-components.md) and UIInputCom in
[ui-system.md](ui-system.md). `RBUILayoutCom`
([rb-meta-options-and-layouts.md](rb-meta-options-and-layouts.md)) gives
the scene's `RBUIInputCom` the input first and waits on the tracker;
`DialogMgrCom` ([rb-meta-dialogs.md](rb-meta-dialogs.md)) sets the input's
`ignore_sticky_focus`.

## Files

| Object | Range | Source |
| --- | --- | --- |
| `rb_meta/RBUIInputCom.o` | `0xCB1A00`-`0xCB3CDF` | `src/rb_meta/input/RBUIInputCom.cpp` |
| input registration and factory | `0xB92B20`, `0xBACAF0`, `0xBACB20` | `src/rb_meta/input/RBUIInputCom.cpp` |
| `rb_meta/RBCharacterLayoutTrackerCom.o` | `0xE5A540`-`0xE5AEFF` | `src/rb_meta/character/RBCharacterLayoutTrackerCom.cpp` |
| tracker registration and factory | `0xB750E0`, `0xBA63C0`, `0xBA63F0` | `src/rb_meta/character/RBCharacterLayoutTrackerCom.cpp` |

`RBUIInputReceiverCom.o` follows the input's object (its constructor is
at `0xCB3CE0`) and is not reconstructed; it calls the input's
`ShouldAutoStrumLock` and `IsStrumLockEnabled`.

## RBUIInputCom

A `UIInputCom` registered under UIInputCom's interface
(`metadata.mInterface`), after UIInputCom's own properties
(`UIInputCom::_InitAsSuperclass`, inline, emitted at `0xCB1EC0`). The
vtable at `0x1977FD0` has 43 slots and overrides the destructors
(`0xCB1F70`, `0xCB32B0`), `Handle` (`0xCB3170`), the identity slots
(`0xCB3330`-`0xCB33B0`, `0xCB3720`, `0xCB3730`), `_PreDestroy`
(`0xCB2340`, empty), `_Enter` (`0xCB2020`), `_Poll` (`0xCB2150`) and the
two joypad `OnMsg` slots (`0xCB2DE0`, `0xCB2FF0`). The object is 536
bytes.

| Offset | Field | Property |
| --- | --- | --- |
| 408 | `mRestoreFocusPending` | |
| 412 | `mLastAction` (15 at first) | |
| 416, 417 | `mFilterButtonDown`, `mFilterButtonUp` (true) | |
| 424 | `mStrumTimer` (`Hmx::Timer`, started) | |
| 448 | `mLastStrumMs[8]` | |
| 480 | `mNoDrumNavigationRemap` | |
| 488 | `mTransitionStartedSink` | |
| 528 | `mIgnoreStickyFocus` | `ignore_sticky_focus` |
| 529 | `mAutoStrumLock` | `auto_strum_lock` (hidden while strum lock is disabled) |
| 530 | `mDisableStrumLock` | `disable_strum_lock` (default true) |

An imprint copies only the three properties.

- Input filtering. `OnMsg(ButtonDownMsg)` records the action (Cancel sets
  `theRBMetaState.mBackPressed`) and asks `_FilterAction` (`0xCB2350`).
  With drum navigation (`RBSystemOptions::mIsDrumNavigationAllowed`) a
  held pad shift button turns Option into Up and Shell Option into Down;
  without it a held pad or cymbal shift button leaves no action. Neither
  applies while a cutscene or vote shows. A lefty user on a controller
  type that flips gets Up/Down and Right/Left swapped. A changed action is
  sent on to UIInputCom as a new `ButtonDownMsg` (or `ButtonUpMsg`) with
  the filter switched off meanwhile. Otherwise a d-pad up or down press
  within 50 ms of the pad's last one is swallowed (`_IsDoubleStrum`,
  inlined; uncalled copy at `0xCB2550`).
- Scene focus. A `"transition_started"` from theUI
  (`_OnTransitionStarted`, inlined into `Handle`; copy at `0xCB3130`)
  saves the focus when the last action was Confirm and forgets it
  otherwise. `_SaveSceneFocusState` (`0xCB25E0`) stores, under the
  scene's resource path, the ids from the scene entity's object down
  through instancing objects to the focused receiver's object, that
  object's name, and the focused row of a `UIListCom` on it.
  `_RestoreSceneFocusState` (`0xCB2180`, on the first poll after enter)
  resolves the path through the instances' entities (`0xCB2B00`),
  focuses the receiver and its row. `_ClearSceneFocusState` is
  `0xCB2CB0`. `ignore_sticky_focus` turns all three off.

### Globals

| Address | Name | Notes |
| ---: | --- | --- |
| `0x1B0F6A8` | `gNullObjectId` | the object's copy of the null id |
| `0x1B0F6B0` | `RBUIInputCom::sId` | `"RBUIInputCom"` |
| `0x1B0F6B8` | `RBUIInputCom::sClassName` | |
| `0x1B0F6C0` | `RBUIInputCom::sPropRegistry` | |
| `0x1B0F760` | `RBUIInputCom::sMetaData` | |
| `0x1B0F908` | `sSceneFocusStates` | `eastl::map<ResourcePath, SceneFocusState>`, 96-byte nodes |
| `0x1B0F940`-`0x1B0F988` | function statics | `_Enter`'s two symbols, `"none"`, `Handle`'s symbol, the metadata heap |

`SceneFocusState` (56 bytes): `mHasListFocus`, `mListFocusIndex` (-1),
`mFocusPath` (`eastl::list<GameObjectId>`), `mFocusName`. The repository's
`eastl::map` assigns values into raw nodes and frees them without
destruction, so the object specializes the map's node creation and freeing
for this value; its `eastl::list` has no assignment, so the path copy
(EASTL's `DoAssign`, `0xCB37F0`) is a local function.

## RBCharacterLayoutTrackerCom

"Handles layout-level tracking of character entities. Points at a scene
containing characters and an RBCharacterSceneTrackerCom, and forwards
requests on to it." (author Matt Ivester, category UI, allowed resource
`UILayoutResource`). A `Component`; the vtable at `0x1987D68` has 41
slots and overrides the destructors (`0xE5A580`, `0xE5A590`), the
identity slots (`0xE5AC50`-`0xE5ACD0`, `0xE5AE10`, `0xE5AE20`), slot 29
(`0xE5A980`) and `_Enter` (`0xE5AA20`). The object is 56 bytes.

| Offset | Field | Property |
| --- | --- | --- |
| 24 | `mCharacterSceneObj` | |
| 32 | `mSceneInstance` | |
| 40 | `mSceneTracker` | |
| 48 | `mCharacterSceneObjId` | `character_scene_obj_id` ("Character Scene Object ID", requires `SceneInstance`) |

The map names the class's resource override `_LoadResources(ObjPtr
const&)`; this build overrides slot 29 (`_OnResourcesLoaded`), which finds
the object and its `RndSceneInstanceCom`. `_Enter` finds the
`RBCharacterSceneTrackerCom` on the root of the scene's entity.
`AnyPlayerEntityIsLoading` (`0xE5AAA0`), `GetPlayerEntityByInstrument`,
`GetPlayerEntityForUser`, `GetPlayerEntityPathByInstrument` and
`GetPlayerEntityPathForUser` (`0xE5AAC0`-`0xE5AB70`) forward to it, with
null, false or the empty path without one. `GetForCurrentLayout`
(`0xE5ABE0`) returns the tracker on the root of theUI's current layout.

The statics are `gNullObjectId` with the shared header's group sizes
(`0x1B328E8`-`0x1B328F0`), `sId` (`0x1B328F8`), `sClassName`
(`0x1B32900`), `sPropRegistry` (`0x1B32910`) and `sMetaData`
(`0x1B329B0`), built by the static initializer at `0xE5AE30`.

## Declarations added

- `src/os/joypads/JoypadMsgs.h`: `ButtonDownMsg` and `ButtonUpMsg`
  (constructors `0x3A0440`, `0x3A0BD0`; `GetUser` `0x3A0BC0`, `0x3A1030`).
- `src/os/joypads/Joypad.h`: `JoypadData`, `JoypadGetPadData`,
  `JoypadControllerTypePadNum`, `JoypadTypePadShiftButton`,
  `JoypadTypeCymbalShiftButton`, `JoypadTypeHasLeftyFlip`.
- `src/ui/input/UIInputCom.h`: `SetFocusedObject(UIInputReceiverCom*,
  int, bool)` (`0x89D9F0`), `SetFocusFirstOnEnter` (`0x89EF20`) and the
  inline `_InitAsSuperclass`.
- `src/ui/lists/UIListCom.h` `SetFocusByDataIndex` (`0x8B80D0`) and
  `src/ui/lists/UIListData.h` `GetNumRows` (`0x8C3BE0`).
- `src/rb_meta/character/RBCharacterSceneTrackerCom.h`: the tracker's five
  queries (`0xB0BF70`-`0xB0C3E0`), `sId` (`0x1AF9DB0`) and
  `MusicianType::Instrument`.
- In `RBUIInputCom.cpp`, until rb_game declares them: BandUser's `User`
  base (cast inline), the identity `BandUserMgr::GetBandUser(User*)`
  (`0xA404F0`, cast inline) and BandUser's virtual `GetControllerType`
  (slot 8) and `GetGameplayOptions` (slot 19) with the options'
  `IsLefty` (slot 1), called through the vtable.

## Weak evidence

- `mNoDrumNavigationRemap`: nothing in this build sets it.
- `mFocusName`: written, never read.
- `ShouldAutoStrumLock`, `IsStrumLockEnabled`, `GetForCurrentLayout`,
  `FindObjectByPath`, `SceneFocusState` and `sSceneFocusStates` are
  inferred; the map has none of them.
- `ButtonDownMsg::GetValue` (the message's float).

## Not reconstructed

`RBUIInputReceiverCom`, `RBCharacterSceneTrackerCom`, BandUser's
virtual methods and the joypad functions are declared only. The binary
calls the out-of-line `UIInputCom::FindFocus(int) const` (`0x89D8E0`) and
`UIListCom::GetDataFocusIndex(bool) const` (`0x8B7E40`), which the model
keeps inline.
