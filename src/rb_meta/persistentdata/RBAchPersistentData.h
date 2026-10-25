#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "persistence/core/PersistentData.h"
#include "rb_game/core/Defines.h"
#include "utl/containers/Vector.h"
#include "utl/text/Symbol.h"

class BinStream;
class RBProfile;
class Stats;

// The game's achievement events and their arguments (the map's RBAch
// namespace). The structures' names are the map's; their field names are
// not in the reference map and rest only on how RBAchPersistentData_PS4
// reads them, so they are weak. Fields the platform half does not read are
// kept as opaque bytes.
namespace RBAch {

// A finished improvised guitar solo.
struct ImprovSoloPerformanceArgs {
    unsigned char mOther[16];  // Not reconstructed.
    // The share of the solo's notes that were tapped (0 to 1).
    float mTappedFraction;
};

static_assert(offsetof(ImprovSoloPerformanceArgs, mTappedFraction) == 16);

// A player who failed out of a song.
struct PlayerFailureArgs {
    ScoreType mScoreType;
    unsigned char mOther[4];  // Not reconstructed.
    // The two conditions the failure trophies test; their meaning is not
    // known.
    bool mFailureTrophy;
    bool mVocalFailureTrophy;
};

static_assert(offsetof(PlayerFailureArgs, mFailureTrophy) == 8);
static_assert(offsetof(PlayerFailureArgs, mVocalFailureTrophy) == 9);

// A player's statistics during a song. Stats is the game's statistics
// tracker (rb_game/Stats.o), which is not modelled.
struct SongStatsArgs {
    const Stats* mStats;
    ScoreType mScoreType;
    // 3 is expert.
    int mDifficulty;
};

static_assert(sizeof(SongStatsArgs) == 16);

// A player's statistics at the end of a song.
struct FinalizedSongStatsArgs : SongStatsArgs {};

// A song a player finished.
struct SongPerformanceArgs {
    int mSongId;
    Symbol mSong;
    ScoreType mScoreType;
    int mDifficulty;
    // The game's RBGameMode (rb_game, not modelled): 1 quickplay, 6 a
    // campaign gig, 7 ignored by the trophies (see
    // kNoAchievementsGameMode).
    int mGameMode;
    int mScore;
    // 6 is gold stars.
    int mStars;
    // A value of the player (0xA5DA00); its meaning is not known.
    int mPlayerValue;
    // 0 a Rock Band 4 song, 1 a Rivals song, 2 any other.
    int mSongSource;
    // Two flags of the session object at 0x1AE4938 (its byte at +180, and
    // whether its +178 is set); their meaning is not known.
    bool mSessionFlag;
    bool mSessionFlag2;
};

static_assert(offsetof(SongPerformanceArgs, mSong) == 8);
static_assert(offsetof(SongPerformanceArgs, mGameMode) == 24);
static_assert(offsetof(SongPerformanceArgs, mStars) == 32);
static_assert(offsetof(SongPerformanceArgs, mSessionFlag) == 44);

// A song's stage presence events. The events are StagePresence::EventDefn
// (rb_game, not modelled); the element type here only has their size.
struct SongStagePresenceArgs {
    const eastl::vector<const void*>* mEvents;
    unsigned int mNumEvents;
};

static_assert(offsetof(SongStagePresenceArgs, mNumEvents) == 8);

// A finished gig (set of songs).
struct GigPerformanceArgs {
    // The result of 0x998B60 over the gig's songs; its meaning is not known.
    int mResult;
    int mGameMode;  // See SongPerformanceArgs::mGameMode.
    // A count taken from the gig's two song counters, clamped at zero.
    unsigned int mCount;
};

static_assert(sizeof(GigPerformanceArgs) == 12);

// A finished campaign gig.
struct CampaignGigPerformanceArgs {
    // The trophy at 24 needs the two to differ.
    Symbol mCity;
    Symbol mHomeCity;
    Symbol mGig;
    // Added to RBAch::CampaignData's totals.
    int mFansEarned;
    int mCashEarned;
    bool mSpecialGig;
    bool mBonusSet;
};

static_assert(offsetof(CampaignGigPerformanceArgs, mFansEarned) == 24);
static_assert(offsetof(CampaignGigPerformanceArgs, mSpecialGig) == 32);
static_assert(offsetof(CampaignGigPerformanceArgs, mBonusSet) == 33);

// A fan-requested gig.
struct FanGigUnlockArgs {
    Symbol mGig;
    bool mUnlocked;
};

static_assert(offsetof(FanGigUnlockArgs, mUnlocked) == 8);

}  // namespace RBAch

