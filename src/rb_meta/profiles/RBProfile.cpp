// rb_meta/RBProfile.o (0xD53430 to 0xD5B31F). The binary also emits here the
// map, vector and set templates the profile uses (0xD59490-0xD5B2DF) and the
// static initializer at 0xD5B2E0, which sets the shared header globals
// every rb_meta object initializes; they are not modelled. IDA leaves Save,
// Poll, CanSave and the deleting destructors undefined; they are read from
// the raw code. The members listed under "Declared only" in RBProfile.h
// are not reconstructed.
#include "rb_meta/profiles/RBProfile.h"

#include <np/np_common.h>
#include <cstring>
#include <functional>

#include "os/locale/Locale.h"
#include "os/memory/MemMgr.h"
#include "os/platform/PlatformMgr.h"
#include "rb_meta/entitlements/RBEntitlementMgr.h"
#include "rb_meta/persistentdata/RBAchPersistentData_PS4.h"
#include "rb_meta/profiles/RBProfileMgr.h"
#include "rb_meta/songs/SongMetadata.h"
#include "rb_meta/songs/SongMgr.h"
#include "utl/data/DataArray.h"
#include "utl/streams/RevisionStream.h"
#include "utl/text/UTF8.h"
#include "utl/time/TimeMgr.h"

// ---------------------------------------------------------------------------
// Declared for the profile: the online service and the band vector's
// reader have no declarations elsewhere. Names not in the reference map.

struct DingoService;
// A user's session with the online service; the profile reads its state.
struct DingoUserSession {
    unsigned char mHeader[16];
    // 4 once the user is logged in to the service.
    int mState;
};
// The online service, from the startup code.
extern DingoService g_dingo_service;  // 0x19FBAF0
// The pad's session, or null.
DingoUserSession* dingo_find_user_session(DingoService& service, int padNum);  // 0x33F700

// Reads a count and that many bands, as the profile's
// Revision::AsOfOp<eastl::vector<RBCampaignBandPersistentData>> (vtable
// 0x197DB98, Read 0xD59A70) does.
BinStream& operator>>(BinStream& stream, eastl::vector<RBCampaignBandPersistentData>& bands);

// ---------------------------------------------------------------------------

namespace {

// The NP request the network availability check runs on. Name not in the
// reference map.
SceNpCreateAsyncRequestParameter MakeAsyncRequestParameter() {
    SceNpCreateAsyncRequestParameter parameter;
    memset(&parameter, 0, sizeof(parameter));
    parameter.size = sizeof(parameter);
    parameter.cpuAffinityMask = 0x3F;
    parameter.threadPriority = 700;
    return parameter;
}

// The longest platform name the profile keeps, in characters, and the
// UTF-8 buffer it is cut into. Names not in the reference map.
constexpr unsigned long kPlatformNameLength = 16;
constexpr unsigned long kPlatformNameBufferSize = 65;

// Entitlements the save keeps: the ids of the on-disc and launch songs.
// Names not in the reference map.
constexpr int kFirstSavedEntitlement = 3;
constexpr int kLastSavedEntitlement = 0x147;

// How long a server job may run before it is retried, in milliseconds.
// Names not in the reference map.
constexpr float kEntitlementRetryMs = 20000.0f;
constexpr float kSongUploadRetryMs = 10000.0f;

// The NP errors the availability check maps to restrictions. Names not in
// the reference map.
constexpr int kNpErrorAgeRestriction = static_cast<int>(0x8055000B);
constexpr unsigned int kNpErrorLatestSystemSoftware = 0x8055000F;

// The profile's restrictions (RBProfile::mSignInState). Names not in the
// reference map.
enum SignInState {
    kSignInOk = 0,
    kSignInAgeRestricted = 1,
    kSignInNeedsPatch = 2,
    kSignInNotSignedIn = 3,
    kSignInUnknown = 5,
};

// Whether the song is a public RB4 song, which GetHighestStatStars counts.
// Inlined. Name not in the reference map.
bool IsCountedRB4Song(int songId) {
    SongMetadata* const metadata = theSongMgr->GetMetadata(songId, false);
    return metadata != nullptr && metadata->IsRB4Song() && !metadata->IsPrivate();
}

// The song packs whose ownership logs a trophy (UpdateSongPackAchievements).
// Names not in the reference map.
constexpr int kSongPackEntitlement15 = 15;
constexpr int kSongPackEntitlement16 = 16;
constexpr int kSongPackEntitlement17 = 17;

// The highest song rating. Name not in the reference map.
constexpr unsigned char kMaxSongRating = 5;

// Calls the function with each song id an entitlement can name, 1 to 327.
// Inlined into the entitlement sets, which build a std::function (vtables
// 0x197DDC0, 0x197DE10, 0x197DE60). Name not in the reference map.
void ForEachEntitlementSongId(const std::function<void(int)>& function) {
    for (int songId = 1; songId <= kLastSavedEntitlement; ++songId) {
        function(songId);
    }
}

// The node of a set or map key.
template <typename Key>
const Key& NodeKey(const eastl::rbtree_node_base* node) {
    return static_cast<const typename eastl::set<Key>::node_type*>(node)->mValue;
}

}  // namespace

// Adds the ids of the songs with results to the set. Name not in the
// reference map.
static void AddSongsPlayed(eastl::set<int>& songs, const eastl::map<int, QuickplaySongPersistentData>& songData);

