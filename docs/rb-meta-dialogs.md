# Meta dialogs

`rb_meta/DialogMgrCom.o` (`0xADAE60`-`0xAE2620`) holds the game's popup
dialogs: `DialogMgrCom`, the component of the dialog scene
(`ui/dialog/dialog.scene`, `DialogMgrCom::kDialogScenePath` at
`0x19BE248`), and the static dialog manager around it. The source is
`src/rb_meta/dialog/DialogMgrCom.{h,cpp}`.

The game fills a `DialogMgrCom::DialogArgs` with one of the `InitFor*`
functions and passes it to `ShowDialog`. When it is safe (no active or
pending dialog, and `UIMgr::IsInTransition` false), `ShowDialog` copies the
arguments to the current dialog and adds the dialog scene; otherwise it
queues a copy. A conditional dialog without a condition function is
dropped. When the scene's `DialogMgrCom` enters, it becomes the active one
and copies the texts and icons into its own fields, which its properties
read. Selecting one of its button objects, the cancel action, or a met
condition hides the dialog (`_HideDialog`): it removes the scene, clears the
current dialog and calls the dialog's callback with the answer (`Button`)
and the user. `DialogMgrCom::Poll` (called by `App::RunOneFrame`) shows the
first queued dialog once it is safe. `RBMetaState::Goto` drops everything
through `ClearAllDialogs`.

## Globals

| Address | Name | Notes |
| ---: | --- | --- |
| `0x1AF49E8` | `gNullObjectId` | -1, set first by the static initializer, with the group sizes 8 and 4 after it |
| `0x1AF49F8` | `DialogMgrCom::sId` | `"DialogMgrCom"` |
| `0x1AF4A00` | `DialogMgrCom::sClassName` | `"DialogMgrCom"` |
| `0x1AF4A10` | `DialogMgrCom::sPropRegistry` | revision 0 |
| `0x1AF4AB0` | `DialogMgrCom::sMetaData` | category UI, author "Chris McCarthy", allowed resource `RndDrawableEntityResource` |
| `0x1AF4C60` | `sDialogArgs` | the current dialog (256 bytes) |
| `0x1AF4D60` | `sDialogQueue` | `eastl::vector<DialogArgs>`; `_Init` reserves 16 |
| `0x1AF4D80` | `sDialogQueueLock` | `CritSec` ("hx crit sec") guarding the queue |
| `0x1AF4D90` | `sActiveDialog` | the entered `DialogMgrCom`, or null |
| `0x1AF4D98`-`0x1AF4DC8` | `_Enter`'s statics | `dialog_override`, `dialog_cancel_override`, `dialog_cancel_only_override`, `dialog_directed_override` |
| `0x196E770` | (compiler) | `_Enter`'s switch table of the helpbar overrides for types 3-7, right after the vtable |

The map lists only `sId`, `sPropRegistry`, `sMetaData` and
`kDialogScenePath`; the current dialog, the queue, its lock and the active
component were file statics, so their names are inferred. The static
initializer (`0xAE23E0`) constructs them in that order and registers the
destructors of `sDialogArgs` (`0xADAE60`) and the queue (`0xADAED0`).

Both `ShowDialog` (when it queues) and `Poll` take the lock with
`ScopedCritSec`; `ClearAllDialogs` holds it while it clears the current
dialog and the queue. `ShowDialog` itself does not lock the current dialog.

## DialogArgs (256 bytes)

