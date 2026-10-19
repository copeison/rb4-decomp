# Meta songs

`rb_meta/SongMgr.o` (`0xF29E40`-`0xF33742`) holds the game's song manager
`SongMgr`, the song file resolvers it installs, and `SongMountHandle`. The
source is `src/rb_meta/songs/SongMgr.{h,cpp}`. The objects it uses from
other modules are declared without definitions in `SongMetadata.h` and
`SongMetadataResource.h` (in `src/rb_meta/songs/`), and in `ContentMgr.h`,
`PlatformMgr.h` and the mogg resource headers.

`SongMgr::Init` creates the manager and installs three resolvers:

- the content file path function (`SetContentFilePathFunc`, `0x392690`);
- the mogg music file path function (`SetMoggMusicFilePathFunc`, `0x5F5D0`);
- the mogg companion path function (`SetMoggCompanionPathFunc`, `0x61010`).

As the content manager's `Callback`, the manager collects each mounted
content's `.songdta` files. For each song it records the metadata, the
content it came from and its folder. When a refresh finishes, the songs
discovered during it become the available songs. The song cache (the songs
of each content, the folders, a copy of every metadata block, and the last
100 recently acquired songs) is saved inside `RBSystemOptions` as a
revision-8 `RevisionStream` block. The metadata in that block is at
`SongMetadata::kRev` 12.

## Files

| File | Contents |
| --- | --- |
| `SongMgr.h/.cpp` | `SongMgr`, `SongMountHandle`, `SongMountCompletedMsg` |
| `SongMetadata.h` | `SongMetadata` (`SongMetadata.o`), declared only: getters, `kRev`, `mShortName` at `0x3AD`, size `0x4B8` |
| `SongMetadataResource.h` | `SongMetadataResource::Id()` and `mMetadata` (+48), declared only |

## Globals

| Address | Name | Notes |
| ---: | --- | --- |
| `0x1B43A00` | `gNullObjectId` | -1; the static initializer (`0xF33720`) sets it first |
| `0x1B43A08` | `sRootContent` | `Symbol(".")`, the game's own content (inferred) |
| `0x1B43A10` | `theSongMgr` | set by the constructor |
| `0x1B43A18` | `sContentFilter` | `SetContentFilter`'s function; the first one installed stays |
| `0x1B43A20`-`0x1B43A88` | function statics | `song_mgr`, `dirs`, `paygated_dirs`, the `.songdta` length, `song_cache_dirty`, `song_content_refreshed`, `has_song` |
| `0x133CCF0` | `kRankTierThresholds` | 7 parts x 6 floats; parts 3 and 5 are zero |
| `0x133CBC0` | `kMountTimeoutMs` | 6000 |
| `0x1ADF9F0` | `SongMountCompletedMsg::Type()`'s static | shared with other objects |

## SongMgr (648 bytes)

The class derives from `SyncMsgSource` (its lock is `mCritSec`) and from
`ContentMgr::Callback` at +32.

| Offset | Field |
| ---: | --- |
| 40 | `mCritSec` |
| 56, 112 | `mAvailableSongs`, `mDiscoveredSongs` (`eastl::set<int>`) |
| 168 | `mSongCache`: `map<int, SongMetadata*>`, allocated with MemAlloc |
| 224, 280 | `mSongIDToShortName`, `mShortNameToSongID` |
| 336 | `mSongIDToContent` (`map<int, Symbol>`) |
| 392 | `mContentSongs` (`map<Symbol, list<int>>`) |
| 448, 480 | `mContentDirs` (`list<String>`), `mPaygatedDirs` (`list<{int, String}>`), from the `song_mgr` config |
| 512 | `mSongDirs` (`map<Symbol, Symbol>`) |
| 568, 569 | `mSongCacheDirty`, `mSongCacheLoaded` |
| 576 | `mRecentlyAcquired` (`list<int>`, at most 100) |
| 608, 612 | discovered content and discovered-mounted counts |
| 616-640 | song counts by origin (other, disc, RB4, Rivals, and the same counts restricted to `HasPart(1)`) |

The vtable at `0x19910B0` has 16 slots:

- 0-1: the destructors (`0xF2CB60`, `0xF2CE00`);
- 2: `Handle` (`0xF32960`), which answers only `has_song`;
- 3: `MsgSink`'s;
- 4-6: `SyncMsgSource`'s;
- 7-15: the Callback overrides in declaration order: `ContentRefreshStarted`, `ContentDiscovered`, `ContentMounted`, `ContentFailed`, `ContentLoaded`, `ContentRefreshDone`, `ContentPattern`, `ContentDirs`, `ContentFileFound`.

The Callback's vtable at `0x1991140` has 13 slots: the thunks, plus the
base's `ContentMountBegun` and `ContentUnmounted` at slots 4 and 6. Both
built vtables match the binary slot for slot.

## SongMountHandle (72 bytes, vtable `0x19911B8`)

The class name is inferred. It derives from `MsgSink`.

