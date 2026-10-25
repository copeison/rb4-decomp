# Meta profiles and their saved data

The game's profile from the map's `rb_meta/RBProfile.o` and the persistent
data objects it owns. Names follow [naming.md](naming.md); the persistence
base classes are in [persistence-profiles.md](persistence-profiles.md) and
saving in [persistence-saveload.md](persistence-saveload.md). The manager
that creates profiles is in
[rb-meta-profile-manager.md](rb-meta-profile-manager.md).

## Files

| Object | Range | Source (`src/rb_meta/...`) |
| --- | --- | --- |
| `rb_meta/RBProfile.o` | `0xD53430`-`0xD5B31F` | `profiles/RBProfile.cpp` |
| `rb_meta/FTUEPersistentData.o` | `0xD45270`-`0xD456EB` | `persistentdata/FTUEPersistentData.cpp` |
| (RBBTMBandPersistentData, declared) | `0xD456F0`-`0xD4600F` | `persistentdata/RBBTMBandPersistentData.h` |
| (RBBTMPersistentData) | `0xD46010`-`0xD4649B` | `persistentdata/RBBTMPersistentData.cpp` |
| `rb_meta/RBCampaignBandPersistentData.o` | `0xD464A0`-`0xD4B65F` | `persistentdata/RBCampaignBandPersistentData.cpp` |
| `rb_meta/RBCampaignCityProgressPersistentData.o` | `0xD4B660`-`0xD4CC9F` | `persistentdata/RBCampaignCityProgressPersistentData.cpp` |
| `rb_meta/RBCampaignGigProgressPersistentData.o` | `0xD4CCA0`-`0xD4D51B` | `persistentdata/RBCampaignGigProgressPersistentData.cpp` |
| `rb_meta/RBCharacterPersistentData.o` | `0xD4D520`-`0xD4FC1F` | `persistentdata/RBCharacterPersistentData.cpp` |
| (RBClanPersistentData) | `0xD4FC20`-`0xD52AEB` | `persistentdata/RBClanPersistentData.cpp` |
| `rb_meta/RBGameplayOptions.o` | `0xD52AF0`-`0xD5342F` | `persistentdata/RBGameplayOptions.cpp` |
| `rb_meta/RBRockShopPersistentData.o` | `0xD62D50`-`0xD650EF` | `persistentdata/RBRockShopPersistentData.cpp` |
| (RBSetlistPersistentData) | `0xD650F0`-`0xD65CAF` | `persistentdata/RBSetlistPersistentData.cpp` |
| (RBSompSettingsPersistentData) | `0xD65CB0`-`0xD663EF` | `persistentdata/RBSompSettingsPersistentData.cpp` |
| (RBUserPreferencesPersistentData) | `0xD68800`-`0xD6907F` | `persistentdata/RBUserPreferencesPersistentData.cpp` |
| (RBSeenIdsPersistentData) | `0xD69080`-`0xD697EB` | `persistentdata/RBSeenIdsPersistentData.cpp` |
| `rb_meta/SongPersistentData.o` | `0xD69800`-`0xD6DD6F` | `persistentdata/SongPersistentData.cpp` |
| `rb_meta/RBAchPersistentData_PS4.o` | `0xF02810`-`0xF0498B` | `persistentdata/RBAchPersistentData_PS4.cpp` |
| `rb_meta/RBAchPersistentData.o` | `0xF07280`-`0xF073DB` (after RBAchUtl.o) | `persistentdata/RBAchPersistentData.cpp` |
| (RBSongLeaderboard) | `0xB515E0`-`0xB530FB` | `leaderboards/RBSongLeaderboard.cpp` |
| (RBMiniLeaderboardResults) | `0xCB9FE0`-`0xCBD7AF` | `leaderboards/RBMiniLeaderboardResults.cpp` |

Objects in parentheses are newer than the map; their class names are
inferred (weak). `RBAchUtl.h` declares `RBAch::SonglistProficiency` and
`RBAch::CampaignData`; `entitlements/RBEntitlementMgr.h` the entitlement
manager.

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

