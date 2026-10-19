#pragma once

#include <cstddef>

#include "os/content/ContentMgr.h"
#include "os/threading/CritSec.h"
#include "utl/containers/List.h"
#include "utl/containers/Map.h"
#include "utl/containers/Set.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataArray.h"
#include "utl/data/DataNode.h"
#include "utl/messages/Message.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class BinStream;
class Resource;
class ResourcePath;
class SongMetadata;

// Sent by the song manager when a song's content mounted or failed to:
// (target "song_mount_completed" success song song_id). The constructor is
// emitted weakly in rb_meta/SongMgr.o (0xF32CF0); the vtable is at
// 0x1960310 and the destructor is Message<3>'s (0x3EE20).
class SongMountCompletedMsg : public Message<3> {
public:
    SongMountCompletedMsg(bool success, Symbol song, int songID) : Message<3>(Type()) {
        mData->Node(2) = DataNode(static_cast<int>(success));
        mData->Node(3) = DataNode(song);
        mData->Node(4) = DataNode(songID);
    }
    // Wraps an existing message array (0x8F84D0).
    explicit SongMountCompletedMsg(DataArray* msg) : Message<3>(msg) {}

    // The message type, built on first use (0x1ADF9F0).
    static Symbol Type() {
        static Symbol t;
        if (t == Symbol()) {
            t = Symbol("song_mount_completed");
        }
        return t;
    }

    // Names not in the reference map.
    bool Success() const {
        return mData->Int(2) != 0;
    }
    Symbol Song() const {
        return mData->Sym(3);
    }
    int SongID() const {
        return mData->Int(4);
    }
};

// Keeps a song's content mounted for its owner: setting a song asks the
// song manager to mount it and listens for its "song_mount_completed";
// clearing it or setting another releases the mount reference. Poll asks
// again when a mount took longer than six seconds. The UI's song previews
// and the game's song loading hold one. The vtable is at 0x19911B8. Name
// not in the reference map.
class SongMountHandle : public MsgSink {
public:
    // The state of the mount. Names not in the reference map.
    enum State : int {
        kMountPending = 0,
        kMounted = 1,
        kMountIdle = 2,
    };

    SongMountHandle();  // 0xF32060
    explicit SongMountHandle(Symbol song);  // 0xF320B0
    // Mounts the other handle's song again.
    SongMountHandle(const SongMountHandle& other);  // 0xF32380
    SongMountHandle& operator=(const SongMountHandle& other);  // 0xF323D0
    ~SongMountHandle() override;  // slots 0-1: 0xF323F0, 0xF32530
    // Slot 2: routes "song_mount_completed" to OnMsg.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xF32780

    // Releases the current song and asks for the new one; the empty symbol
    // only releases.
    void SetSong(Symbol song);  // 0xF32100
    void Clear();  // 0xF32550
    // Asks again for a song whose mount is still pending after
    // kMountTimeoutMs.
    void Poll();  // 0xF325B0
    // Records the result for the handle's song; another song's message is
    // unhandled.
    DataNode OnMsg(const SongMountCompletedMsg& msg);  // 0xF326A0

    // Field names are not in the reference map.
    Symbol mSong;
    State mState;
    // Whether MountSong took a mount reference, released with the song.
    bool mHoldsMountRef;
    // When the mount was asked for (TimeMgr's real time), or -1.
    float mRequestMs;
    MsgSource::EventSinkElem mSinkElem;
};

static_assert(offsetof(SongMountHandle, mSong) == 0x8);
static_assert(offsetof(SongMountHandle, mState) == 0x10);
static_assert(offsetof(SongMountHandle, mHoldsMountRef) == 0x14);
static_assert(offsetof(SongMountHandle, mRequestMs) == 0x18);
static_assert(offsetof(SongMountHandle, mSinkElem) == 0x20);
static_assert(sizeof(SongMountHandle) == 0x48);

// The game's song manager (rb_meta/SongMgr.o, 0xF29E40-0xF33742): the
// songs of the mounted content, their cached metadata, the content each
// came from and the folder it lives in. It receives the content manager's
// events as a ContentMgr::Callback (at +0x20) and exports
// "song_cache_dirty", "song_content_refreshed" and "song_mount_completed"
// to its sinks. The song cache is saved with the system options. The
// vtable is at 0x19910B0 (16 slots) with the Callback's at 0x1991140 (13
// slots).
class SongMgr : public SyncMsgSource, public ContentMgr::Callback {
public:
    // A content folder that needs an entitlement: "paygated_dirs" in the
    // song_mgr config. Names not in the reference map.
    struct PaygatedDir {
        int mEntitlement;
        String mDir;
    };