| Offset | Field |
| ---: | --- |
| 8 | `mSong` |
| 16 | `mState`: 0 pending, 1 mounted, 2 idle |
| 20 | `mHoldsMountRef` |
| 24 | `mRequestMs` |
| 32 | `mSinkElem`: its subscription to `song_mount_completed` on `theSongMgr` |

## Functions

| Address | Function |
| ---: | --- |
| `0xF29E40`, `0xF2A740`, `0xF2A760` | `Init`, `Terminate`, `SetContentFilter` (inferred) |
| `0xF29E80`, `0xF2A100`, `0xF2A3B0` | file statics `SongContentFilePath`, `SongMoggMusicFilePath`, `SongMoggCompanionPath` (inferred) |
| `0xF2A780` | `GetSongFilePathWithExt` (inferred) |
| `0xF2A860`, `0xF2B3B0` | `GetSongFilePath(Symbol, ...)`, `GetSongFilePath(int, ...)` |
| `0xF2B520`-`0xF2BB10` | `GetShortNameFromSongID`, `GetSongFilePathFromResourcePath`, `ResourcePathToSongShortName`, `HasSong(Symbol)`, `GetContentName(Symbol, bool)`, `GetSongDirFromShortName` |
| `0xF2BBD0` | `_SetSongDirFromPath` |
| `0xF2C150`, `0xF2CB60` | constructor, destructor |
| `0xF2CE50`, `0xF2CEA0` | `Poll`, `HasSongCacheLoaded` |
| `0xF2CEE0`, `0xF2D0F0` | `SaveSongCache`, `operator<<` for `map<Symbol, list<int>>` |
| `0xF2D1E0`, `0xF2DA80`, `0xF2DC20` | `LoadSongCache` and its two `operator>>` |
| `0xF2DD40` | `Resource::GetOrLoad<SongMetadataResource>` (template instance) |
| `0xF2DE90`, `0xF2E160`, `0xF2E230`, `0xF2E290`, `0xF2E370` | `_UpdateFromLoadedCache`, `_SetSongCacheDirty`, `GetPosInRecentList`, `ClearSongCache`, `NumSongs` |
| `0xF2E650`-`0xF2E9D0` | `GetMetadata(int)`, `HasSong(int)`, `GetSongIDFromShortName`, `GetMetadata(Symbol)`, `GetMetadataList` (inferred) |
| `0xF2EBD0`-`0xF30000` | Callback overrides; `_AddDiscoveredSongs`, `_ReplaceAvailableSongs`, `_IncDiscoveredContentMounted`, `_SendSongMountCompletedMsg`, `_AddSongData` |
| `0xF30010`-`0xF30BB0` | `GetContentName(int)`, `IsSongEntitled` (inferred), `IsSongContentAvailable`, `MountSong` x2, `IsSongMounted` x2, `ReleaseSongMountRef` x2, `_AddSongAsRecentlyAcquired` |
| `0xF30C70`-`0xF31C20` | `GetAvailableSongSet`, `GetPlaybleSongSet`, `GetDownloadedSongSet`, `GetAvailableSongsIn`, `FilterAvailableSongs`, `GetInstalledDiscSongSet`, `GetRandomPlayableSong` (all but the first two inferred) |
| `0xF31CD0`, `0xF31D20` | `GetRankTier(const SongMetadata*, int)`, `GetRankTier(float, int)` |
| `0xF31D60`, `0xF31E70` | `_SendSongContentRefreshedMsg`, `_UpdateSongCounts` (inferred) |
| `0xF32060`-`0xF32960` | `SongMountHandle` |
| `0xF32B90`-`0xF33719` | EASTL subtree releases and key insertions, `SongMountCompletedMsg(bool, Symbol, int)` |

## Weak names

- `SongMountHandle` and its fields.
- `sRootContent`.
- The inferred set and refresh helpers listed above.
- `IsSongEntitled`.
- `RBEntitlementMgr` / `theRBEntitlementMgr` (`0x1AF4E38`, `IsEntitled` at `0xAE40C0`), declared in `SongMgr.cpp`.
- On `ContentMgr`: `IsRefreshInProgress`, `IsContentMounted` and the `Callback` slot signatures.
- `PlatformMgr::IsMarkerInstalled`, `MakePlatformPath`.
- The three path-function setters.
- The `SongMetadata` getters `IsDownload`, `IsPrivate`, `IsDiscSong`, `IsRB4Song`, `IsRivalsSong`, `GetRank(int)` and `HasPart(int)`.

## Notes

- `eastl::map`'s `operator[]` assigns over a raw node, so `SongMgr.cpp`
  inserts and releases the list and vector values of its maps itself
  (`FindOrInsert`, `ReleaseValues`).
- The `SongMgr` functions that return sets build them from vectors,
  because `eastl::set` has no copy constructor here.
- `ContentFileFound` returns the text of a local string, as the binary does.
- The map's `DumpSongMgrContents`, `ToggleFakeSongs`, `GetRankTierToken`,
  `GetNumRankTiers`, `_CalcTierRanges`, `_GetRankedSongs` and
  `IsSongUnplayable` are not in this build.
