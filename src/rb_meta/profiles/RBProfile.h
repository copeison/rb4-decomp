#pragma once

#include <cstddef>

#include "net/OnlineID.h"
#include "os/memory/MemMgr.h"
#include "os/platform/PlatformMgr.h"
#include "os/threading/CritSec.h"
#include "persistence/profiles/Profile.h"
#include "rb_meta/persistentdata/FTUEPersistentData.h"
#include "rb_meta/persistentdata/RBBTMPersistentData.h"
#include "rb_meta/persistentdata/RBCampaignBandPersistentData.h"
#include "rb_meta/persistentdata/RBCharacterPersistentData.h"
#include "rb_meta/persistentdata/RBClanPersistentData.h"
#include "rb_meta/persistentdata/RBGameplayOptions.h"
#include "rb_meta/persistentdata/RBRockShopPersistentData.h"
#include "rb_meta/persistentdata/RBSeenIdsPersistentData.h"
#include "rb_meta/persistentdata/RBSetlistPersistentData.h"
#include "rb_meta/persistentdata/RBSompSettingsPersistentData.h"
#include "rb_meta/persistentdata/RBUserPreferencesPersistentData.h"
#include "rb_meta/persistentdata/SongPersistentData.h"
#include "utl/containers/Map.h"
#include "utl/containers/Set.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class BinStream;
class ConnectionStatusChangedMsg;
class DataArray;
class RBAchPersistentData;
class ResourcePath;

// ---------------------------------------------------------------------------
// Types of other modules that the profile embeds. They are declared here, in
// the profile's header, because their owners have not declared them; only
// the layout and the members the profile calls are given. Names not in the
// reference map; they are inferred from the code that uses them.

// The results of a pad's recent score submissions for the mini
// leaderboards (rb_meta code at 0xCB9FE0-0xCBD7AF, after RBUpsellScreen): a
// MsgSink that follows "score_submissions_started" and
// "score_submissions_complete". The vtable is at 0x1978970; the object is
// 664 bytes. The evidence for the name is weak.
class RBMiniLeaderboardResults : public MsgSink {
public:
    explicit RBMiniLeaderboardResults(int padNum);  // 0xCBA1E0
    ~RBMiniLeaderboardResults() override;           // 0xCBA3C0
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xCBD2B0
    // Forgets the results.
    void ClearResults();  // 0xCBA5B0

    int mPadNum;
    // Set by "score_submissions_started".
    bool mSubmissionsStarted;
    // The results, their lock and the two subscriptions; not reconstructed.
    unsigned char mResults[664 - 16];
};

static_assert(offsetof(RBMiniLeaderboardResults, mPadNum) == 8);
static_assert(offsetof(RBMiniLeaderboardResults, mSubmissionsStarted) == 12);
static_assert(sizeof(RBMiniLeaderboardResults) == 664);

// A pad's Rivals player profile from the server: experience, level, clan
// and invitations (the clans code at 0xB24DF0-0xB27000, which runs the
// "player_profile_job_complete" job). A MsgSink; the vtable is at
// 0x196F348 and the object is 712 bytes. The evidence for the name is weak.
class RBRivalsPlayerProfile : public MsgSink {
public:
    RBRivalsPlayerProfile(RBProfile* profile, int padNum);  // 0xB24DF0
    ~RBRivalsPlayerProfile() override;                      // 0xB25460
    DataNode Handle(DataArray* msg, bool warn) override;    // 0xB261A0
    // Clears the clan membership. Name not in the reference map; the
    // evidence for it is weak.
    void ClearClan();  // 0xB25610

    // The pad (+8), the profile (+16) and the player's state; not
    // reconstructed. +80 holds a name that profiles saved before revision
    // 21 stored.
    unsigned char mPlayerState[712 - 8];
};

static_assert(sizeof(RBRivalsPlayerProfile) == 712);

// A song's leaderboard as the profile caches it (the fail screen's code at
// 0xB517C0-0xB51C80): the entries around the player and the player's own,
// for the global board or the friends' board. The vtable is at 0x196F588;
// the object is 96 bytes. The evidence for the name is weak.
class RBSongLeaderboard {
public:
    RBSongLeaderboard(RBProfile* profile, int songId, bool friends);  // 0xB517C0
    virtual ~RBSongLeaderboard();                                    // 0xB51A50

    // The song, the entries and the request state; not reconstructed.
    unsigned char mEntries[96 - 8];
};

static_assert(sizeof(RBSongLeaderboard) == 96);

// ---------------------------------------------------------------------------