    SongMgr();  // 0xF2C150
    // Slots 0-1: 0xF2CB60, 0xF2CE00; the Callback's thunks are 0xF2CDF0
    // and 0xF2CE20.
    ~SongMgr() override;
    // Slot 2: answers "has_song"; everything else is unhandled.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xF32960

    // ContentMgr::Callback, in the primary vtable's slots 7-15 (the
    // Callback's thunks after the arrow).
    // Slot 7: a full refresh (2) forgets the discovered songs.
    void ContentRefreshStarted(int refreshType) override;  // 0xF2EBD0 -> 0xF2EC30
    // Slot 8: content new to the cache is mounted; known content's songs
    // are discovered from the cache.
    void ContentDiscovered(int location, Symbol content, bool& mount) override;  // 0xF2EF30 -> 0xF2F170
    void ContentMounted(Symbol content, const char* root) override;  // slot 9: 0xF2F180 -> 0xF2F3A0
    void ContentFailed(Symbol content) override;                     // slot 10: 0xF2F410 -> 0xF2F470
    // Slot 11: adds the loaded .songdta's song.
    void ContentLoaded(Symbol content, Resource* resource) override;  // 0xF2F4D0 -> 0xF2FBA0
    // Slot 12: a full refresh (2) replaces the available songs with the
    // discovered ones; a partial one (1) adds them.
    void ContentRefreshDone(int refreshType) override;  // 0xF2EC90 -> 0xF2EF20
    // Slot 13: "*/*.songdta*".
    const char* ContentPattern() override;  // 0xF2FC00 -> 0xF2FC10
    // Slot 14: the config's dirs and the paygated dirs the user owns.
    eastl::list<String> ContentDirs() override;  // 0xF2FC20 -> 0xF2FD90
    // Slot 15: cuts the path after ".songdta" and, for content not on a
    // local volume, before the first folder.
    const char* ContentFileFound(Symbol content, const char* file) override;  // 0xF2FDB0 -> 0xF30000

    // Creates the manager and installs the song file resolvers.
    static void Init();  // 0xF29E40
    static void Terminate();  // 0xF2A740
    // Installs the filter that makes the callbacks ignore content; only
    // the first one sticks. Name not in the reference map; the entitlement
    // manager (0xAE25D0) installs one.
    static void SetContentFilter(bool (*filter)(Symbol content));  // 0xF2A760

    // The path of a song's file: in its content root and song folder, else
    // beside `source`, unless a patched copy exists. The result is in a
    // MakeString buffer.
    static const char* GetSongFilePath(Symbol shortName, const char* file, const ResourcePath& source);  // 0xF2A860
    // The song's "<short name>.<ext>". The map's signature is
    // GetSongFilePath(int, char const*, ResourcePath const&).
    static const char* GetSongFilePath(int songID, const char* ext, const ResourcePath& source);  // 0xF2B3B0
    // GetSongFilePath of "<short name>.<ext>". Name not in the reference
    // map.
    static const char* GetSongFilePathWithExt(Symbol shortName, const char* ext, const ResourcePath& source);  // 0xF2A780
    // GetSongFilePath of the song whose folder holds the path.
    static const char* GetSongFilePathFromResourcePath(const ResourcePath& path, const char* ext);  // 0xF2B5B0
    // The name of the folder holding the path.
    static Symbol ResourcePathToSongShortName(const ResourcePath& path);  // 0xF2B7D0