// Reconstructed from eboot.elf at 0xD53430. _StartOnlineCheck is inlined.
RBProfile::RBProfile(int padNum)
    : Profile(padNum),
      mMiniLeaderboardResults(padNum),
      mGameplayOptions(this),
      mCharacterData(this),
      mRockShopData(this),
      mSeenIds(this),
      mSetlists(this),
      mFTUEData(this),
      mAchievementLogCount(0),
      mBTMData(this),
      mClanData(this),
      mRivalsPlayerProfile(this, padNum),
      mSompSettings(this),
      mUserPreferences(this),
      mSignInState(kSignInUnknown),
      mServerReachable(false),
      mSignedIntoLive(false),
      mNpRequestId(0),
      mNeedsOnlineCheck(true),
      mPlatformEntitlementsJob{true, false, -1.0f},
      mServerEntitlementsJob{true, false, -1.0f},
      mSongUploadJob{false, false, -1.0f} {
    mStorage.mOwner = padNum;
    mAchData = new RBAchPersistentData_PS4(this);
    DeleteAll();
    mPlatformName = ThePlatformMgr->GetName(padNum);
    if (*mPlatformName.c_str() != '\0') {
        char shortName[kPlatformNameLength];
        const unsigned long length =
            UTF8Resize(shortName, kPlatformNameLength, kPlatformNameBufferSize, mPlatformName.c_str());
        if (length < strlen(mPlatformName.c_str())) {
            mPlatformName = shortName;
            mPlatformName += "...";
        }
    }
    _StartOnlineCheck();
    ThePlatformMgr->AddSink(
        &mConnectionStatusSink, this, ConnectionStatusChangedMsg::Event(), Symbol());
}

// Reconstructed from eboot.elf at 0xD53D70.
void RBProfile::_StartOnlineCheck() {
    ThePlatformMgr->GetOnlineID(mPadNum, &mOnlineID);
    if (mOnlineID.mValid) {
        const SceNpCreateAsyncRequestParameter parameter = MakeAsyncRequestParameter();
        mNpRequestId = sceNpCreateAsyncRequest(&parameter);
        if (mNpRequestId > 0) {
            sceNpCheckNpAvailabilityA(mNpRequestId, mPadNum);
            return;
        }
    }
    mSignInState = kSignInNotSignedIn;
}

// Reconstructed from eboot.elf at 0xD53E20. The deleting destructor is at
// 0xD541D0.
RBProfile::~RBProfile() {
    DeleteAll();
    delete mAchData;
    mAchData = nullptr;
    if (mNpRequestId > 0) {
        sceNpAbortRequest(mNpRequestId);
        sceNpDeleteRequest(mNpRequestId);
        mNpRequestId = 0;
    }
    if (mConnectionStatusSink.mSource != nullptr) {
        mConnectionStatusSink.mSource->RemoveSink(&mConnectionStatusSink);
    }
}

// Reconstructed from eboot.elf at 0xD54220. The poll result 0 means the
// request finished.
void RBProfile::Poll() {
    if (mNpRequestId != 0) {
        int result;
        if (sceNpPollAsync(mNpRequestId, &result) == 0) {
            if (result >= 0) {
                mSignInState = kSignInOk;
                mServerReachable = true;
                mSignedIntoLive = true;
            } else if (static_cast<unsigned int>(result) - kNpErrorLatestSystemSoftware <= 1) {
                mSignInState = kSignInNeedsPatch;
            } else {
                mSignInState = result != kNpErrorAgeRestriction ? kSignInNotSignedIn : kSignInAgeRestricted;
            }
            sceNpDeleteRequest(mNpRequestId);
            mNpRequestId = 0;
        }
    } else if (mNeedsOnlineCheck) {
        DingoUserSession* const session = dingo_find_user_session(g_dingo_service, mPadNum);
        if (session != nullptr && session->mState == 4) {
            ThePlatformMgr->GetOnlineID(mPadNum, &mOnlineID);
            bool started = false;
            if (mOnlineID.mValid) {
                const SceNpCreateAsyncRequestParameter parameter = MakeAsyncRequestParameter();
                mNpRequestId = sceNpCreateAsyncRequest(&parameter);
                if (mNpRequestId > 0) {
                    sceNpCheckNpAvailabilityA(mNpRequestId, mPadNum);
                    started = true;
                }
            }
            if (!started) {
                mSignInState = kSignInNotSignedIn;
            }
            mNeedsOnlineCheck = false;
        }
    }
    if (mSignedIntoLive != mServerReachable) {
        mServerReachable = mSignedIntoLive;
    }
}

// Reconstructed from eboot.elf at 0xD543A0. The first-time-user events,
// the achievements, the mini leaderboard results and the Rivals player
// profile are kept.
void RBProfile::DeleteAll() {
    ScopedCritSec songDataLock(mSongDataLock);
    ScopedCritSec bandsLock(mBandsLock);
    EnableDirtying(false);
    mQuickplaySongData.clear();
    mCampaignSongData.clear();
    mSongRatings.clear();
    mGameplayOptions.Clear();
    mSompSettings.Clear();
    mCharacterData.Clear();
    mCampaignBands.clear();
    mRockShopData.Clear();
    mSetlists.Clear();
    mAchievementLogCount = 0;
    mBTMData.Clear();
    mSeenIds.Clear();
    mClanData.Clear();
    mUserPreferences.Clear();
    mEntitlements.clear();
    mServerEntitlementsJob.mNeeded = true;
    mPlatformEntitlementsJob.mNeeded = true;
    mPlatformEntitlementsJob.mPending = false;
    mServerEntitlementsJob.mPending = false;
    EnableDirtying(true);
    Profile::DeleteAll();
}

// Reconstructed from eboot.elf at 0xD54650.
void RBProfile::SetNeedsEntitlementRefresh(bool needed) {
    mPlatformEntitlementsJob.mNeeded = needed;
    mServerEntitlementsJob.mNeeded = needed;
}

