#pragma once

#include <cstddef>

#include <np/np_trophy.h>

#include "os/memory/MemMgr.h"
#include "os/threading/CritSec.h"
#include "rb_meta/persistentdata/RBAchPersistentData.h"
#include "rb_meta/persistentdata/RBAchUtl.h"
#include "utl/containers/Vector.h"
#include "utl/threading/ThreadCall.h"

class BinStream;
class RBProfile;

// The PS4 half of a profile's achievement progress
// (rb_meta/RBAchPersistentData_PS4.o, 0xF02810-0xF0498B): the trophy
// context and handle of the profile's user, the song and campaign progress
// and four counters the trophies need, and a threaded trophy unlocker that
// sends the queued trophies on the "Hmx ThreadCall" thread. The profile
// creates it from the "profile" heap and owns it through its base
// (RBProfile::mAchData).
//
// The vtable group at 0x198F548 has the primary vtable (0x198F558, 38
// slots: PersistentData's 7, then the interface's overrides in the order
// below) and the interface's (0x198F698, offset 24, 31 slots of thunks).
// The object is 512 bytes.
class RBAchPersistentData_PS4 : public RBAchPersistentData {
public:
    // The game's trophies, by their id in the trophy set (62, with the
    // platinum). The ids are used as numbers; their names are not known.
    // Name in the reference map; the enumerator is not.
    enum TrophyId {
        kNumTrophies = 62,
    };

    // Sends the queued trophies on the "Hmx ThreadCall" thread, one call at
    // a time. The vtable at 0x198F790 has ThreadCallback's four slots (the
    // implicit destructors 0xF02B50 and 0xF04730); the object is 24 bytes.
    class ThreadedTrophyUnlocker : public ThreadCallback {
    public:
        // Inlined into the owner's constructor. Name not in the reference
        // map.
        explicit ThreadedTrophyUnlocker(RBAchPersistentData_PS4* owner)
            : mOwner(owner), mRunning(false) {}

        // Slot 2: sends the queued trophies, then empties the queue. A
        // kNumTrophies entry stops it and leaves the queue.
        int ThreadStart() override;  // 0xF044F0
        // Slot 3: starts again while trophies are queued.
        void ThreadDone(int result) override;  // 0xF046C0

        // Field names are not in the reference map.
        RBAchPersistentData_PS4* mOwner;
        // Set while a call is queued or running.
        bool mRunning;
    };

    // Allocated from the "profile" heap; the profile's constructor inlines
    // it.
    static void* operator new(unsigned long size) {
        static const long sHeap = MemFindHeap("profile");
        MemPushHeap(sHeap);
        void* const allocation = MemAlloc(size, "RBAchPersistentData_PS4", 0);
        MemPopHeap();
        return allocation;
    }
    DELETE_OVERLOAD

    // Creates and registers the trophy context of the profile's user and
    // reads the trophies already unlocked.
    explicit RBAchPersistentData_PS4(RBProfile* profile);  // 0xF02810
    // Slots 0-1: 0xF029E0, 0xF02B60. Empties the queue, waits for the
    // unlocker and destroys the trophy handle and context.
    ~RBAchPersistentData_PS4() override;

    // Slot 5: a revision-6 block.
    void Save(BinStream& stream) const override;  // 0xF02B80
    // Slot 6.
    void Load(BinStream& stream) override;  // 0xF02CC0

    // Slots 7-37, the interface's overrides; the interface's thunks are at
    // 0xF043E0 and 0xF03240-0xF04270. Most events are ignored in the game
    // mode kNoAchievementsGameMode, and some while autokick is on.
    // Slot 7: presents the queued trophies unless a player is in the way.
    // Not reconstructed: it inlines _TryPresentPendingTrophies(false).
    void PushPendingAchievements() override;  // 0xF04280
    void LogVocalImprovHit() override;          // 0xF02FC0
    void LogTripleAwesomeHit() override;        // 0xF03280
    void LogPerfectDynamicFill() override;      // 0xF03300
    void LogPerfectImprovGuitarSolo(int gameMode) override;  // 0xF033C0
    void LogMaxOverdriveMultiplier() override;  // 0xF03400
    void LogImprovSoloPerformance(const RBAch::ImprovSoloPerformanceArgs& args) override;  // 0xF034A0
    void LogPlayerFailure(const RBAch::PlayerFailureArgs& args) override;  // 0xF03540
    // Not reconstructed: it reads the game's Stats (not modelled).
    void LogMidSongStats(const RBAch::SongStatsArgs& args) override;  // 0xF03600
    // Logs the statistics as mid-song ones too. Not reconstructed: it reads
    // the game's Stats (not modelled).
    void LogFinalizedStats(const RBAch::FinalizedSongStatsArgs& args) override;  // 0xF03760
    void LogSongPerformed(const RBAch::SongPerformanceArgs& args) override;  // 0xF03890
    void LogGigStart() override;                 // 0xF03B30
    // Counts the decisions in a row that were false.
    void LogEncoreDecision(bool decision) override;   // 0xF03B50
    // Counted as an encore decision.
    void LogRequestDecision(bool decision) override;  // 0xF03BF0
    void LogSplitVote(int vote) override;             // 0xF03C20
    void LogSongStagePresence(const RBAch::SongStagePresenceArgs& args) override;  // 0xF03C50
    void LogGigPerformed(const RBAch::GigPerformanceArgs& args) override;  // 0xF03D10
    void LogCampaignGigPerformed(const RBAch::CampaignGigPerformanceArgs& args) override;  // 0xF03DE0
    void LogFanGigUnlocked(const RBAch::FanGigUnlockArgs& args) override;  // 0xF03F00
    void LogCampaignCompleted() override;              // 0xF03FB0
    void LogQuickplayCareerScore(long score) override;  // 0xF03FD0
    void LogCashSpent(int cash) override;              // 0xF04010
    void LogRockShopItemAwarded() override;            // 0xF04090
    void LogChallengeWon() override;                   // 0xF040B0
    void LogChallengeWonTopTier() override;            // 0xF040D0
    void LogPlayerLevel(int level) override;           // 0xF040F0
    void LogSkillRating(int skill) override;           // 0xF04170
    void LogCalibrationPerformed() override;           // 0xF04200
    void LogEntitlement16Owned() override;             // 0xF04220
    void LogEntitlement15Owned() override;             // 0xF04240
    void LogEntitlement17Owned() override;             // 0xF04260