    // The flags the map's signatures carry are unused in this build.
    Symbol GetShortNameFromSongID(int songID, bool unused) const;  // 0xF2B520
    bool HasSong(Symbol shortName) const;  // 0xF2B960
    bool HasSong(int songID) const;  // 0xF2E740
    Symbol GetContentName(Symbol shortName, bool unused) const;  // 0xF2BA70
    Symbol GetContentName(int songID) const;  // 0xF30010
    Symbol GetSongDirFromShortName(Symbol shortName);  // 0xF2BB10
    int GetSongIDFromShortName(Symbol shortName, bool unused) const;  // 0xF2E7B0
    SongMetadata* GetMetadata(int songID, bool unused) const;  // 0xF2E650
    SongMetadata* GetMetadata(Symbol shortName, bool unused) const;  // 0xF2E840
    // The metadata of each song, in order. Name not in the reference map.
    eastl::vector<SongMetadata*> GetMetadataList(const eastl::vector<Symbol>& shortNames, bool unused) const;  // 0xF2E9D0
    // The available songs; downloads only unless `includeDiscSongs`,
    // without tutorials unless `includeTutorials`. Parameter names weak.
    int NumSongs(bool includeDiscSongs, bool includeTutorials) const;  // 0xF2E370
    // The song's place in the recently acquired songs, or -1.
    int GetPosInRecentList(int songID);  // 0xF2E230
    // Whether the song's folder is entitled, when it is a paygated folder.
    // Name not in the reference map.
    bool IsSongEntitled(Symbol shortName) const;  // 0xF30150
    bool IsSongContentAvailable(int songID) const;  // 0xF30240

    // Asks for the song's content to be mounted; `mounted` reports a
    // mount reference taken. A song without content sends a failed
    // "song_mount_completed" at once. The map's signatures have a second
    // bool&.
    void MountSong(int songID, bool& mounted);  // 0xF30470
    void MountSong(Symbol shortName, bool& mounted);  // 0xF30500
    // True for songs without content.
    bool IsSongMounted(int songID) const;  // 0xF30760
    bool IsSongMounted(Symbol shortName) const;  // 0xF308B0
    void ReleaseSongMountRef(int songID) const;  // 0xF30980
    void ReleaseSongMountRef(Symbol shortName) const;  // 0xF30AE0

    const eastl::set<int>& GetAvailableSongSet() const;  // 0xF30C70
    // The available songs that are not private and are downloads or have
    // their marker installed.
    eastl::set<int> GetPlaybleSongSet() const;  // 0xF30C80
    // Names not in the reference map. The available downloads.
    eastl::set<int> GetDownloadedSongSet() const;  // 0xF30F50
    // The available disc songs whose marker is installed.
    eastl::set<int> GetInstalledDiscSongSet() const;  // 0xF31930
    // The available, non-private songs among `songs`, walking the
    // available songs and walking `songs`.
    eastl::set<int> GetAvailableSongsIn(const eastl::set<int>& songs) const;  // 0xF31200
    eastl::set<int> FilterAvailableSongs(const eastl::set<int>& songs) const;  // 0xF31550
    // A random playable song, or 0.
    int GetRandomPlayableSong() const;  // 0xF31C20
    // The tier (0-6) of a rank for the part (0-6), from the rank tables.
    // The map's signature is GetRankTier(float, Symbol) const.
    int GetRankTier(float rank, int part) const;  // 0xF31D20
    // The tier of the song's rank for the part. Name not in the reference
    // map.
    int GetRankTier(const SongMetadata* metadata, int part) const;  // 0xF31CD0

    // Marks the cache loaded once a profile is ready to save.
    void Poll();  // 0xF2CE50
    // Whether any profile is ready to save.
    bool HasSongCacheLoaded() const;  // 0xF2CEA0
    // Writes the cache as a revision-8 block: the songs of each content,
    // the song folders, the metadata and the recently acquired songs.
    void SaveSongCache(BinStream& stream);  // 0xF2CEE0
    // Reads the block SaveSongCache writes and adds the songs it does not
    // have yet.
    void LoadSongCache(BinStream& stream);  // 0xF2D1E0
    // Forgets the songs of each content and frees the metadata.
    void ClearSongCache();  // 0xF2E290