// Reconstructed from eboot.elf at 0xD54660. A job that starts records the
// time.
void RBProfile::SetPlatformEntitlementsPending(bool pending) {
    const bool wasPending = mPlatformEntitlementsJob.mPending;
    mPlatformEntitlementsJob.mPending = pending;
    if (!wasPending && pending) {
        mPlatformEntitlementsJob.mStartTime = TheTimeMgr->mRealTime.SplitMs();
    }
}

// Reconstructed from eboot.elf at 0xD546E0.
void RBProfile::SetServerEntitlementsPending(bool pending) {
    const bool wasPending = mServerEntitlementsJob.mPending;
    mServerEntitlementsJob.mPending = pending;
    if (!wasPending && pending) {
        mServerEntitlementsJob.mStartTime = TheTimeMgr->mRealTime.SplitMs();
    }
}

// Reconstructed from eboot.elf at 0xD54760.
const char* RBProfile::GetPlatformName() const {
    if (mPadNum != -1) {
        return mPlatformName.c_str();
    }
    return Localize(Symbol("Guest"), nullptr, 0, TheLocale);
}

// Reconstructed from eboot.elf at 0xD547D0.
bool RBProfile::CanSave() const {
    return mStorage.IsValid();
}

// Reconstructed from eboot.elf at 0xD547E0. The entitlements are written as
// _GetSavedEntitlements (0xD54C50) collects them, inline here.
void RBProfile::Save(BinStream& stream) const {
    ScopedCritSec songDataLock(const_cast<CritSec&>(mSongDataLock));
    ScopedCritSec bandsLock(const_cast<CritSec&>(mBandsLock));
    RevisionStream revisionStream(stream, kRevision, 0);
    revisionStream.BeginWrite();
    theRBProfileMgr->mSystemOptions.Save(revisionStream);
    int count = static_cast<int>(mCampaignBands.size());
    revisionStream.WriteEndian(&count, sizeof(count));
    for (const RBCampaignBandPersistentData* band = mCampaignBands.begin(); band != mCampaignBands.end(); ++band) {
        band->Save(revisionStream);
    }
    const eastl::map<int, QuickplaySongPersistentData>* const songMaps[] = {
        &mQuickplaySongData,
        &mCampaignSongData,
    };
    for (const auto* songs : songMaps) {
        count = static_cast<int>(songs->mnSize);
        revisionStream.WriteEndian(&count, sizeof(count));
        for (const eastl::rbtree_node_base* node = songs->mAnchor.mpNodeLeft; node != &songs->mAnchor;
             node = eastl::RBTreeIncrement(node)) {
            const auto& entry =
                static_cast<const eastl::map<int, QuickplaySongPersistentData>::node_type*>(node)->mValue;
            int songId = entry.first;
            revisionStream.WriteEndian(&songId, sizeof(songId));
            entry.second.Save(revisionStream);
        }
    }
    count = static_cast<int>(mSongRatings.mnSize);
    revisionStream.WriteEndian(&count, sizeof(count));
    for (const eastl::rbtree_node_base* node = mSongRatings.mAnchor.mpNodeLeft; node != &mSongRatings.mAnchor;
         node = eastl::RBTreeIncrement(node)) {
        const auto& entry = static_cast<const eastl::map<int, unsigned char>::node_type*>(node)->mValue;
        int songId = entry.first;
        revisionStream.WriteEndian(&songId, sizeof(songId));
        unsigned char rating = entry.second;
        revisionStream.Write(&rating, sizeof(rating));
    }
    mGameplayOptions.Save(revisionStream);
    mSompSettings.Save(revisionStream);
    mCharacterData.Save(revisionStream);
    mRockShopData.Save(revisionStream);
    mFTUEData.Save(revisionStream);
    mAchData->Save(revisionStream);
    int achievementLogCount = static_cast<int>(mAchievementLogCount);
    revisionStream.WriteEndian(&achievementLogCount, sizeof(achievementLogCount));
    mSetlists.Save(revisionStream);
    mBTMData.Save(revisionStream);
    mSeenIds.Save(revisionStream);
    mClanData.Save(revisionStream);
    mUserPreferences.Save(revisionStream);
    count = 0;
    for (const eastl::rbtree_node_base* node = mEntitlements.mAnchor.mpNodeLeft; node != &mEntitlements.mAnchor;
         node = eastl::RBTreeIncrement(node)) {
        const int songId = NodeKey<int>(node);
        if (songId >= kFirstSavedEntitlement && songId <= kLastSavedEntitlement) {
            ++count;
        }
    }
    revisionStream.WriteEndian(&count, sizeof(count));
    for (const eastl::rbtree_node_base* node = mEntitlements.mAnchor.mpNodeLeft; node != &mEntitlements.mAnchor;
         node = eastl::RBTreeIncrement(node)) {
        int songId = NodeKey<int>(node);
        if (songId >= kFirstSavedEntitlement && songId <= kLastSavedEntitlement) {
            revisionStream.WriteEndian(&songId, sizeof(songId));
        }
    }
}

// Reconstructed from eboot.elf at 0xD54C50.
eastl::set<int> RBProfile::_GetSavedEntitlements() const {
    unsigned int saved = 0;
    MemPushTemp(saved, true, true);
    eastl::set<int> entitlements;
    for (const eastl::rbtree_node_base* node = mEntitlements.begin(); node != mEntitlements.end();
         node = eastl::RBTreeIncrement(node)) {
        const int songId = NodeKey<int>(node);
        if (songId >= kFirstSavedEntitlement && songId <= kLastSavedEntitlement) {
            entitlements.insert(songId);
        }
    }
    MemPopTemp(saved);
    return entitlements;
}

