# Meta profiles and their saved data

The game's profile from the map's `rb_meta/RBProfile.o` and the persistent
data objects it owns. Names follow [naming.md](naming.md); the persistence
base classes are in [persistence-profiles.md](persistence-profiles.md) and
saving in [persistence-saveload.md](persistence-saveload.md). The manager
that creates profiles is in
[rb-meta-profile-manager.md](rb-meta-profile-manager.md).

## Files

| Object | Range | Source | State |
| --- | --- | --- | --- |
| `rb_meta/RBProfile.o` | `0xD53430`-`0xD5B31F` | `src/rb_meta/profiles/RBProfile.cpp` | partly reconstructed |
| `rb_meta/FTUEPersistentData.o` | `0xD45270`-`0xD456EB` | `src/rb_meta/persistentdata/FTUEPersistentData.cpp` | reconstructed |
| `rb_meta/RBCampaignGigProgressPersistentData.o` | `0xD4CCA0`-`0xD4D51B` | `src/rb_meta/persistentdata/RBCampaignGigProgressPersistentData.cpp` | reconstructed |
| `rb_meta/RBCampaignCityProgressPersistentData.o` | `0xD4B660`-`0xD4CC9F` | `.../RBCampaignCityProgressPersistentData.h` | declared |
| `rb_meta/RBCampaignBandPersistentData.o` | `0xD464A0`-`0xD4B65F` | `.../RBCampaignBandPersistentData.h` | declared |
| `rb_meta/RBCharacterPersistentData.o` | `0xD4D520`-`0xD4FC1F` | `.../RBCharacterPersistentData.h` | declared |
| `rb_meta/RBGameplayOptions.o` | `0xD52AF0`-`0xD5342F` | `.../RBGameplayOptions.h` | declared |
| `rb_meta/RBRockShopPersistentData.o` | `0xD62D50`-`0xD650EF` | `.../RBRockShopPersistentData.h` | declared |
| `rb_meta/SongPersistentData.o` | from `0xD69800` | `.../SongPersistentData.h` | declared |
| `rb_meta/RBAchPersistentData.o`, `_PS4.o` | `0xF07280`, from `0xF02810` | `.../RBAchPersistentData.h`, `RBAchPersistentData_PS4.h` | declared |
| newer objects (below) | see the headers | `src/rb_meta/persistentdata/` | declared |

Each object ends with a static initializer (`0xD456B0`, `0xD4D4E0`,
`0xD52AB0`, `0xD5B2E0`, ...) that sets globals of a shared header; it is
not modelled, but it marks the object boundaries. IDA leaves most Save,
Load and deleting destructors undefined; they were read from the raw code.

## RBProfile

3360 bytes from the "profile" heap (class `operator new`, label
"RBProfile"); `RBProfileMgr::_OnUserLoggedIn` creates one per pad through
`ProfileMgr::FindOrCreateProfile<RBProfile>` (`0xBB02E0`). The vtable at
`0x197DAB8` has Profile's 14 slots (destructors `0xD53E20`/`0xD541D0`,
Save `0xD547E0`, Load `0xD54E00`, Handle `0xD59300`, Poll `0xD54220`,
CanSave `0xD547D0`, DeleteAll `0xD543A0`, GetSaveFileName `0xD59470`
"rb_profile.bin"); Profile's IsDirty and GetSaveVersion are kept. MsgSink's
vtable is at `0x197DB38` (offset 24). The built vtable matches.

| Offset | Field | Type / note |
| --- | --- | --- |
| 200 | `mCampaignBands` | `vector<RBCampaignBandPersistentData>` (768-byte bands), lock `mBandsLock` |
| 232 | `mQuickplaySongData` | `map<int, QuickplaySongPersistentData>`, saved from revision 3 |
| 288 | `mCampaignSongData` | same type, from revision 14 |
| 344, 400 | `mSongLeaderboards`, `mFriendsSongLeaderboards` | `map<int, RBSongLeaderboard>` caches |
| 456 | `mMiniLeaderboardResults` | `RBMiniLeaderboardResults`; +12 (468) `mSubmissionsStarted` |
| 1120 | `mSongRatings` | `map<int, unsigned char>` (1 to 5), from revision 20 |
| 1176, 1232 | `mSongScoreCache`, `mSongScoreCacheStamps` | per-song 56-byte entries and stamps |
| 1288 | `mGameplayOptions` | `RBGameplayOptions`, from revision 4 |
| 1496 | `mCharacterData` | `RBCharacterPersistentData`, from 5 |
| 1624 | `mRockShopData` | `RBRockShopPersistentData`, from 9 |
| 1824 | `mSeenIds` | `RBSeenIdsPersistentData`, from 19 |
| 1912 | `mSetlists` | `RBSetlistPersistentData`, from 16 |
| 1992 | `mFTUEData` | `FTUEPersistentData`, from 10 |
| 2048 | `mAchData` | `RBAchPersistentData*` (an `RBAchPersistentData_PS4`), from 12 |
| 2056 | `mAchievementLogCount` | 64-bit, saved as an int from 15 |
| 2064 | `mBTMData` | `RBBTMPersistentData`, from 18 |
| 2144 | `mClanData` | `RBClanPersistentData`, from 22 |
| 2280 | `mRivalsPlayerProfile` | `RBRivalsPlayerProfile` (712 bytes); its +80 name came from revisions 17-20 |
| 2992 | `mSompSettings` | `RBSompSettingsPersistentData`, from 24 |
| 3056 | `mUserPreferences` | `RBUserPreferencesPersistentData`, from 25 |
| 3112 | `mPlatformName` | `String`, cut to 16 characters plus "..." |
| 3128 | `mOnlineID` | `OnlineID` (account id, valid flag, name) |
| 3160 | `mSignInState` | 0 ok, 1 age, 2 patch, 3 not signed in, 5 unknown |
| 3164, 3165 | `mServerReachable`, `mSignedIntoLive` | Poll copies 3165 into 3164 |
| 3168 | `mNpRequestId` | the NP availability request |
| 3172 | `mNeedsOnlineCheck` | set by "connection_status_changed" |
| 3176 | `mEntitlements` | `set<int>` of entitled songs; ids 3-327 saved from 23 |
| 3232, 3240, 3248 | `mPlatformEntitlementsJob`, `mServerEntitlementsJob`, `mSongUploadJob` | `JobState {needed, pending, start ms}` |
| 3256-3304 | `mBandsLock`, `mSongDataLock`, `mLeaderboardLock`, `mSongScoreCacheLock` | `CritSec`s |
| 3320 | `mConnectionStatusSink` | subscription to the platform's "connection_status_changed" |