| Class | Bytes | Vtable | Revision | Contents |
| --- | ---: | --- | ---: | --- |
| `FTUEPersistentData` | 56 | `0x197D408` | 0 | seen event symbols; `MarkEventSeen` always dirties |
| `RBBTMPersistentData` | 80 | `0x197D498` | 0 | playthroughs started and ended, the best playthrough (a 40-byte rb_game record, vtable `0x1963298`) |
| `RBCampaignBandPersistentData` | 768 | `0x197D4F0` | 19 | name, cities, tours, fans and cash per tour, the cities' progress, session musicians, songs and songs played, pending fixes, BTM band data; Load converts revisions 8-17 |
| `RBCampaignCityProgressPersistentData` | 96 | `0x197D6F0` | 0 | fans, cash, `map<Symbol, gig>` |
| `RBCampaignGigProgressPersistentData` | 48 | `0x197D760` | 2 | venue, stars, display stars, highest score, attempted |
| `RBCharacterPersistentData` | 128 | `0x197D7A8` | 4 | at most 16 `RBChar::RBCharDesc` (3984 bytes), the saved character, the users' characters under a lock |
| `RBClanPersistentData` | 136 | `0x197D7F0` | 6 | cached clan points and tiers, Band Wars spotlight scores per song (layouts changed at 5 and 6) |
| `RBGameplayOptions` | 208 | `0x197D8A0`, `0x197D908` | 11 | `PersistentData` plus rb_game's `GameplayOptions` |
| `RBRockShopPersistentData` | 200 | `0x197DF90` | 3 | unlocked, owned and unviewed piece short names (`set<Symbol>`) |
| `RBSetlistPersistentData` | 80 | `0x197E028` | 0 | `vector<RBSetlist>` (96-byte rb_game records) under a lock |
| `RBSompSettingsPersistentData` | 64 | `0x197E098` | 1 | the SOMP session settings (40 bytes) |
| `RBUserPreferencesPersistentData` | 56 | `0x197E210` | 1 | a symbol, a value and four flags a band user copies |
| `RBSeenIdsPersistentData` | 88 | `0x197E280` | - | `set<Symbol>` |
| `SongPersistentData` | 408 | `0x197E2C8` | 8 | scores, stars, accuracies and eligibility per score type and difficulty, play count, last played |
| `QuickplaySongPersistentData` | 464 | `0x197E318` | 3 | adds the song's 56-byte checksum |
| `RBAchPersistentData` | 40 | `0x198F8C8`, `0x198F918` | - | PersistentData plus the achievement interface (`IRBAchievementLogger`, 31 slots) at +24 |
| `RBAchPersistentData_PS4` | 512 | `0x198F558` | 6 | trophy context and handle, proficiency and campaign data, counters, a threaded trophy unlocker (62 trophies) |

`RBSongLeaderboard` (96 bytes, vtable `0x196F568`) keeps a board of 56-byte
rows per score type; `RBMiniLeaderboardResults` (664, `0x1978960`) the
pad's score submission results.

The profile's remaining functions are reconstructed: the leaderboard
caches (created on first use under `mLeaderboardLock`, and
`GetLeaderboardScore`, the profile's own row), the song library's score
cache (`mSongScoreCache` holds leaderboard rows), `GetOrCreate*SongData`,
the played-song sets, `GetHighestStatStars`, `IsSongEntitled` (the profile's
entitlements, then `RBEntitlementMgr::IsSongEntitled`), the entitled-song
sets over ids 1-327, `UpdateSongPackAchievements` (entitlements 15-17) and
the ratings (1 to 5, dropped at 0).

## Weak names

- Every field name of RBProfile; `GetNumBands`, `GetNumQuickplaySongs`,
  the rating, entitlement, job and seen-id methods, `_StartOnlineCheck`,
  `IncrementAchievementLogCount`, `ClearClan`, `ClearClanName`.
- The newer pieces' class names, their records' names, the leaderboard
  classes, `RBRivalsPlayerProfile`, `IRBAchievementLogger`,
  `RBEntitlementMgr` and `DingoUserSession`/`dingo_find_user_session`.
- The profile's newer helpers: the score cache, `GetLeaderboardScore`,
  `GetMoreScoredType`, the played-song and entitled-song sets and
  `UpdateSongPackAchievements`; that the score cache's rows are leaderboard
  rows.
- In the map the campaign song lookups take a band; this build keeps one
  campaign map in the profile, and `GetOrCreateCampaignSongData` comes
  before `GetOrCreateQuickplaySongData`.

## Not reconstructed

Declared only, because they call classes that are not modelled:

- Character: `UpdateCharacter`, `CheckCharUsage`, `_IsUserUsingChar`
  (RBCampaignMgr and RBBandUser's character members `0x9AA8E0`-`0x9AA930`).
- Song data: `Update` (`0xD6B320`; RBPlayer, Game, the band, the game data).
- Gameplay options: `_OnVocalStemVolumeChanged` (the game's song audio,
  `theMutableGame` +0xB0, slot 37).
- Achievements: `LogMidSongStats`, `LogFinalizedStats`,
  `PushPendingAchievements`, `_UnlockTrophy`, `_TryPresentPendingTrophies`,
  `_CanPresentTrophyNow` (the game's stats, `Game::GetActivePlayers` at
  `0xA45F40` and the players' state at +0x510).
- Leaderboards: `RequestLeaderboard`, `OnLeaderboardJobComplete` and
  `OnScoreSubmissionsComplete` (the DingoJob server jobs; the leaderboard job
  keeps its score type at +0x220, song at +0x224, user at +0x230, friends
  flag at +0x250 and the reply's `Json::Value` at +0x1C0).

Reconstructed since: the campaign band's song-map reader (`0xD4AF20`,
which loads each `SongPersistentData` in place), `_AwardOutfitPiece` (the
gender swap's unviewed mark goes to the piece itself, and an award calls
`LogRockShopItemAwarded`), `_IsAutokickInBand` (`theBandUserMgr`'s slot 1),
`_OnMicVolumeChanged` (`TheMicChannelMgr`'s channel `SetVolume`) and
`RBSongLeaderboard::Entry::Read`, with JsonCpp's `Json::Value` declared in
`src/net/json/jsoncpp.h` (rank as `asUInt64`, score as `asInt64`, the
rest as `asInt`, "is_you" through `get` with `Value::null`).
- The rb_game records the pieces embed (the BTM record, `RBSetlist`, the
  SOMP settings) are declared in the pieces' headers.

`AddEntitlement` omits the entitlement manager's own songs (`0xB46170`).