// Reconstructed from eboot.elf at 0xD54E00. Saves older than revision 13
// are dropped. The binary reads each piece through a Revision::AsOfOp of
// its type; the revision checks are written out here. A guest profile whose
// block cannot be read only logs its name.
void RBProfile::Load(BinStream& stream) {
    ScopedCritSec songDataLock(mSongDataLock);
    ScopedCritSec bandsLock(mBandsLock);
    RevisionStream revisionStream(stream, kRevision, 0);
    if (!revisionStream.BeginRead()) {
        return;
    }
    DeleteAll();
    const int revision = revisionStream.mReadRevision;
    if (revision <= 12) {
        mReadyToSave = true;
        return;
    }
    EnableDirtying(false);
    {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        theRBProfileMgr->mSystemOptions.Load(revisionStream);
        MemPopHeap();
        if (revision == 6) {
            theSongMgr->LoadSongCache(revisionStream);
        }
    }
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    if (revision > 7) {
        revisionStream >> mCampaignBands;
    } else if (revision >= 2) {
        RBCampaignBandPersistentData band(nullptr);
        band.Load(revisionStream);
        mCampaignBands.push_back(band);
    }
    if (revision >= 3) {
        revisionStream >> mQuickplaySongData;
    }
    if (revision >= 14) {
        revisionStream >> mCampaignSongData;
    }
    if (revision >= 20) {
        revisionStream >> mSongRatings;
    }
    if (revision >= 4) {
        mGameplayOptions.Load(revisionStream);
    }
    if (revision >= 24) {
        mSompSettings.Load(revisionStream);
    }
    if (revision >= 5) {
        mCharacterData.Load(revisionStream);
    }
    if (revision >= 9) {
        mRockShopData.Load(revisionStream);
    }
    if (revision >= 10) {
        mFTUEData.Load(revisionStream);
    }
    if (revision >= 12) {
        mAchData->Load(revisionStream);
    }
    if (revision >= 15) {
        int achievementLogCount;
        revisionStream.ReadEndian(&achievementLogCount, sizeof(achievementLogCount));
        mAchievementLogCount = achievementLogCount;
    }
    if (revision >= 16) {
        mSetlists.Load(revisionStream);
    }
    if (revision >= 17 && revision <= 20) {
        // A name of the Rivals player profile (its String at +80) that later
        // revisions dropped; the binary reads it into the player profile.
        String rivalsName;
        revisionStream >> rivalsName;
    }
    if (revision >= 18) {
        mBTMData.Load(revisionStream);
    }
    if (revision >= 19) {
        mSeenIds.Load(revisionStream);
    }
    if (revision >= 22) {
        mClanData.Load(revisionStream);
    }
    if (revision >= 25) {
        mUserPreferences.Load(revisionStream);
    }
    if (revision >= 23) {
        unsigned int entitlementCount;
        revisionStream.ReadEndian(&entitlementCount, sizeof(entitlementCount));
        eastl::set<int> entitlements;
        for (unsigned int index = 0; index < entitlementCount; ++index) {
            int songId;
            revisionStream.ReadEndian(&songId, sizeof(songId));
            entitlements.insert(songId);
        }
        AddEntitlements(entitlements);
    }
    _PreparePersistentData();
    EnableDirtying(true);
    MakeClean();
    mReadyToSave = true;
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD556F0.
void RBProfile::_PreparePersistentData() {
    ScopedCritSec songDataLock(mSongDataLock);
    for (auto it = mQuickplaySongData.begin(); it != mQuickplaySongData.end(); ++it) {
        it->second.mParent = this;
    }
    for (auto it = mCampaignSongData.begin(); it != mCampaignSongData.end(); ++it) {
        it->second.mParent = this;
    }
    ScopedCritSec bandsLock(mBandsLock);
    for (RBCampaignBandPersistentData* band = mCampaignBands.begin(); band != mCampaignBands.end(); ++band) {
        band->mParent = this;
    }
    mBTMData.mParent = this;
    mSeenIds.mParent = this;
    mClanData.mParent = this;
}

// Reconstructed from eboot.elf at 0xD55800. The new board is copied into
// the cache; the copies start without a subscription.
RBSongLeaderboard* RBProfile::GetSongLeaderboard(int songId) {
    ScopedCritSec leaderboardLock(mLeaderboardLock);
    auto it = mSongLeaderboards.find(songId);
    if (it == mSongLeaderboards.end()) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        const RBSongLeaderboard board(this, songId, false);
        it = mSongLeaderboards.insert(eastl::pair<const int, RBSongLeaderboard>{songId, board}).first;
        MemPopHeap();
    }
    return &it->second;
}

// Reconstructed from eboot.elf at 0xD55AB0.
RBSongLeaderboard* RBProfile::GetSongLeaderboard(Symbol song) {
    return GetSongLeaderboard(theSongMgr->GetSongIDFromShortName(song, true));
}

// Reconstructed from eboot.elf at 0xD55AE0.
RBSongLeaderboard* RBProfile::GetFriendsSongLeaderboard(int songId) {
    ScopedCritSec leaderboardLock(mLeaderboardLock);
    auto it = mFriendsSongLeaderboards.find(songId);
    if (it == mFriendsSongLeaderboards.end()) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        const RBSongLeaderboard board(this, songId, true);
        it = mFriendsSongLeaderboards.insert(eastl::pair<const int, RBSongLeaderboard>{songId, board}).first;
        MemPopHeap();
    }
    return &it->second;
}

// Reconstructed from eboot.elf at 0xD55D90.
RBSongLeaderboard* RBProfile::GetFriendsSongLeaderboard(Symbol song) {
    return GetFriendsSongLeaderboard(theSongMgr->GetSongIDFromShortName(song, true));
}