    // The number of trophies unlocked. Name not in the reference map.
    int GetNumTrophiesUnlocked() const;  // 0xF02F80
    // kNumTrophies. Name not in the reference map.
    int GetNumTrophies() const;  // 0xF02FB0

    // Queues the trophy and presents the queue unless a player is in the
    // way. Not reconstructed: it inlines _TryPresentPendingTrophies(false),
    // which reads the game (not modelled). The map's
    // _UnlockTrophy(RBAchPersistentData_PS4::TrophyId).
    void _UnlockTrophy(TrophyId trophy);  // 0xF03000
    // Starts the unlocker, unless `force` is clear and _CanPresentTrophyNow
    // fails; whether it started. Not reconstructed: it inlines
    // _CanPresentTrophyNow.
    bool _TryPresentPendingTrophies(bool force);  // 0xF04330
    // False while the game (0x1AEF770) runs and one of its active players
    // is in state 3. Not reconstructed: the game and its players are not
    // modelled.
    bool _CanPresentTrophyNow() const;  // 0xF04490
    // Unlocks the trophy unless it is unlocked already, and records it when
    // the platform unlocked it or reports it unlocked.
    void _SendTrophyUnlock(TrophyId trophy);  // 0xF045D0

    // Field names below are not in the reference map.
    SceNpTrophyContext mTrophyContext;
    SceNpTrophyHandle mTrophyHandle;
    RBAch::SonglistProficiency mSonglistProficiency;
    RBAch::CampaignData mCampaignData;
    // Saved from revision 1; LogCashSpent's total.
    int mCashSpent;
    // Saved from revision 2.
    int mPerfectDynamicFills;
    // Saved from revision 3, and reset when loaded from an older one than
    // 4.
    int mFanGigsUnlocked;
    // Saved from revision 4; the encore and request decisions in a row
    // that were false.
    int mFalseDecisionStreak;
    // Guards the queue and the unlocker's state.
    CritSec mTrophyLock;
    ThreadedTrophyUnlocker mUnlocker;
    eastl::vector<TrophyId> mPendingTrophies;
    SceNpTrophyFlagArray mUnlockedTrophies;
};

static_assert(offsetof(RBAchPersistentData_PS4::ThreadedTrophyUnlocker, mOwner) == 8);
static_assert(offsetof(RBAchPersistentData_PS4::ThreadedTrophyUnlocker, mRunning) == 16);
static_assert(sizeof(RBAchPersistentData_PS4::ThreadedTrophyUnlocker) == 24);
static_assert(offsetof(RBAchPersistentData_PS4, mTrophyContext) == 40);
static_assert(offsetof(RBAchPersistentData_PS4, mTrophyHandle) == 44);
static_assert(offsetof(RBAchPersistentData_PS4, mSonglistProficiency) == 48);
static_assert(offsetof(RBAchPersistentData_PS4, mCampaignData) == 264);
static_assert(offsetof(RBAchPersistentData_PS4, mCashSpent) == 408);
static_assert(offsetof(RBAchPersistentData_PS4, mFalseDecisionStreak) == 420);
static_assert(offsetof(RBAchPersistentData_PS4, mTrophyLock) == 424);
static_assert(offsetof(RBAchPersistentData_PS4, mUnlocker) == 440);
static_assert(offsetof(RBAchPersistentData_PS4, mPendingTrophies) == 464);
static_assert(offsetof(RBAchPersistentData_PS4, mUnlockedTrophies) == 496);
static_assert(sizeof(RBAchPersistentData_PS4) == 512);