Save writes a revision-25 `RevisionStream`: the console's options
(`theRBProfileMgr->mSystemOptions`), the bands, both song maps, the
ratings, the pieces in the order of the table above (gameplay options,
SOMP settings, characters, Rock Shop, FTUE, achievements, the log count,
setlists, BTM, seen ids, clan, user preferences) and the saved
entitlements. Load drops revisions up to 12, reads the system options
(and the song cache at revision 6), reads a single band for revisions 2-7
and the band vector from 8, then each piece from the revision in the
table, re-points the pieces at the profile, and marks the profile clean
and ready to save. DeleteAll clears every piece except the FTUE events,
achievements, mini leaderboard and Rivals player profile, and asks for
both entitlement refreshes.

The three jobs: `SetNeedsEntitlementRefresh` (`0xD54650`) sets both
entitlement jobs' needed flags; the `...Pending` setters record the start
time from `TheTimeMgr`'s real time; the retry checks wait 20 s for the
entitlement jobs and 10 s for the upload.

## Pieces

- `FTUEPersistentData` (56 bytes, vtable `0x197D408`): a vector of seen
  event symbols; revision-0 block. `MarkEventSeen` always dirties.
- `RBCampaignGigProgressPersistentData` (48, `0x197D760`): venue, stars,
  display stars, highest score, attempted; revision 2 (highest score from
  1, display stars from 2). Copy constructor and assignment are emitted.
- `RBCampaignCityProgressPersistentData` (96, `0x197D6F0`): fans, cash,
  `map<Symbol, gig>` and the profile.
- `RBCampaignBandPersistentData` (768, `0x197D4F0`): `mRecentlyPlayed` at
  352; the rest is opaque.
- `RBGameplayOptions` (208, `0x197D8A0`): PersistentData plus an rb_game
  base at +24 (vtable `0x197D908`, constructor `0xA4C280`) that holds the
  option values.
- `SongPersistentData` (408, `0x197E2C8`) and
  `QuickplaySongPersistentData` (464, `0x197E318`); the base's eighth slot
  (`SetChecksum`) is newer than the map.

Newer than the map, named from their contents (weak): `RBBTMPersistentData`
(80, `0x197D4A8`), `RBClanPersistentData` (136, `0x197D7F0`),
`RBSetlistPersistentData` (80, `0x197E028`),
`RBSompSettingsPersistentData` (64, `0x197E098`),
`RBUserPreferencesPersistentData` (56, `0x197E210`),
`RBSeenIdsPersistentData` (88, `0x197E280`).

## Weak names

- Every field name of RBProfile; `GetNumBands`, `GetNumQuickplaySongs`,
  the rating, entitlement, job and seen-id methods, `_StartOnlineCheck`,
  `IncrementAchievementLogCount`, `ClearClan`, `ClearClanName`.
- The newer pieces' class names and the foreign types declared in
  `RBProfile.h` (`RBMiniLeaderboardResults`, `RBRivalsPlayerProfile`,
  `RBSongLeaderboard`) and in `RBProfile.cpp` (`DingoUserSession`,
  `dingo_find_user_session`).
- In the map the campaign song lookups take a band; this build keeps one
  campaign map in the profile, and `GetOrCreateCampaignSongData` comes
  before `GetOrCreateQuickplaySongData`.

## Not reconstructed

In RBProfile.o: the leaderboard caches (`0xD55800`-`0xD55DC0`), the score
cache (`0xD56000`-`0xD564F0`), `GetOrCreate*SongData`, `GetAllSongsPlayed`,
`GetHighestStatStars`, the song-id range helpers (`0xD58600`-`0xD58940`),
`IsSongEntitled`, the achievement update `0xD589C0`, rating increase and
decrease, `_GetSavedEntitlements` (inlined into Save here) and the
templates. All piece objects except FTUE and the gig progress are declared
only; their layouts are opaque where the profile does not reach into them.
`AddEntitlement` omits the entitlement manager's own songs (`0xB46170`).