// The game's profile (rb_meta/RBProfile.o, 0xD53430-0xD5B31F with the
// static initializer at 0xD5B2E0): a pad's saved progress, built on
// Profile. It owns the campaign bands, the quickplay and campaign song
// results, the gameplay options, the characters, the Rock Shop pieces, the
// first-time-user events, the achievements and the Rivals data, and saves
// them in "rb_profile.bin" as a revision-25 block after the console's
// options. It also tracks, for RBProfileMgr, the pad's online state, the
// songs it is entitled to and three server jobs: the platform and server
// entitlement refreshes and the songs upload. Four locks guard the bands,
// the song results, the leaderboard caches and the score caches against
// the worker threads.
//
// RBProfileMgr creates one per signed-in pad. The vtable at 0x197DAB8 has
// Profile's 14 slots, with MsgSink's at 0x197DB38 (offset 24); the profile
// adds no virtual methods. The object is 3360 bytes from the "profile"
// heap.
class RBProfile : public Profile {
public:
    // A server job's state. Name and field names not in the reference map.
    struct JobState {
        // The job should run.
        bool mNeeded;
        // The job is running.
        bool mPending;
        // When it started, in milliseconds, or -1.
        float mStartTime;
    };

    // The revision Save writes. Name not in the reference map.
    static constexpr int kRevision = 25;