| Offset | Field | Notes |
| ---: | --- | --- |
| 0 | `mLayoutType` | `DialogLayoutType`, `"dialog_layout_type"` |
| 4 | `mType` | `DialogType`, `"dialog_type"` |
| 8 | `mName` | the dialog's id (`IsDialogActive(Symbol)`): the token of the main text |
| 16-40 | `mHeaderText`, `mSubHeaderText`, `mBody1Text`, `mBody2Text` | texts, localized by the `InitFor*` functions unless `DialogTextLocType` is literal |
| 48-96 | `mOkButtonTextToken` ... `mOptionDButtonTextToken` | button labels (ok, yes, no, options A-D) |
| 104, 112 | `mIcon1Image`, `mIcon2Image` | `ResourcePath`s to `RndTexture2DResource`s |
| 120 | `mInitialButtonFocus` | 0 default, 1 Yes, 2 No, 3 Cancel |
| 124, 128 | `mXPosOverride`, `mZPosOverride` | `FLT_MIN` when unset; the directed layout's position |
| 132 | `mModal` | `UIInputCom::SetIsModal` on the scene's `RBUIInputCom` |
| 136 | `mRequiredInstrument` | 0 any, 1 guitar, 2 bass, 3 vocals, 4 drums |
| 144 | `mOnHide` | `std::function<void (Button, BandUser*)>` (48 bytes) |
| 192 | `mCondition` | `std::function<bool ()>` |
| 240 | `mPending` | set by `InitFor*`, cleared by `Clear`: a dialog is shown or its scene is being added |

The constructor, destructor and `Clear` are inline (the map emits them in
`rb_audio/RBSong.o`; this build's `Clear` is at `0x8F82D0`). `Clear` sets the
symbols with `Symbol("")` and the paths with `ResourcePath::operator=`.

`_HideDialog` ignores an answer from a user who neither holds
`theRBMetaState.mFocusSlot` nor plays the required instrument
(`BandUser::GetTrackType`), as long as some band user plays it.

## DialogMgrCom (464 bytes)

| Offset | Field |
| ---: | --- |
| 24, 32 | `mIcon1Image`, `mIcon2Image` |
| 40-120 | `mHeaderText` ... `mOptionDButtonTextToken` (11 symbols) |
| 128 | `mPreviousHelpbarOverride`: the helpbar override `_Enter` replaced, restored by the destructor |
| 136, 140 | `mXPosOverride`, `mZPosOverride` (left uninitialized by the constructors) |
| 144-424 | eight `PropArray<GameObjectId>`: `ok_buttons`, `yes_buttons`, `no_buttons`, `cancel_buttons`, `option_a_buttons` ... `option_d_buttons` |

The properties have no storage (offset -1): `dialog_layout_type`,
`dialog_type` and the read-only `initial_button_focus` read and write
`sDialogArgs`; the icons, texts and position overrides read and write the
component's copies. None of them is saved. The copy constructor (inlined
into `_Imprint`, binary copy `0xAE0D30`) copies only the button arrays.

The vtable at `0x196E628` has 41 slots; it overrides slots 0-1 (`0xADB150`,
`0xADB420`), 2 `Handle` (`0xAE0780`), 4 `GetId` (`0xAE0950`), 5
`GetClassName` (`0xAE0960`), 7 `CurrentRev` (`0xAE0970`), 8 `IsA`
(`0xAE0990`), 9 `AsComponent` (`0xAE09C0`), 10 `_Imprint` (`0xAE09D0`), 22
`_GetPropRegistry` (`0xAE0AE0`), 23 `_GetMetaData` (`0xAE0AF0`), 31 `_Enter`
(`0xADE350`) and 33 `_Poll` (`0xADE660`). The built object's vtable matches
slot for slot. `Init` and `_Create` are emitted with the other rb_meta
registrations (`0xB64000`, `0xBA0B80`).

## Functions