    // Exports "song_cache_dirty".
    void _SetSongCacheDirty();  // 0xF2E160
    // Records the song folder of a .songdta path: its grandparent folder
    // (its name only for a local path).
    void _SetSongDirFromPath(ResourcePath path);  // 0xF2BBD0
    // Adds a song of the content, unless it is discovered already.
    void _AddSongData(SongMetadata* metadata, Symbol content);  // 0xF2F530
    // Adds the loaded songs the cache does not have, and the folders.
    void _UpdateFromLoadedCache(
        const eastl::map<Symbol, eastl::vector<int>>& contentSongs,
        const eastl::map<int, SongMetadata*>& metadata,
        const eastl::map<Symbol, Symbol>& songDirs);  // 0xF2DE90
    // Keeps the last kMaxRecentSongs songs. The metadata is unused in this
    // build.
    void _AddSongAsRecentlyAcquired(int songID, const SongMetadata* metadata);  // 0xF30BB0
    void _IncDiscoveredContentMounted(Symbol content);  // 0xF2F130
    // Sends "song_mount_completed" for each song of the content.
    void _SendSongMountCompletedMsg(Symbol content, bool success);  // 0xF2F1E0
    // Names not in the reference map. The available songs become the
    // discovered ones (an uncalled copy; ContentRefreshDone inlines it).
    void _ReplaceAvailableSongs();  // 0xF2EE70
    void _AddDiscoveredSongs();  // 0xF2ED60
    // Forgets the discovered songs and exports "song_content_refreshed".
    void _SendSongContentRefreshedMsg();  // 0xF31D60
    // Counts the available songs by origin.
    void _UpdateSongCounts();  // 0xF31E70

    // How many recently acquired songs are kept. Name not in the reference
    // map.
    static constexpr unsigned long kMaxRecentSongs = 100;

    // Field names are not in the reference map.
    CritSec mCritSec;  // SyncMsgSource's lock.
    // The songs of the mounted content, and those the running refresh
    // found.
    eastl::set<int> mAvailableSongs;
    eastl::set<int> mDiscoveredSongs;
    // The cached metadata of every song seen, from MemAlloc.
    eastl::map<int, SongMetadata*> mSongCache;
    eastl::map<int, Symbol> mSongIDToShortName;
    eastl::map<Symbol, int> mShortNameToSongID;
    eastl::map<int, Symbol> mSongIDToContent;
    eastl::map<Symbol, eastl::list<int>> mContentSongs;
    // The song_mgr config's "dirs" and "paygated_dirs".
    eastl::list<String> mContentDirs;
    eastl::list<PaygatedDir> mPaygatedDirs;
    // The folder of each song, by short name.
    eastl::map<Symbol, Symbol> mSongDirs;
    bool mSongCacheDirty;
    bool mSongCacheLoaded;
    eastl::list<int> mRecentlyAcquired;
    int mNumDiscoveredContent;
    int mNumDiscoveredContentMounted;
    // The available songs by origin: other games, the disc (RB4 and
    // Rivals), and those with the second part bit (HasPart(1)).
    int mNumOtherSongs;
    int mNumDiscSongs;
    int mNumRB4Songs;
    int mNumRivalsSongs;
    int mNumDiscSongsWithPart;
    int mNumRB4SongsWithPart;
    int mNumRivalsSongsWithPart;
};

static_assert(sizeof(SongMgr::PaygatedDir) == 24);
static_assert(offsetof(SongMgr, mCritSec) == 40);
static_assert(offsetof(SongMgr, mAvailableSongs) == 56);
static_assert(offsetof(SongMgr, mDiscoveredSongs) == 112);
static_assert(offsetof(SongMgr, mSongCache) == 168);
static_assert(offsetof(SongMgr, mSongIDToShortName) == 224);
static_assert(offsetof(SongMgr, mShortNameToSongID) == 280);
static_assert(offsetof(SongMgr, mSongIDToContent) == 336);
static_assert(offsetof(SongMgr, mContentSongs) == 392);
static_assert(offsetof(SongMgr, mContentDirs) == 448);
static_assert(offsetof(SongMgr, mPaygatedDirs) == 480);
static_assert(offsetof(SongMgr, mSongDirs) == 512);
static_assert(offsetof(SongMgr, mSongCacheDirty) == 568);
static_assert(offsetof(SongMgr, mSongCacheLoaded) == 569);
static_assert(offsetof(SongMgr, mRecentlyAcquired) == 576);
static_assert(offsetof(SongMgr, mNumDiscoveredContent) == 608);
static_assert(offsetof(SongMgr, mNumDiscoveredContentMounted) == 612);
static_assert(offsetof(SongMgr, mNumOtherSongs) == 616);
static_assert(offsetof(SongMgr, mNumRivalsSongsWithPart) == 640);
static_assert(sizeof(SongMgr) == 648);

// The manager, set by the constructor.
extern SongMgr* theSongMgr;  // 0x1B43A10