    // The profile comes from the "profile" heap.
    static void* operator new(unsigned long size) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        void* const allocation = MemAlloc(size, "RBProfile", 0);
        MemPopHeap();
        return allocation;
    }
    DELETE_OVERLOAD

    explicit RBProfile(int padNum);  // 0xD53430
    // Slots 0-1: 0xD53E20, 0xD541D0; MsgSink thunks 0xD541C0, 0xD541F0.
    // Clears the data, deletes the achievements and aborts the network
    // check.
    ~RBProfile() override;

    // Slot 5: the console's options, the bands, the song results, the song
    // ratings, every piece and the entitlements worth keeping, in a
    // revision-25 block.
    void Save(BinStream& stream) const override;  // 0xD547E0
    // Slot 6: reads what the stored revision has, then marks the profile
    // clean and ready to save.
    void Load(BinStream& stream) override;  // 0xD54E00
    // Slot 7 (MsgSink thunk 0xD59440): "connection_status_changed", then
    // Profile's messages.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xD59300
    // Slot 8: finishes the network availability check and restarts it
    // when asked.
    void Poll() override;  // 0xD54220
    // Slot 9: whether the storage can save.
    bool CanSave() const override;  // 0xD547D0
    // Slot 10: clears every piece of data.
    void DeleteAll() override;  // 0xD543A0
    // Slot 12: "rb_profile.bin".
    const char* GetSaveFileName() const override;  // 0xD59470

    // Reads the pad's online id and starts the network availability check.
    // The constructor inlines it; this copy has no caller. Name not in the
    // reference map.
    void _StartOnlineCheck();  // 0xD53D70
    // "connection_status_changed": asks Poll to check again. Name not in the
    // reference map.
    DataNode OnMsg(const ConnectionStatusChangedMsg& msg);  // 0xD59220

    // The pad's user name, or the localized "Guest" for the guest profile.
    const char* GetPlatformName() const;  // 0xD54760
    // The token of the reason the pad cannot go online, or "". Name not in
    // the reference map.
    Symbol GetSignInErrorToken() const;  // 0xD580C0

    // The song score cache's lock. Names not in the reference map.
    void LockSongScoreCache();    // 0xD55FB0
    void UnlockSongScoreCache();  // 0xD55FE0

    // Points the pieces back at the profile after a load.
    void _PreparePersistentData();  // 0xD556F0

    // The campaign bands. A band is addressed by its index.
    RBCampaignBandPersistentData* CreateNewBand();  // 0xD56660
    RBCampaignBandPersistentData* GetBandCampaignProgressData(unsigned long band);  // 0xD567A0
    const RBCampaignBandPersistentData* GetBandCampaignProgressData(unsigned long band) const;  // 0xD56830
    void DeleteCampaignProgress(unsigned long band);  // 0xD568C0
    // Name not in the reference map.
    unsigned long GetNumBands() const;  // 0xD569C0
    void MarkBandAsRecentlyPlayed(unsigned long band);  // 0xD56A20
    // The band played last and its index, or null and -1.
    RBCampaignBandPersistentData* GetMostRecentlyPlayedBand(unsigned long& band) const;  // 0xD56AF0

    // The song results. The map's campaign lookups take a band; this build
    // keeps one map for the profile. A symbol names the song.
    QuickplaySongPersistentData* GetQuickplaySongData(int songId);  // 0xD56B90
    QuickplaySongPersistentData* GetQuickplaySongData(Symbol song);  // 0xD56C60
    QuickplaySongPersistentData* GetCampaignSongData(int songId);  // 0xD56D40
    QuickplaySongPersistentData* GetCampaignSongData(Symbol song);  // 0xD56E10
    QuickplaySongPersistentData* GetOrCreateCampaignSongData(Symbol song);  // 0xD56EF0
    QuickplaySongPersistentData* GetOrCreateQuickplaySongData(Symbol song);  // 0xD57250
    // Name not in the reference map.
    unsigned long GetNumQuickplaySongs() const;  // 0xD575B0
    // The sum of the quickplay songs' scores, without the songs the song
    // manager no longer lists as playable.
    int GetQuickplayCareerScore() const;  // 0xD57FF0

    // Song ratings, kept as counts from 1 to 5. Names not in the reference
    // map.
    unsigned char GetSongRating(int songId) const;  // 0xD58D70
    bool IncreaseSongRating(int songId);  // 0xD58DF0
    bool DecreaseSongRating(int songId);  // 0xD58F90

    // The songs the pad owns, which the entitlement jobs fill. Names not in
    // the reference map.
    void ClearEntitlements();  // 0xD582B0
    void AddEntitlement(int songId);  // 0xD58310
    void AddEntitlements(const eastl::set<int>& songIds);  // 0xD58510
    bool HasEntitlement(int songId) const;  // 0xD58210

    // The server jobs' state. Names not in the reference map.
    void SetNeedsEntitlementRefresh(bool needed);  // 0xD54650
    void SetPlatformEntitlementsPending(bool pending);  // 0xD54660
    void SetServerEntitlementsPending(bool pending);  // 0xD546E0
    void SetNeedsPlatformEntitlements(bool needed);  // 0xD585E0
    void SetNeedsServerEntitlements(bool needed);  // 0xD585F0
    bool IsEntitlementRetryWaiting() const;  // 0xD58BE0
    void SetSongsUploaded(bool uploaded);  // 0xD58C70
    void SetSongUploadPending(bool pending);  // 0xD58C80
    bool IsSongUploadRetryWaiting() const;  // 0xD58D00

    // Counts an achievement event. Name not in the reference map; the
    // evidence for it is weak.
    void IncrementAchievementLogCount();  // 0xD58150

    // The Rock Shop's pieces.
    bool OutfitPieceIsUnlocked(const ResourcePath& piece) const;  // 0xD590C0
    bool OutfitPieceIsOwned(const ResourcePath& piece) const;     // 0xD590D0
    void UnlockOutfitPiece(const ResourcePath& piece);            // 0xD590E0
    void AwardOutfitPiece(const ResourcePath& piece);             // 0xD590F0
    void PurchaseOutfitPiece(const ResourcePath& piece);          // 0xD59100
    // The marked ids. Names not in the reference map.
    bool HasSeenId(unsigned long id) const;  // 0xD59110
    bool MarkIdSeen(unsigned long id);       // 0xD59120
    void ClearSeenIds();                     // 0xD59130
    // The Rivals clan membership. Names not in the reference map; the
    // evidence for them is weak.
    void ClearClan();      // 0xD59170
    void ClearClanName();  // 0xD591A0

    // Declared only: these read the song manager, the entitlement manager,
    // the achievements or the leaderboard caches in ways not reconstructed.
    // Names not in the reference map unless the map has them.
    eastl::set<int> _GetSavedEntitlements() const;  // 0xD54C50
    RBSongLeaderboard* GetSongLeaderboard(int songId);  // 0xD55800
    RBSongLeaderboard* GetSongLeaderboard(Symbol song);  // 0xD55AB0
    RBSongLeaderboard* GetFriendsSongLeaderboard(int songId);  // 0xD55AE0
    RBSongLeaderboard* GetFriendsSongLeaderboard(Symbol song);  // 0xD55D90
    int GetLeaderboardRank(int songId, const unsigned int& instrument, bool friends);  // 0xD55DC0
    eastl::set<int> GetAllSongsPlayed() const;  // 0xD576C0
    void GetHighestStatStars(int& stars, int& maxStars) const;  // 0xD57BF0
    bool IsSongEntitled(int songId) const;  // 0xD58180
    void UpdateSongPackAchievements(void* context);  // 0xD589C0

    // Field names are not in the reference map.
    eastl::vector<RBCampaignBandPersistentData> mCampaignBands;
    eastl::map<int, QuickplaySongPersistentData> mQuickplaySongData;
    eastl::map<int, QuickplaySongPersistentData> mCampaignSongData;
    eastl::map<int, RBSongLeaderboard> mSongLeaderboards;
    eastl::map<int, RBSongLeaderboard> mFriendsSongLeaderboards;
    RBMiniLeaderboardResults mMiniLeaderboardResults;
    eastl::map<int, unsigned char> mSongRatings;
    // A per-song list of 56-byte score entries with a 64-bit stamp
    // (0xD56000-0xD564F0, for the song library); not reconstructed.
    eastl::map<int, eastl::vector<unsigned char>> mSongScoreCache;
    eastl::map<int, long> mSongScoreCacheStamps;
    RBGameplayOptions mGameplayOptions;
    RBCharacterPersistentData mCharacterData;
    RBRockShopPersistentData mRockShopData;
    RBSeenIdsPersistentData mSeenIds;
    RBSetlistPersistentData mSetlists;
    FTUEPersistentData mFTUEData;
    // An RBAchPersistentData_PS4.
    RBAchPersistentData* mAchData;
    long mAchievementLogCount;
    RBBTMPersistentData mBTMData;
    RBClanPersistentData mClanData;
    RBRivalsPlayerProfile mRivalsPlayerProfile;
    RBSompSettingsPersistentData mSompSettings;
    RBUserPreferencesPersistentData mUserPreferences;
    // The pad's user name, cut to 16 characters with "...".
    String mPlatformName;
    OnlineID mOnlineID;
    // The pad's network restriction from the availability check: 0 when it
    // can go online, 1 age-restricted, 2 needs a patch, 3 not signed in, 5
    // not checked yet.
    int mSignInState;
    // The signed-into-network state Poll publishes from the worker
    // thread's mSignedIntoLive.
    bool mServerReachable;
    bool mSignedIntoLive;
    // The availability request, or 0.
    int mNpRequestId;
    // Asks Poll to check the availability again.
    bool mNeedsOnlineCheck;
    eastl::set<int> mEntitlements;
    JobState mPlatformEntitlementsJob;
    JobState mServerEntitlementsJob;
    JobState mSongUploadJob;
    CritSec mBandsLock;
    CritSec mSongDataLock;
    CritSec mLeaderboardLock;
    CritSec mSongScoreCacheLock;
    MsgSource::EventSinkElem mConnectionStatusSink;
};