| Address | Function |
| ---: | --- |
| `0xADAF90` | constructor |
| `0xADB150` | destructor |
| `0xADB440` | `_Init` |
| `0xADD090` | `GetDialogTypeAllowedValues` (inferred) |
| `0xADD560` | `ShowDialog` |
| `0xADD8B0` | `CancelDialog(bool)` (inferred): hides with No, Cancel or OK by type |
| `0xADD8F0` | `_HideDialog(Button, BandUser*, bool)`; the map has no flag |
| `0xADDB80` | `ClearAllDialogs` (inferred; named by `RBMetaState::Goto`) |
| `0xADDDB0`, `0xADDF40` | EASTL `vector::swap` and the shrink, for `ClearAllDialogs` |
| `0xADE0A0` | `_IsUserRequiredInstrument` (inferred), inlined into `_HideDialog` |
| `0xADE100`, `0xADE110` | `IsDialogActive()`, `IsDialogActive(Symbol)` |
| `0xADE120` | `Poll` |
| `0xADE350` | `_Enter` |
| `0xADE660` | `_Poll` (compares the type with 2 and 5 but runs the check either way) |
| `0xADE6A0` | `_UpdateConditionDialog` |
| `0xADE6D0` | `OnMsg(UISelectMsg const&)` |
| `0xADEAF0` | `OnMsg(UIButtonDownMsg const&)` (newer than the map) |
| `0xADEBA0`-`0xAE05E0` | the 17 `InitFor*` functions |
| `0xAE0780`-`0xAE0AF0` | `Handle` and the identity slots |
| `0xAE0B00`-`0xAE0D20` | property accessor lambdas |
| `0xAE0D30`-`0xAE2170` | copy constructor and EASTL vector helpers |
| `0xAE23E0` | static initializer |

`_SafeToShowDialog` and the map's `_IsOkButton`, `_IsYesButton` and
`_IsNoButton` are inlined in this build; the five other button checks are
inferred names.

## InitFor* functions

| Address | Function | Layout | Id |
| ---: | --- | --- | --- |
| `0xADEBA0` | `InitForStandardTextPopup` (+ focus) | 0 | body |
| `0xADED20` | `InitForStandardTextPopup` (+ focus, OK label) | 0 | body |
| `0xADEE50` | `InitForCustomTextPopup` (Yes/No labels, focus) | 9 | body |
| `0xADEFE0` | `InitForCustomTextPopup` (+ header) | 9 | body |
| `0xADF1C0` | `InitForTextOnlyOnboardingPopup` | 1 | body |
| `0xADF370` | `InitForTextOnlyOnboardingPopup` (+ required instrument) | 1 | body |
| `0xADF530` | `InitForOneIconOnboardingPopup` | 2 | header |
| `0xADF730` | `InitForTwoIconOnboardingPopup` | 3 | header |
| `0xADF970` | `InitForTextDirectedOnboardingPopup` (inferred) | 10 | body |
| `0xADFB40` | `InitForBannerUnlockPopup` | 6 | header |
| `0xADFD80` | `InitForTextOnlyUnlockPopup` | 4 | header |
| `0xADFF70` | `InitForPatchNotesPopup` (inferred, no caller) | 7 | header |
| `0xAE0120` | `InitForSetlistChoicePopup` (inferred) | 8 | body |
| `0xAE02B0` | `InitForTwoOptionPopup` (inferred, no caller) | 9, type 6 | body |
| `0xAE0440` | `InitForThreeOptionPopup` (inferred, no caller) | 9, type 7 | body |
| `0xAE05E0` | `InitForFourOptionPopup` (inferred) | 9, type 4 | body |

## Dependencies

- `RBUIInputCom` ([rb-meta-input.md](rb-meta-input.md)): `_Enter` sets
  its `ignore_sticky_focus` (offset 528), so the dialog scene keeps no
  remembered focus, and registers the dialog with
  `UIInputCom::RegisterCom`.
- `RBOvershellCom::IsUserAllowingInputToShell(BandUser*)` (`0xBEB060`) on
  `theOvershell` (`0x1B02C38`, set by the overshell's constructor): both
  names are inferred and weak.
- `BandUser::GetTrackType`, `BandUser::GetSlot`, `BandUserMgr::GetBandUsers`
  on `theBandUserMgr`, `TrackType`, and the `UISelectMsg` and
  `UIButtonDownMsg` messages (`src/ui/input/UIMsg.h`), declared only.
- The joypad actions OnMsg checks (1 confirm, 2 cancel) are local
  constants, since `JoypadAction` has no enumerators yet.

## Weak names

`DialogModalType`'s and `DialogTextLocType`'s enumerators (which value means
modal and literal), `DialogLayoutType`, `RequiredInstrument`, `mPending`,
`CancelDialog`, `_IsUserRequiredInstrument`, the inferred `InitFor*` names,
`theOvershell` and `IsUserAllowingInputToShell`.
