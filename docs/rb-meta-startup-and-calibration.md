# rb_meta: startup, content refresh, calibration and leaderboard rows

The objects that follow the drivable checkbox in this build's rb_meta block
(`0xDA94A0`-`0xDBA25F`) and the calibration detector that the build places
after `RBAchUtl.o`. Each component follows the usual pattern (`sId`,
`sClassName`, `sPropRegistry`, `sMetaData`, the identity slots, `_Imprint`
and the property lambdas after them); the registrations and factories are
emitted with the other rb_meta classes'.

| Object | Range | Vtable | Source |
| --- | --- | --- | --- |
| `CalbertProcessor` | `0xF06030`-`0xF0727F` | none | `rb_meta/calibration/CalbertProcessor` |
| `CalibrationCom` ("Calibration") | `0xDA94A0`-`0xDB027F` | `0x19810F0` | not started |
| `CalibrationNumbersCom` ("CalibrationNumbers") | `0xDB0280`-`0xDB1A2F` | `0x1981370` | `rb_meta/calibration/CalibrationNumbersCom` |
| `ContentRefreshingCom` | `0xDB1A30`-`0xDB23A7` | `0x19814C8` | `rb_meta/content/ContentRefreshingCom` |
| `GameStartupCom` | `0xDB23B0`-`0xDB39CF` | `0x1981620` | `rb_meta/startup/GameStartupCom` |
| `LeaderboardCom` | `0xDB39D0`-`0xDB873F` | — | not started |
| `LeaderboardListItemCom` (newer than the map) | `0xDB8740`-`0xDB937F` | `0x1981980` | `rb_meta/leaderboards/LeaderboardListItemCom` |
| `ManualCalibrationFlowCom` ("ManualCalibrationFlow") | `0xDB9380`-`0xDBA25F` | `0x1981AD8` | `rb_meta/calibration/ManualCalibrationFlowCom` |

Registrations (`Init`) and factories (`_Create`): `CalibrationNumbersCom`
`0xB638A0`/`0xBA0AE0`, `ContentRefreshingCom` `0xB63C50`/`0xBA0B30`,
`GameStartupCom` `0xB643C0`/`0xBA0BD0`, `LeaderboardListItemCom`
`0xB64BE0`/`0xBA0D10`, `ManualCalibrationFlowCom` `0xB64FA0`/`0xBA0D60`.

## Behaviour

- `CalbertProcessor` resamples the input 50 times per burst period, scores
  each period's points by their rise (times the rise weight) plus the mean
  of the burst after them, and records the best point's position when its
  burst mean exceeds the threshold (4) times the rest of the period's mean.
  With ten bursts the middle ten give the delay unless their standard
  deviation exceeds 0.75; the progress is 0.05 per burst before that.
- `ContentRefreshingCom` makes the root's `RBUIInputCom` modal while
  `TheContentMgr` refreshes and keeps "loading_additional_progress" with the
  mounted and discovered content counts.
- `CalibrationNumbersCom` fills two `UIComboBoxEnumCom`s (in instanced
  scenes) with -300 to 300 ms in steps of 5 and stores the negated choices
  in the system options; `ManualCalibrationFlowCom` exports
  "calibration_flow" (-1 back, 1 forward, 2 shell option) for the meta
  state's calibration user.
- `GameStartupCom` runs the queued logo states. The ESRB state starts the
  included-content refresh, preloads the shell scenes, the four musicians
  and the title layout; the last state goes to the title. It is the writer
  of `RBMetaState::mApplyVolumeOptions` (once the shell banks load).
- `LeaderboardListItemCom` holds a leaderboard row's texts. Its first field
  sits after the `Component` base's tail padding (`alignas(8)`).

## Not reconstructed

- `GameStartupCom::_GotoTitle` (`0xDB3550`): it sets `ThePlatformMgr`+0x83
  and `theRBNetMgr`+0x80, whose layouts are not modelled; `_PostPoll` calls
  it. The inlined copy also calls an empty ContentMgr function
  (`0x393250`) and an archive handle cleanup (`0x3A8B60`).
- `CalibrationCom` (about 28 KB: the calibration test's audio and video
  states, the special pad reader and the results) and `LeaderboardCom`
  (about 20 KB: the leaderboard jobs and their JSON).

## Weak evidence

- Inferred names: the state and row enums, `RuntimeData` fields,
  `FillCalibrationComboBox`, `StoreSelectedCalibration`, the timer
  `gStartupTimer`, `Hmx::Timer::Restart`, `ContentMgr::IsRefreshDone` and
  `StartRefresh` (placed by map order), the `PlatformRegion` enumerators,
  `RBSongLibSortListProviderCom::kSortListScenePath`, and the
  `CalbertProcessor` fields.
- `RBCharacterPreviewMgrCom::kCharPreviewScenePath` and
  `kCharPreviewBackplateScenePath` are matched to the overshell band scenes
  (`0x19BE270`, `0x19BE278`) by their place beside `kDialogScenePath` and the
  "char_preview" tag that guards them.