// Reconstructed from eboot.elf at 0xD55DC0. The board is copied first; the
// profile's row is the one with its platform user id.
int RBProfile::GetLeaderboardScore(int songId, const ScoreType& type, bool friends) {
    ScopedCritSec leaderboardLock(mLeaderboardLock);
    const RBSongLeaderboard board = friends ? *GetFriendsSongLeaderboard(songId) : *GetSongLeaderboard(songId);
    if (!board.HasResults(type)) {
        return 0;
    }
    const eastl::vector<RBSongLeaderboard::Entry> entries = board.GetEntries(type);
    const OnlineID onlineId = mOnlineID;
    const Symbol platformUid(onlineId.ToString());
    for (const RBSongLeaderboard::Entry& entry : entries) {
        if (entry.mPlatformUid == platformUid) {
            return static_cast<int>(entry.mScore);
        }
    }
    return 0;
}

// Reconstructed from eboot.elf at 0xD55FB0.
void RBProfile::LockSongScoreCache() {
    mSongScoreCacheLock.Enter();
}

// Reconstructed from eboot.elf at 0xD55FE0.
void RBProfile::UnlockSongScoreCache() {
    mSongScoreCacheLock.Exit();
}

// Reconstructed from eboot.elf at 0xD56000.
eastl::vector<RBSongLeaderboard::Entry>* RBProfile::GetCachedSongScores(const int& songId) {
    ScopedCritSec scoreCacheLock(mSongScoreCacheLock);
    return &mSongScoreCache[songId];
}

// Reconstructed from eboot.elf at 0xD56110.
long RBProfile::GetCachedSongScoresStamp(const int& songId) {
    ScopedCritSec scoreCacheLock(mSongScoreCacheLock);
    return mSongScoreCacheStamps[songId];
}

