# Common phrases

The band's common phrase capturer. Names follow [naming.md](naming.md).

## Files

| Object | Range | Source |
| --- | --- | --- |
| `rb_game/CommonPhraseCapturer.o` | `0xA41BC0`-`0xA43090` | `src/rb_game/phrases/CommonPhraseCapturer.{h,cpp}` |
| `rb_game/CrowdRating.o` | `0xA430A0`-`0xA43EE8` | not reconstructed |

## CommonPhraseCapturer

Not polymorphic, 48 bytes: a `vector<PhraseState>` (12 bytes each: the
outcome, the tracks that hit the last gem, the tracks that failed), the
tracks that take no part, the tracks that ended the current unison phrase,
the current unison phrase and two flags. The band keeps it (Band +0xC8,
`Game::GetBand` `0xA48570`); the game applies the remote players' hits and
failures through `LocalHitLastGem` and `LocalFail` (`0x9AE350`,
`0x9AE3B0`). Every method has its map name. The vector's insertion at
`0xA42F80` is the template instance.

Reconstructed: the constructor, `ExtendPhraseStates`, `Reset`,
`ComputeTracks`, `IsUnisonPhrase`, `_CheckUnisonPhraseCompleted`,
`IsMultiplayerPhrase`, `_HasPlayedWholePhrase`, `LocalHitLastGem`,
`OneTrackCompletedPhrase`, `AllTracksCompletedPhrase`, `LocalFail`,
`GetPhraseStatus` and `DidTrackFail`.

Declared only: `HandlePhraseNote` (`0xA41DE0`), `HitLastGem` (`0xA424D0`),
`Fail` (`0xA42550`), `HandleVocalPhrase` (`0xA425D0`) and `Enabled`
(`0xA42E80`). They go through the player's controller (RBPlayer +0x4F8,
vtable `0x19A8B30` for a local player, created at `0x1236E00`): slot 8
sends a phrase's last gem hit, slot 9 its failure, slot 22 asks whether
the player's user is local (`BandUser` slot 24). The controller's 26 slots
are not identified, so the class (`RBPlayerController`, inferred name) is
only forward-declared. The bodies are recorded in the header comments.

## Declarations added

- `rb_game/song/SongDB.h`: `GetGems`, `GetNumTracks`, `GetPhraseID`,
  `IsUnisonPhrase`, `GetCommonPhraseTracks`, `GetCommonPhraseID` and
  `theSongDB` (`0x1AEFCE0`).
- `rb_game/game/Game.h`: `Game` (a `MsgSource`, weak), `GetActivePlayers`,
  `GetPlayerFromTrack`, `OvershellSetPaused`, `theGame`, `theMutableGame`.
  `RBOvershellCom.cpp` declares the same class locally and can include this
  header instead.
- `rb_game/players/RBPlayer.h`: `RBPlayer` is a `SyncMsgSource` (vtable
  `0x1968DE8`); the controller, user, track type, player state and track
  fields; `EnergyPhraseHit`, `EnergyPhraseMiss`, `UnisonPhraseMiss` (weak),
  `SoloUnisonPhraseHit`, `BandUnisonPhraseHit`.
- `rb_game/players/RBGemPlayer.h`: `HasHitGem` (`0xA56290`).
- `rb_beatmatch/gems/GameGem.h`: `PlayableBy` (`0x91C350`).

## CrowdRating

The next object (`0xA430A0` constructor `CrowdRating(TrackType,
Difficulty)`, created by the `RBPlayer` constructor at +0x4E8) is pure
arithmetic over its thresholds (`0xA43450`-`0xA437E0`) and `_Configure`
(`0xA43800`); it is not reconstructed yet.