static_assert(offsetof(RBProfile, mCampaignBands) == 200);
static_assert(offsetof(RBProfile, mQuickplaySongData) == 232);
static_assert(offsetof(RBProfile, mCampaignSongData) == 288);
static_assert(offsetof(RBProfile, mSongLeaderboards) == 344);
static_assert(offsetof(RBProfile, mFriendsSongLeaderboards) == 400);
static_assert(offsetof(RBProfile, mMiniLeaderboardResults) == 456);
static_assert(offsetof(RBProfile, mSongRatings) == 1120);
static_assert(offsetof(RBProfile, mSongScoreCache) == 1176);
static_assert(offsetof(RBProfile, mSongScoreCacheStamps) == 1232);
static_assert(offsetof(RBProfile, mGameplayOptions) == 1288);
static_assert(offsetof(RBProfile, mCharacterData) == 1496);
static_assert(offsetof(RBProfile, mRockShopData) == 1624);
static_assert(offsetof(RBProfile, mSeenIds) == 1824);
static_assert(offsetof(RBProfile, mSetlists) == 1912);
static_assert(offsetof(RBProfile, mFTUEData) == 1992);
static_assert(offsetof(RBProfile, mAchData) == 2048);
static_assert(offsetof(RBProfile, mAchievementLogCount) == 2056);
static_assert(offsetof(RBProfile, mBTMData) == 2064);
static_assert(offsetof(RBProfile, mClanData) == 2144);
static_assert(offsetof(RBProfile, mRivalsPlayerProfile) == 2280);
static_assert(offsetof(RBProfile, mSompSettings) == 2992);
static_assert(offsetof(RBProfile, mUserPreferences) == 3056);
static_assert(offsetof(RBProfile, mPlatformName) == 3112);
static_assert(offsetof(RBProfile, mOnlineID) == 3128);
static_assert(offsetof(RBProfile, mSignInState) == 3160);
static_assert(offsetof(RBProfile, mServerReachable) == 3164);
static_assert(offsetof(RBProfile, mSignedIntoLive) == 3165);
static_assert(offsetof(RBProfile, mNpRequestId) == 3168);
static_assert(offsetof(RBProfile, mNeedsOnlineCheck) == 3172);
static_assert(offsetof(RBProfile, mEntitlements) == 3176);
static_assert(offsetof(RBProfile, mPlatformEntitlementsJob) == 3232);
static_assert(offsetof(RBProfile, mServerEntitlementsJob) == 3240);
static_assert(offsetof(RBProfile, mSongUploadJob) == 3248);
static_assert(offsetof(RBProfile, mBandsLock) == 3256);
static_assert(offsetof(RBProfile, mSongDataLock) == 3272);
static_assert(offsetof(RBProfile, mLeaderboardLock) == 3288);
static_assert(offsetof(RBProfile, mSongScoreCacheLock) == 3304);
static_assert(offsetof(RBProfile, mConnectionStatusSink) == 3320);
static_assert(sizeof(RBProfile) == 3360);

// Orders profiles by platform name.
bool ProfileAlphaSort(const RBProfile* a, const RBProfile* b);  // 0xD59240