// Reconstructed from eboot.elf at 0xD56210.
void RBProfile::SetCachedSongScores(
    const int& songId,
    const eastl::vector<RBSongLeaderboard::Entry>& scores,
    long stamp) {
    ScopedCritSec scoreCacheLock(mSongScoreCacheLock);
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    eastl::vector<RBSongLeaderboard::Entry>& cached = mSongScoreCache[songId];
    if (&cached != &scores) {
        cached = scores;
    }
    mSongScoreCacheStamps[songId] = stamp;
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD564F0.
void RBProfile::ClearCachedSongScores(const int& songId) {
    ScopedCritSec scoreCacheLock(mSongScoreCacheLock);
    mSongScoreCache[songId].clear();
    mSongScoreCacheStamps[songId] = 0;
}

// Reconstructed from eboot.elf at 0xD56660.
RBCampaignBandPersistentData* RBProfile::CreateNewBand() {
    ScopedCritSec bandsLock(mBandsLock);
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCampaignBands.push_back(RBCampaignBandPersistentData(nullptr));
    RBCampaignBandPersistentData& band = mCampaignBands.back();
    band.mParent = this;
    band.GenerateGuid();
    MemPopHeap();
    return &band;
}

// Reconstructed from eboot.elf at 0xD567A0.
RBCampaignBandPersistentData* RBProfile::GetBandCampaignProgressData(unsigned long band) {
    ScopedCritSec bandsLock(mBandsLock);
    return band < mCampaignBands.size() ? &mCampaignBands[band] : nullptr;
}

// Reconstructed from eboot.elf at 0xD56830.
const RBCampaignBandPersistentData* RBProfile::GetBandCampaignProgressData(unsigned long band) const {
    ScopedCritSec bandsLock(const_cast<CritSec&>(mBandsLock));
    return band < mCampaignBands.size() ? &mCampaignBands[band] : nullptr;
}

// Reconstructed from eboot.elf at 0xD568C0.
void RBProfile::DeleteCampaignProgress(unsigned long band) {
    ScopedCritSec bandsLock(mBandsLock);
    if (band < mCampaignBands.size()) {
        mCampaignBands.erase(&mCampaignBands[band]);
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD569C0.
unsigned long RBProfile::GetNumBands() const {
    ScopedCritSec bandsLock(const_cast<CritSec&>(mBandsLock));
    return mCampaignBands.size();
}

// Reconstructed from eboot.elf at 0xD56A20.
void RBProfile::MarkBandAsRecentlyPlayed(unsigned long band) {
    ScopedCritSec bandsLock(mBandsLock);
    for (unsigned long index = 0; index < mCampaignBands.size(); ++index) {
        mCampaignBands[index].MarkAsRecentlyPlayed(band == index);
    }
}

// Reconstructed from eboot.elf at 0xD56AF0.
RBCampaignBandPersistentData* RBProfile::GetMostRecentlyPlayedBand(unsigned long& band) const {
    ScopedCritSec bandsLock(const_cast<CritSec&>(mBandsLock));
    unsigned long index = 0;
    for (const RBCampaignBandPersistentData* data = mCampaignBands.begin(); data != mCampaignBands.end();
         ++data, ++index) {
        if (data->mRecentlyPlayed) {
            band = index;
            return const_cast<RBCampaignBandPersistentData*>(data);
        }
    }
    band = static_cast<unsigned long>(-1);
    return nullptr;
}

// Reconstructed from eboot.elf at 0xD56B90.
QuickplaySongPersistentData* RBProfile::GetQuickplaySongData(int songId) {
    ScopedCritSec songDataLock(mSongDataLock);
    const auto it = mQuickplaySongData.find(songId);
    return it != mQuickplaySongData.end() ? &it->second : nullptr;
}

// Reconstructed from eboot.elf at 0xD56C60.
QuickplaySongPersistentData* RBProfile::GetQuickplaySongData(Symbol song) {
    return GetQuickplaySongData(theSongMgr->GetSongIDFromShortName(song, false));
}

// Reconstructed from eboot.elf at 0xD56D40.
QuickplaySongPersistentData* RBProfile::GetCampaignSongData(int songId) {
    ScopedCritSec songDataLock(mSongDataLock);
    const auto it = mCampaignSongData.find(songId);
    return it != mCampaignSongData.end() ? &it->second : nullptr;
}

// Reconstructed from eboot.elf at 0xD56E10.
QuickplaySongPersistentData* RBProfile::GetCampaignSongData(Symbol song) {
    return GetCampaignSongData(theSongMgr->GetSongIDFromShortName(song, false));
}

// Reconstructed from eboot.elf at 0xD56EF0. A new song's results are
// assigned into the map's default node and pointed back at the profile.
QuickplaySongPersistentData* RBProfile::GetOrCreateCampaignSongData(Symbol song) {
    const int songId = theSongMgr->GetSongIDFromShortName(song, true);
    if (songId == 0) {
        return nullptr;
    }
    ScopedCritSec songDataLock(mSongDataLock);
    auto it = mCampaignSongData.find(songId);
    if (it == mCampaignSongData.end()) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        const QuickplaySongPersistentData data(this, songId);
        mCampaignSongData[songId] = data;
        mCampaignSongData[songId].mParent = this;
        it = mCampaignSongData.find(songId);
        MemPopHeap();
    }
    return &it->second;
}

// Reconstructed from eboot.elf at 0xD57250. As GetOrCreateCampaignSongData.
QuickplaySongPersistentData* RBProfile::GetOrCreateQuickplaySongData(Symbol song) {
    const int songId = theSongMgr->GetSongIDFromShortName(song, true);
    if (songId == 0) {
        return nullptr;
    }
    ScopedCritSec songDataLock(mSongDataLock);
    auto it = mQuickplaySongData.find(songId);
    if (it == mQuickplaySongData.end()) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        const QuickplaySongPersistentData data(this, songId);
        mQuickplaySongData[songId] = data;
        mQuickplaySongData[songId].mParent = this;
        it = mQuickplaySongData.find(songId);
        MemPopHeap();
    }
    return &it->second;
}

// Reconstructed from eboot.elf at 0xD575B0.
unsigned long RBProfile::GetNumQuickplaySongs() const {
    return mQuickplaySongData.size();
}

// Reconstructed from eboot.elf at 0xD575C0. Each song's results are
// copied before they are asked; the song data lock is not taken.
ScoreType RBProfile::GetMoreScoredType(ScoreType first, ScoreType second) const {
    int firstCount = 0;
    int secondCount = 0;
    for (auto it = mQuickplaySongData.begin(); it != mQuickplaySongData.end(); ++it) {
        const QuickplaySongPersistentData data(it->second);
        firstCount += data.HasScore(first) ? 1 : 0;
        secondCount += data.HasScore(second) ? 1 : 0;
    }
    return secondCount > firstCount ? second : first;
}

// Reconstructed from eboot.elf at 0xD576C0. The bands are read under their
// own lock inside the song data's.
eastl::set<int> RBProfile::GetAllSongsPlayed() const {
    ScopedCritSec songDataLock(const_cast<CritSec&>(mSongDataLock));
    eastl::set<int> songs;
    AddSongsPlayed(songs, mQuickplaySongData);
    AddSongsPlayed(songs, mCampaignSongData);
    ScopedCritSec bandsLock(const_cast<CritSec&>(mBandsLock));
    for (const RBCampaignBandPersistentData& band : mCampaignBands) {
        for (const eastl::rbtree_node_base* node = band.mSongsPlayed.begin(); node != band.mSongsPlayed.end();
             node = eastl::RBTreeIncrement(node)) {
            songs.insert(NodeKey<int>(node));
        }
    }
    return songs;
}

// Reconstructed from eboot.elf at 0xD57920.
static void AddSongsPlayed(eastl::set<int>& songs, const eastl::map<int, QuickplaySongPersistentData>& songData) {
    for (auto it = songData.begin(); it != songData.end(); ++it) {
        songs.insert(it->second.GetID());
    }
}

// Reconstructed from eboot.elf at 0xD57A30.
eastl::set<int> RBProfile::GetQuickplaySongsPlayed() const {
    ScopedCritSec songDataLock(const_cast<CritSec&>(mSongDataLock));
    eastl::set<int> songs;
    AddSongsPlayed(songs, mQuickplaySongData);
    return songs;
}

// Reconstructed from eboot.elf at 0xD57B10.
eastl::set<int> RBProfile::GetCampaignSongsPlayed() const {
    ScopedCritSec songDataLock(const_cast<CritSec&>(mSongDataLock));
    eastl::set<int> songs;
    AddSongsPlayed(songs, mCampaignSongData);
    return songs;
}

// Reconstructed from eboot.elf at 0xD57BF0. Only the quickplay songs count
// toward `stars`; the best stars of each song count once toward
// `rb4Stars`.
void RBProfile::GetHighestStatStars(int& stars, int& rb4Stars) const {
    ScopedCritSec songDataLock(const_cast<CritSec&>(mSongDataLock));
    stars = 0;
    rb4Stars = 0;
    eastl::map<int, int> bestStars;
    for (auto it = mQuickplaySongData.begin(); it != mQuickplaySongData.end(); ++it) {
        const int songId = it->first;
        const int songStars = it->second.GetHighStatStars();
        stars += songStars;
        if (IsCountedRB4Song(songId)) {
            int& best = bestStars[songId];
            best = best > songStars ? best : songStars;
        }
    }
    ScopedCritSec bandsLock(const_cast<CritSec&>(mBandsLock));
    for (const RBCampaignBandPersistentData& band : mCampaignBands) {
        for (auto it = band.mSongData.begin(); it != band.mSongData.end(); ++it) {
            const int songId = it->first;
            if (!IsCountedRB4Song(songId)) {
                continue;
            }
            const int songStars = it->second.GetHighStatStars();
            int& best = bestStars[songId];
            best = best > songStars ? best : songStars;
        }
    }
    for (auto it = bestStars.begin(); it != bestStars.end(); ++it) {
        rb4Stars += it->second;
    }
}

// Reconstructed from eboot.elf at 0xD57FF0.
int RBProfile::GetQuickplayCareerScore() const {
    ScopedCritSec songDataLock(const_cast<CritSec&>(mSongDataLock));
    int score = 0;
    for (const eastl::rbtree_node_base* node = mQuickplaySongData.mAnchor.mpNodeLeft;
         node != &mQuickplaySongData.mAnchor; node = eastl::RBTreeIncrement(node)) {
        const auto& entry =
            static_cast<const eastl::map<int, QuickplaySongPersistentData>::node_type*>(node)->mValue;
        SongMetadata* const metadata = theSongMgr->GetMetadata(entry.first, false);
        if (metadata == nullptr || !metadata->IsPrivate()) {
            score += entry.second.GetCareerScore();
        }
    }
    return score;
}

// Reconstructed from eboot.elf at 0xD580C0.
Symbol RBProfile::GetSignInErrorToken() const {
    const char* token;
    switch (mSignInState) {
    case kSignInAgeRestricted:
        token = "store_restricted_age";
        break;
    case kSignInNeedsPatch:
        token = "store_need_to_patch";
        break;
    case kSignInNotSignedIn:
        token = "store_need_to_signin";
        break;
    case kSignInUnknown:
        token = "error_unknown";
        break;
    default:
        token = "";
        break;
    }
    return Symbol(token);
}

// Reconstructed from eboot.elf at 0xD58150.
void RBProfile::IncrementAchievementLogCount() {
    if (mAchievementLogCount != 0x7FFFFFFFFFFFFFFF) {
        ++mAchievementLogCount;
        MakeDirty();
    }
}

// Reconstructed from eboot.elf at 0xD58180.
bool RBProfile::IsSongEntitled(int songId) const {
    if (HasEntitlement(songId)) {
        return true;
    }
    return theRBEntitlementMgr->IsSongEntitled(songId, mPadNum);
}

// Reconstructed from eboot.elf at 0xD58210.
bool RBProfile::HasEntitlement(int songId) const {
    return mEntitlements.find(songId) != mEntitlements.end();
}

// Reconstructed from eboot.elf at 0xD582B0.
void RBProfile::ClearEntitlements() {
    mEntitlements.clear();
}

// Reconstructed from eboot.elf at 0xD58310. Song ids the entitlement manager
// (0xB46170) handles itself go there instead; the binary logs the guest
// profile's name for them.
void RBProfile::AddEntitlement(int songId) {
    if (HasEntitlement(songId)) {
        return;
    }
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mEntitlements.insert(songId);
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xD58510.
void RBProfile::AddEntitlements(const eastl::set<int>& songIds) {
    for (const eastl::rbtree_node_base* node = songIds.mAnchor.mpNodeLeft; node != &songIds.mAnchor;
         node = eastl::RBTreeIncrement(node)) {
        AddEntitlement(NodeKey<int>(node));
    }
}

// Reconstructed from eboot.elf at 0xD585E0.
void RBProfile::SetNeedsPlatformEntitlements(bool needed) {
    mPlatformEntitlementsJob.mNeeded = needed;
}

// Reconstructed from eboot.elf at 0xD585F0.
void RBProfile::SetNeedsServerEntitlements(bool needed) {
    mServerEntitlementsJob.mNeeded = needed;
}

// Reconstructed from eboot.elf at 0xD58600.
eastl::set<int> RBProfile::GetEntitledSongs() const {
    unsigned int saved = 0;
    MemPushTemp(saved, true, true);
    eastl::set<int> songs;
    ForEachEntitlementSongId([this, &songs](int songId) {
        if (IsSongEntitled(songId)) {
            songs.insert(songId);
        }
    });
    MemPopTemp(saved);
    return songs;
}

// Reconstructed from eboot.elf at 0xD58740.
eastl::set<int> RBProfile::GetProfileEntitledSongs() const {
    unsigned int saved = 0;
    MemPushTemp(saved, true, true);
    eastl::set<int> songs;
    ForEachEntitlementSongId([this, &songs](int songId) {
        if (HasEntitlement(songId)) {
            songs.insert(songId);
        }
    });
    MemPopTemp(saved);
    return songs;
}

// Reconstructed from eboot.elf at 0xD58880.
eastl::set<int> RBProfile::GetPlatformEntitledSongs() const {
    unsigned int saved = 0;
    MemPushTemp(saved, true, true);
    eastl::set<int> songs;
    ForEachEntitlementSongId([this, &songs](int songId) {
        if (theRBEntitlementMgr->IsSongEntitled(songId, mPadNum)) {
            songs.insert(songId);
        }
    });
    MemPopTemp(saved);
    return songs;
}

// Reconstructed from eboot.elf at 0xD589C0. The achievements' interface
// slots run 28, 27 and 29 for the entitlements 15, 16 and 17.
void RBProfile::UpdateSongPackAchievements() {
    if (mAchData == nullptr) {
        return;
    }
    if (IsSongEntitled(kSongPackEntitlement15)) {
        mAchData->LogEntitlement15Owned();
    }
    if (IsSongEntitled(kSongPackEntitlement16)) {
        mAchData->LogEntitlement16Owned();
    }
    if (IsSongEntitled(kSongPackEntitlement17)) {
        mAchData->LogEntitlement17Owned();
    }
}

// Reconstructed from eboot.elf at 0xD58BE0.
bool RBProfile::IsEntitlementRetryWaiting() const {
    const float now = TheTimeMgr->mRealTime.SplitMs();
    return now - mPlatformEntitlementsJob.mStartTime < kEntitlementRetryMs
        || now - mServerEntitlementsJob.mStartTime < kEntitlementRetryMs;
}

// Reconstructed from eboot.elf at 0xD58C70.
void RBProfile::SetSongsUploaded(bool uploaded) {
    mSongUploadJob.mNeeded = uploaded;
}

// Reconstructed from eboot.elf at 0xD58C80.
void RBProfile::SetSongUploadPending(bool pending) {
    const bool wasPending = mSongUploadJob.mPending;
    mSongUploadJob.mPending = pending;
    if (!wasPending && pending) {
        mSongUploadJob.mStartTime = TheTimeMgr->mRealTime.SplitMs();
    }
}

// Reconstructed from eboot.elf at 0xD58D00.
bool RBProfile::IsSongUploadRetryWaiting() const {
    return TheTimeMgr->mRealTime.SplitMs() - mSongUploadJob.mStartTime < kSongUploadRetryMs;
}

// Reconstructed from eboot.elf at 0xD58D70.
unsigned char RBProfile::GetSongRating(int songId) const {
    auto& ratings = const_cast<eastl::map<int, unsigned char>&>(mSongRatings);
    const auto it = ratings.find(songId);
    return it != ratings.end() ? it->second : 0;
}

// Reconstructed from eboot.elf at 0xD58DF0. A rated song counts up to 5; a
// new one starts at 1.
bool RBProfile::IncreaseSongRating(int songId) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    bool changed = true;
    const auto it = mSongRatings.find(songId);
    if (it != mSongRatings.end()) {
        if (it->second > kMaxSongRating - 1) {
            changed = false;
        } else {
            ++it->second;
        }
    } else {
        mSongRatings[songId] = 1;
    }
    if (changed) {
        MakeDirty();
    }
    MemPopHeap();
    return changed;
}

// Reconstructed from eboot.elf at 0xD58F90. A rating that reaches 0 is
// dropped.
bool RBProfile::DecreaseSongRating(int songId) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    bool changed = false;
    const auto it = mSongRatings.find(songId);
    if (it != mSongRatings.end() && it->second != 0) {
        if (--it->second == 0) {
            mSongRatings.erase(it);
        }
        MakeDirty();
        changed = true;
    }
    MemPopHeap();
    return changed;
}

