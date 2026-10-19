# Meta options and layout components

The game's console-wide options from the map's `rb_meta/RBSystemOptions.o`
and the UI layout component from `rb_meta/RBUILayoutCom.o`. Names follow
[naming.md](naming.md); the component machinery is described in
[entity-components.md](entity-components.md), the persistence base classes
in the persistence documents, and the meta state the layout component
reports to in [rb-meta-state.md](rb-meta-state.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `rb_meta/RBSystemOptions.o` | `0xD663F0`-`0xD687FF` | `src/rb_meta/profiles/RBSystemOptions.cpp` |
| `rb_meta/RBUILayoutCom.o` | `0xCB49F0`-`0xCB5B0F` | `src/rb_meta/ui/RBUILayoutCom.cpp` |
| layout registration and factory | `0xB936F0`, `0xBACBE0`, `0xBACC10` | `src/rb_meta/ui/RBUILayoutCom.cpp` |

IDA leaves most of `RBSystemOptions.o` undefined (Save, Load, the
destructors and most setters); they were read from the raw code.

Declaration-only headers for what the objects call:
`src/rb_meta/songs/SongMgr.h` (`SongMgr::SaveSongCache`,
`SongMgr::LoadSongCache`, `theSongMgr`), `src/rb_meta/songs/RBSongSort.h`
(`RBSongSort::SortMethod`). `RBUIInputCom` and
`RBCharacterLayoutTrackerCom` are described in
[rb-meta-input.md](rb-meta-input.md). `UIButtonDownMsg` comes from
`src/ui/input/UIMsg.h`. Engine headers gained
`UIInputCom::UnregisterCom` (`0x89ED30`),
`BinStream::operator<<(const FixedString&)` (`0x21A020`), the inline
`BinStream::operator>>` for `int` and `unsigned int`, the
`Revision::RangeOp` template and the declaration of the map reader
`operator>>(BinStream&, eastl::map<Key, T>&)` (`0xD4A980` for
`<int, Symbol>`).

## RBSystemOptions

`RBSystemOptions` derives from `SystemOptions` and from two interfaces
without destructors that the game queries for gameplay settings. The
object is 336 bytes; `RBProfileMgr` keeps it as `mSystemOptions` at
offset 344 (`theRBProfileMgr` is the pointer at `0x1B1F7F0`), and its
`GetSystemOptions` (`0xD62560`) returns it. The save file is
`rb_system_settings.bin` (`0xD62570`).

The vtable group at `0x197E0D0`:

| Address | Offset | Slots |
| --- | --- | --- |
| `0x197E0E0` | 0 | 16: the destructors (`0xD68370`, `0xD68460`), `MakeDirty`, `MakeClean`, `IsDirty`, `Save` (`0xD667C0`), `Load` (`0xD66B70`), `Handle`, `Reset` (`0xD66500`), then the interfaces' overriders `0xD68550`-`0xD685B0` |
| `0x197E170` | 24 | `SystemOptions`' MsgSource slots, with the destructor thunks `0xD685C0`, `0xD686A0` |
| `0x197E1B8` | 216 | `IGameplayModifierOptions`: independent track speed, no fail |
| `0x197E1D8` | 224 | `IPlayerGameplayOptions`: cymbal lanes, autokick, improv solos scored, audio offset, video offset |

The primary slots 9-15 return, in order, no fail, independent track
speed, the cymbal lanes, improv solos scored, autokick and
`SystemOptions`' audio and video offsets. The game's lookup at `0x9AE5C0`
returns the session's modifier options (`+800` of its object) or the
system options'; `0x9AE5F0` returns a remote player's options (`+232` of
its user) or the system options'.

### Fields

| Offset | Field | Default | Setter | Saved as of |
| --- | --- | --- | --- | --- |
| 232 | `mBackgroundVolume` | 11 | `0xD67570` | 0 |
| 236 | `mInstrumentsVolume` | 11 | `0xD675F0` | 0 |
| 240 | `mSFXVolume` | 11 | `0xD67670` | 0 |
| 244 | `mCrowdVolume` | 11 | `0xD676F0` | 0 |
| 248 | `mDolby` | false | `0xD67870` | 0 |
| 249 | `mBassBoost` | false | `0xD677F0` | 0 |
| 250 | `mOverscanEnabled` | false | `0xD678F0` | 0 |
| 251 | `mIsDrumNavigationAllowed` | false | `0xD67970` | 5 |
| 252 | `mProDrumCymbalLanes` | `0x1C` | `0xD67AF0` | 2 |
| 256 | `mNoFail` | false | `0xD679F0` | 6 |
| 257 | `mIndependentTrackSpeed` | true | `0xD67A70` | 7 |
| 258 | `mImprovSolosScored` | true | `0xD67B70` | 8 |
| 259 | `mAwesomenessDetection` | false | `0xD67C70` | 10 |
| 260 | `mLeaderboardLadder` | true | `0xD67CF0` | 14 |
| 261 | `mAutokick` | false | `0xD681F0` | 15 |
| 262 | `mSubtitles` | false | `0xD67BF0` | 21 |
| 263 | `mHide1RatedSongs` | false | `0xD67DF0` | 19 |
| 264 | `mHideUnavailableSongs` | false | `0xD67E70` | 25 |
| 265 | `mRivalsChallengeFilter` | false | `0xD67EF0` | 26 |
| 268 | `mSongLibSort` | 0 | `0xD67D70` | 11 (kept from 20) |
| 272 | `mSongLibStatsMode` | 1 | `0xD67F70` | 28 |
| 276 | `mSongLibLeaderboardMode` | 0 | `0xD67FF0` | 28 |
| 280 | `mAutoShowLeaderboard` | true | `0xD68070` | 28 |
| 284 | `mLastSeenPatch` | 0 | `0xD680F0` | 13 |
| 288 | `mHasSeenUpsell` | false | `0xD68170` | 22 |
| 296 | `mStoredText` | "" | `0xD665E0` | 17 |
| 312 | `mStoredText2` | "" | `0xD666D0` | 17 |
| 328 | `mGraphicsMode` | 1 | `0xD68270` | 27 |
| 332 | `mHDR` | 1 | `0xD682F0` | 27 |

The volume getters (`0xD67770`-`0xD677D0`) index two 12-entry decibel
tables: `0x13139E0` (-8 to 0 dB) for the background and instruments,
`0x1313A10` (-18 to 0 dB) for the SFX and crowd. The map's
`_SliderIndexToDb` is inlined into them. `RBMetaState::Poll` applies the
SFX and crowd volumes to the FMOD VCAs.

Every setter compares, pushes the `"profile"` heap (each with its own
guarded `MemFindHeap` result), stores and calls `MakeDirty` through the
vtable. `Reset` keeps the volumes and most checkboxes and restores the
cymbal lanes, the song library's sort, filters and modes, the patch and
upsell flags and the two texts.

`Save` writes a revision-28 `RevisionStream` block: `SystemOptions`'
block, then the fields in this order (volumes, Dolby, bass boost, overscan, cymbal lanes, drum navigation, no
fail, track speed, improv, awesomeness, ladder, hide-1-rated, subtitles,
sort, patch, upsell, autokick, the two texts, hide-unavailable, rivals
filter, the stats and leaderboard modes, auto-show, graphics mode, HDR),
then `theSongMgr->SaveSongCache`. `Load` reads them back with
`Revision::AsOfOp`, clamps the volumes to 11, drops an int stored by
revisions 12 to 15 (`Revision::RangeOp`), sets improv solos scored for
revisions up to 8 and independent track speed for revisions up to 22,
reads the song cache from revision 4, and for revisions 18 to 23 reads and
drops a nested revision-2 block (a set of ints and a `map<int, Symbol>`).
It ends with `MakeClean`.

## RBUILayoutCom

"Allows screens to set state about themselves." (author Chris McCarthy,
category UI, allowed resource `UILayoutResource`). A `Component`; the
vtable at `0x1978348` has 41 slots, overriding the destructors
(`0xCB4EF0`, `0xCB4F50`), `Handle` (`0xCB54A0`), the identity slots
(`0xCB5700`-`0xCB5780`, `0xCB58E0`, `0xCB58F0`), `_Enter` (`0xCB4FC0`)
and `_Poll` (`0xCB52E0`). The object is 96 bytes.

| Offset | Field | Property |
| --- | --- | --- |
| 24 | `mRuntime.mSkippingDraw` | |
| 32 | `mRuntime.mInputCom` | |
| 40 | `mRuntime.mTransitionStartedSink` | |
| 80 | `mLayout` | `layout` (default `kLayoutTitle`) |
| 84 | `mGoBackToLayout` | `go_back_to_layout` (default `kLayoutInvalid`) |
| 88 | `mInitialFramesToSkipDraw` | `initial_frames_to_skip_draw` (0-300) |

The run-time fields sit in a `RuntimeData` struct: the binary places the
first bool at 24, not in `Component`'s tail padding.

- `_Enter` reports the layout (`RBMetaState::_SetCurrentLayout`), runs
  `_DoInputRegistration` (`0xCB5140`: the `RBUIInputCom` on the root of
  the last instanced scene of the layout resource gets the input first via
  `UIInputCom::RegisterCom`), subscribes to `theUI`'s
  `"transition_started"` as `"on_transition_started"`, and sets the layout
  resource's frames to skip drawing to the initial value, at least 10.
- `_Poll` waits for the root's `RBCharacterLayoutTrackerCom` to stop
  loading player characters, then sets ten more frames and stops waiting.
  Without a tracker it sets the ten frames again on every poll.
- `OnMsg(UIButtonDownMsg)` (`0xCB53C0`) goes to `go_back_to_layout` on
  cancel (action 2). `_OnTransitionStarted` (`0xCB5420`) unregisters the
  input (`0xCB53A0`), leaves the event, runs `SaveLoadMgr::SaveAll(false)`
  and marks the system options clean. `_SetFramesToSkipDraw` is
  `0xCB52A0`. These four are inlined into their callers and kept as
  uncalled copies.

The statics are `sId` (`0x1B0FC60`), `sClassName` (`0x1B0FC68`),
`sPropRegistry` (`0x1B0FC70`) and `sMetaData` (`0x1B0FD10`), built by
the static initializer at `0xCB5A10`.

## Weak evidence

- `IGameplayModifierOptions`, `IPlayerGameplayOptions` and their getter
  names: no RTTI; named from the fields and the two lookups.
- `mStoredText`, `mStoredText2` and their setters: nothing outside Reset
  and Load touches them.
- `mSongLibStatsMode` and `mSongLibLeaderboardMode`: only uncalled getters
  (`0xC80B90`, `0xC80BB0`) beside the song library filters read them; the
  defaults match the stats ("Global All Time" is 1) and leaderboard
  ("None") lists.
- `mHasSeenUpsell`: set after the startup upsell (`0xB56910`).
- The dropped revision 12-15 int as the map's party mode sort.
- `mRuntime.mSkippingDraw` and `_UndoInputRegistration`.

## Not reconstructed

`SongMgr` and the map reader template are declared only. `RBProfileMgr` is in [rb-meta-profile-manager.md](rb-meta-profile-manager.md).