// The achievement events the game reports. In the map's build these were
// RBAchPersistentData's own virtual methods; this build moves them into an
// interface at RBAchPersistentData's offset 24. Every slot but the first is
// pure; PushPendingAchievements's empty body is at 0xF07390. The vtable as
// RBAchPersistentData's secondary one is at 0x198F918 (31 slots); the
// interface has no destructor. Name not in the reference map; the method
// names are the map's RBAchPersistentData_PS4 ones, matched by the order of
// their definitions and their bodies. Slots 23-29 are newer than the map and
// named from their callers (weak).
class IRBAchievementLogger {
public:
    // Slot 0.
    virtual void PushPendingAchievements() {}
    // Slots 1-3.
    virtual void LogVocalImprovHit() = 0;
    virtual void LogTripleAwesomeHit() = 0;
    virtual void LogPerfectDynamicFill() = 0;
    // Slot 4. The map's argument is an RBGameMode (rb_game, not modelled).
    virtual void LogPerfectImprovGuitarSolo(int gameMode) = 0;
    // Slots 5-10.
    virtual void LogMaxOverdriveMultiplier() = 0;
    virtual void LogImprovSoloPerformance(const RBAch::ImprovSoloPerformanceArgs& args) = 0;
    virtual void LogPlayerFailure(const RBAch::PlayerFailureArgs& args) = 0;
    virtual void LogMidSongStats(const RBAch::SongStatsArgs& args) = 0;
    virtual void LogFinalizedStats(const RBAch::FinalizedSongStatsArgs& args) = 0;
    virtual void LogSongPerformed(const RBAch::SongPerformanceArgs& args) = 0;
    // Slots 11-14.
    virtual void LogGigStart() = 0;
    virtual void LogEncoreDecision(bool decision) = 0;
    virtual void LogRequestDecision(bool decision) = 0;
    virtual void LogSplitVote(int vote) = 0;
    // Slots 15-19.
    virtual void LogSongStagePresence(const RBAch::SongStagePresenceArgs& args) = 0;
    virtual void LogGigPerformed(const RBAch::GigPerformanceArgs& args) = 0;
    virtual void LogCampaignGigPerformed(const RBAch::CampaignGigPerformanceArgs& args) = 0;
    virtual void LogFanGigUnlocked(const RBAch::FanGigUnlockArgs& args) = 0;
    virtual void LogCampaignCompleted() = 0;
    // Slots 20-22.
    virtual void LogQuickplayCareerScore(long score) = 0;
    virtual void LogCashSpent(int cash) = 0;
    virtual void LogRockShopItemAwarded() = 0;
    // Slots 23-24: a won challenge (any of "win_1" to "win_3") and a won
    // challenge at its top tier ("win_3"); the caller is at 0x932480. Names
    // not in the reference map.
    virtual void LogChallengeWon() = 0;
    virtual void LogChallengeWonTopTier() = 0;
    // Slot 25: the player's level, from the score submission results
    // ("player_level"). Name not in the reference map.
    virtual void LogPlayerLevel(int level) = 0;
    // Slot 26: the player's skill rating from the score submission results
    // ("skill"). Name not in the reference map.
    virtual void LogSkillRating(int skill) = 0;
    // Slots 27-29: the profile owns the entitlement (RBProfile's achievement
    // update at 0xD589C0 asks theRBEntitlementMgr for 16, 15 and 17 in turn
    // for slots 27, 28 and 29). Names not in the reference map.
    virtual void LogEntitlement16Owned() = 0;
    virtual void LogEntitlement15Owned() = 0;
    virtual void LogEntitlement17Owned() = 0;
    // Slot 30: SystemOptions::SetHasCalibrated's caller (0xDAC7E2) logs it
    // for every participating profile.
    virtual void LogCalibrationPerformed() = 0;
};

static_assert(sizeof(IRBAchievementLogger) == 8);

// The achievement progress a profile saves (rb_meta/RBAchPersistentData.o,
// 0xF07280-0xF073DB in this build, after RBAchUtl.o): the platform half
// derives from it, and the profile saves it through this type
// (Revision::AsOfOp<RBAchPersistentData> in the map). The vtable group at
// 0x198F8C8 has PersistentData's 7 slots (Save and Load pure) and the
// interface's (0x198F918, offset 24). The object is 40 bytes.
class RBAchPersistentData : public PersistentData, public IRBAchievementLogger {
public:
    DELETE_OVERLOAD

    explicit RBAchPersistentData(RBProfile* profile);  // 0xF07280
    // Slots 0-1: 0xF07370, 0xF07380.
    ~RBAchPersistentData() override;

    // Whether the profile is the one in band slot 1 (RBProfileMgr's
    // GetProfileFromSlotIdx) while the console's options use autokick, which
    // the player trophies exclude. Name not in the reference map.
    bool _IsAutokickPlayer() const;  // 0xF072B0
    // Whether autokick is on while the profile or any user holds band slot
    // 1, which the band trophies exclude. Name not in the reference map.
    bool _IsAutokickInBand() const;  // 0xF07300

    // The profile the progress belongs to. Field name not in the reference
    // map.
    RBProfile* mProfile;
};

static_assert(offsetof(RBAchPersistentData, mProfile) == 32);
static_assert(sizeof(RBAchPersistentData) == 40);