// Reconstructed from eboot.elf at 0xD590C0.
bool RBProfile::OutfitPieceIsUnlocked(const ResourcePath& piece) const {
    return mRockShopData.OutfitPieceIsUnlocked(piece);
}

// Reconstructed from eboot.elf at 0xD590D0.
bool RBProfile::OutfitPieceIsOwned(const ResourcePath& piece) const {
    return mRockShopData.OutfitPieceIsOwned(piece);
}

// Reconstructed from eboot.elf at 0xD590E0.
void RBProfile::UnlockOutfitPiece(const ResourcePath& piece) {
    mRockShopData.UnlockOutfitPiece(piece);
}

// Reconstructed from eboot.elf at 0xD590F0.
void RBProfile::AwardOutfitPiece(const ResourcePath& piece) {
    mRockShopData.AwardOutfitPiece(piece);
}

// Reconstructed from eboot.elf at 0xD59100.
void RBProfile::PurchaseOutfitPiece(const ResourcePath& piece) {
    mRockShopData.PurchaseOutfitPiece(piece);
}

// Reconstructed from eboot.elf at 0xD59110.
bool RBProfile::HasSeenId(Symbol id) const {
    return mSeenIds.HasSeenId(id);
}

// Reconstructed from eboot.elf at 0xD59120.
bool RBProfile::MarkIdSeen(Symbol id) {
    return mSeenIds.MarkIdSeen(id);
}

// Reconstructed from eboot.elf at 0xD59130.
void RBProfile::ClearSeenIds() {
    mSeenIds.Clear();
    mSeenIds.MakeDirty();
}

// Reconstructed from eboot.elf at 0xD59170.
void RBProfile::ClearClan() {
    mRivalsPlayerProfile.ClearClan();
    MakeDirty();
}

// Reconstructed from eboot.elf at 0xD591A0. The empty string the binary
// builds is not passed on.
void RBProfile::ClearClanName() {
    const String name("");
    mRivalsPlayerProfile.ClearClan();
    MakeDirty();
}

// Reconstructed from eboot.elf at 0xD59220.
DataNode RBProfile::OnMsg(const ConnectionStatusChangedMsg& msg) {
    static_cast<void>(msg);
    mNeedsOnlineCheck = true;
    return DataNode(0);
}

// Reconstructed from eboot.elf at 0xD59240.
bool ProfileAlphaSort(const RBProfile* a, const RBProfile* b) {
    return strcmp(a->GetPlatformName(), b->GetPlatformName()) < 0;
}

// Reconstructed from eboot.elf at 0xD59300.
DataNode RBProfile::Handle(DataArray* msg, bool warn) {
    if (msg->Sym(1) == ConnectionStatusChangedMsg::Event()) {
        const DataNode result = OnMsg(ConnectionStatusChangedMsg(msg));
        if (result.Type() != kDataUnhandled) {
            return result;
        }
    }
    return Profile::Handle(msg, warn);
}

// Reconstructed from eboot.elf at 0xD59470.
const char* RBProfile::GetSaveFileName() const {
    return "rb_profile.bin";
}
