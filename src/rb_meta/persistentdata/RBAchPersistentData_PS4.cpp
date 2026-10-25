// rb_meta/RBAchPersistentData_PS4.o (0xF02810 to 0xF0498B). The object also
// holds the compiler's copies of the Revision operations Load uses:
// AsOfOp<RBAch::SonglistProficiency> (vtable 0x198F7C0, destructors
// 0xF02F50 and 0xF04820, Read 0xF04830), AsOfOp<RBAch::CampaignData>
// (0x198F7E8: 0xF02F60, 0xF04850, Read 0xF04860) and
// AtOp<eastl::vector<Symbol>> (0x198F810: 0xF02F70, 0xF04880, Read
// 0xF04890), and SonglistProficiency's implicit destructor (0xF04740). The
// static initializer at 0xF04950 sets the shared header globals every
// rb_meta object initializes; it is not modelled.
//
// Not reconstructed: LogMidSongStats (0xF03600) and LogFinalizedStats
// (0xF03760), which read the game's Stats, and PushPendingAchievements
// (0xF04280), _UnlockTrophy (0xF03000), _TryPresentPendingTrophies
// (0xF04330) and _CanPresentTrophyNow (0xF04490), which read the game and
// its players. None of them is modelled.
#include "rb_meta/persistentdata/RBAchPersistentData_PS4.h"

#include <cstring>

#include "os/memory/MemMgr.h"
#include "rb_meta/profiles/RBProfile.h"
#include "utl/messages/MsgSink.h"
#include "utl/streams/RevisionStream.h"

// The game's data (rb_game/RBGameData.o), which is not modelled. The game
// mode is an RBGameMode. Only the member the trophies use is declared.
class RBGameData : public MsgSource {
public:
    int GetGameMode() const;  // 0x965E50
};

extern RBGameData theGameData;  // 0x1AE1818

// The game mode whose events earn no trophies. Name not in the reference
// map; weak: the mode is probably practice.
static constexpr int kNoAchievementsGameMode = 7;

// Saves and loads a piece of data through its virtual Save and Load, as the
// Revision operations and Save do. Names not in the reference map; inlined.
static inline BinStream& operator<<(BinStream& stream, const PersistentData& data) {
    data.Save(stream);
    return stream;
}

static inline BinStream& operator>>(BinStream& stream, PersistentData& data) {
    data.Load(stream);
    return stream;
}

// Reads a count and then the symbols, as AtOp<eastl::vector<Symbol>>::Read
// (0xF04890) inlines it. Name not in the reference map.
template <typename T>
inline BinStream& operator>>(BinStream& stream, eastl::vector<T>& values) {
    unsigned int count;
    stream.ReadEndian(&count, sizeof(count));
    values.resize(count);
    for (T* value = values.begin(); value != values.end(); ++value) {
        stream >> *value;
    }
    return stream;
}

// Reconstructed from eboot.elf at 0xF02810.
RBAchPersistentData_PS4::RBAchPersistentData_PS4(RBProfile* profile)
    : RBAchPersistentData(profile),
      mTrophyContext(SCE_NP_TROPHY_INVALID_CONTEXT),
      mTrophyHandle(SCE_NP_TROPHY_INVALID_HANDLE),
      mSonglistProficiency(this),
      mCampaignData(this),
      mCashSpent(0),
      mPerfectDynamicFills(0),
      mFanGigsUnlocked(0),
      mFalseDecisionStreak(0),
      mUnlocker(this),
      mUnlockedTrophies() {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    SCE_NP_TROPHY_FLAG_ZERO(&mUnlockedTrophies);
    sceNpTrophyCreateContext(&mTrophyContext, mProfile->mPadNum, 0, 0);
    sceNpTrophyCreateHandle(&mTrophyHandle);
    sceNpTrophyRegisterContext(mTrophyContext, mTrophyHandle, 0);
    unsigned int count = 0;
    sceNpTrophyGetTrophyUnlockState(mTrophyContext, mTrophyHandle, &mUnlockedTrophies, &count);
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xF029E0. The deleting destructor is at
// 0xF02B60.
RBAchPersistentData_PS4::~RBAchPersistentData_PS4() {
    mTrophyLock.Enter();
    mPendingTrophies.clear();
    mTrophyLock.Exit();
    bool running;
    do {
        mTrophyLock.Enter();
        running = mUnlocker.mRunning;
        mTrophyLock.Exit();
    } while (running);
    sceNpTrophyDestroyHandle(mTrophyHandle);
    sceNpTrophyDestroyContext(mTrophyContext);
}

// Reconstructed from eboot.elf at 0xF02B80.
void RBAchPersistentData_PS4::Save(BinStream& stream) const {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 6, 0);
        revisionStream.BeginWrite();
        revisionStream << mSonglistProficiency;
        revisionStream << mCampaignData;
        int value = mCashSpent;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mPerfectDynamicFills;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mFanGigsUnlocked;
        revisionStream.WriteEndian(&value, sizeof(value));
        value = mFalseDecisionStreak;
        revisionStream.WriteEndian(&value, sizeof(value));
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xF02CC0. Revision 5 alone saved a list
// of symbols, which is read and dropped.
void RBAchPersistentData_PS4::Load(BinStream& stream) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    {
        RevisionStream revisionStream(stream, 6, 0);
        if (revisionStream.BeginRead()) {
            revisionStream >> Revision::AsOfOp<RBAch::SonglistProficiency>(&mSonglistProficiency, 0);
            revisionStream >> Revision::AsOfOp<RBAch::CampaignData>(&mCampaignData, 6);
            revisionStream >> Revision::AsOfOp<int>(&mCashSpent, 1);
            revisionStream >> Revision::AsOfOp<int>(&mPerfectDynamicFills, 2);
            revisionStream >> Revision::AsOfOp<int>(&mFanGigsUnlocked, 3);
            revisionStream >> Revision::AsOfOp<int>(&mFalseDecisionStreak, 4);
            eastl::vector<Symbol> dropped;
            revisionStream >> Revision::AtOp<eastl::vector<Symbol>>(&dropped, 5);
            if (revisionStream.mReadRevision < 4) {
                mFanGigsUnlocked = 0;
            }
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xF02F80.
int RBAchPersistentData_PS4::GetNumTrophiesUnlocked() const {
    int count = 0;
    for (int trophy = 0; trophy < kNumTrophies; ++trophy) {
        if (SCE_NP_TROPHY_FLAG_ISSET(trophy, &mUnlockedTrophies)) {
            ++count;
        }
    }
    return count;
}

// Reconstructed from eboot.elf at 0xF02FB0.
int RBAchPersistentData_PS4::GetNumTrophies() const {
    return kNumTrophies;
}

// Reconstructed from eboot.elf at 0xF02FC0.
void RBAchPersistentData_PS4::LogVocalImprovHit() {
    if (theGameData.GetGameMode() != kNoAchievementsGameMode) {
        _UnlockTrophy(TrophyId(40));
    }
}

// Reconstructed from eboot.elf at 0xF03280.
void RBAchPersistentData_PS4::LogTripleAwesomeHit() {
    if (theGameData.GetGameMode() != kNoAchievementsGameMode) {
        _UnlockTrophy(TrophyId(13));
    }
}

// Reconstructed from eboot.elf at 0xF03300.
void RBAchPersistentData_PS4::LogPerfectDynamicFill() {
    if (theGameData.GetGameMode() == kNoAchievementsGameMode || _IsAutokickPlayer()) {
        return;
    }
    ++mPerfectDynamicFills;
    MakeDirty();
    if (mPerfectDynamicFills >= 10) {
        _UnlockTrophy(TrophyId(15));
    }
}

// Reconstructed from eboot.elf at 0xF033C0.
void RBAchPersistentData_PS4::LogPerfectImprovGuitarSolo(int gameMode) {
    if (gameMode >= 1 && gameMode <= 3) {
        _UnlockTrophy(TrophyId(39));
    }
}

// Reconstructed from eboot.elf at 0xF03400.
void RBAchPersistentData_PS4::LogMaxOverdriveMultiplier() {
    if (theGameData.GetGameMode() == kNoAchievementsGameMode || _IsAutokickInBand()) {
        return;
    }
    _UnlockTrophy(TrophyId(19));
}

// Reconstructed from eboot.elf at 0xF034A0.
void RBAchPersistentData_PS4::LogImprovSoloPerformance(const RBAch::ImprovSoloPerformanceArgs& args) {
    if (theGameData.GetGameMode() == kNoAchievementsGameMode) {
        return;
    }
    if (args.mTappedFraction >= 0.8f) {
        _UnlockTrophy(TrophyId(49));
    }
}

// Reconstructed from eboot.elf at 0xF03540.
void RBAchPersistentData_PS4::LogPlayerFailure(const RBAch::PlayerFailureArgs& args) {
    if (theGameData.GetGameMode() == kNoAchievementsGameMode) {
        return;
    }
    if (args.mFailureTrophy) {
        _UnlockTrophy(TrophyId(46));
    }
    if (args.mVocalFailureTrophy &&
        (args.mScoreType == kScoreVocals || args.mScoreType == kScoreHarmony)) {
        _UnlockTrophy(TrophyId(48));
    }
}

// Reconstructed from eboot.elf at 0xF03890.
void RBAchPersistentData_PS4::LogSongPerformed(const RBAch::SongPerformanceArgs& args) {
    if (theGameData.GetGameMode() == kNoAchievementsGameMode) {
        return;
    }
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    const bool autokick = _IsAutokickPlayer();
    const int stars = args.mStars;
    if (args.mGameMode == 1) {
        if (stars >= 5 && !autokick) {
            _UnlockTrophy(TrophyId(1));
            if (stars >= 6) {
                _UnlockTrophy(TrophyId(5));
            }
        }
        if (args.mScore >= 1000000 && !_IsAutokickInBand()) {
            _UnlockTrophy(TrophyId(16));
        }
    }
    if (args.mGameMode == 6 && !autokick) {
        _UnlockTrophy(TrophyId(51));
        if (stars >= 6) {
            _UnlockTrophy(TrophyId(52));
        }
    }
    if (args.mGameMode == 2) {
        const bool fewStars = stars < 5;
        if (args.mSessionFlag && !fewStars) {
            _UnlockTrophy(TrophyId(22));
        }
        if (args.mSessionFlag2 && !fewStars) {
            _UnlockTrophy(TrophyId(23));
        }
    }
    if (args.mGameMode == 3 && args.mSessionFlag2) {
        _UnlockTrophy(TrophyId(42));
    }
    if (args.mGameMode == 2 || args.mGameMode == 3) {
        const bool fewStars = stars < 6;
        if (args.mSessionFlag && !fewStars && !autokick) {
            _UnlockTrophy(TrophyId(7));
        }
        if (args.mSessionFlag2 && !fewStars && !autokick) {
            _UnlockTrophy(TrophyId(8));
        }
    }
    if (args.mGameMode == 4 && args.mScoreType == kScoreGuitar &&
        strcmp(args.mSong.Str(), "igstutorial1") == 0) {
        _UnlockTrophy(TrophyId(38));
    }
    mSonglistProficiency.AddPerformance(args);
    if (mSonglistProficiency.IsRockFan() && !autokick) {
        _UnlockTrophy(TrophyId(2));
    }
    if (mSonglistProficiency.IsRockMaster() && !autokick) {
        _UnlockTrophy(TrophyId(3));
    }
    if (mSonglistProficiency.IsRockImmortal() && !autokick) {
        _UnlockTrophy(TrophyId(4));
    }
    if (mSonglistProficiency.IsGoldStar() && !autokick) {
        _UnlockTrophy(TrophyId(6));
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xF03B30.
void RBAchPersistentData_PS4::LogGigStart() {}

// Reconstructed from eboot.elf at 0xF03B50.
void RBAchPersistentData_PS4::LogEncoreDecision(bool decision) {
    mFalseDecisionStreak = decision ? 0 : mFalseDecisionStreak + 1;
    MakeDirty();
    if (mFalseDecisionStreak >= 3) {
        _UnlockTrophy(TrophyId(50));
    }
}

// Reconstructed from eboot.elf at 0xF03BF0.
void RBAchPersistentData_PS4::LogRequestDecision(bool decision) {
    LogEncoreDecision(decision);
}

// Reconstructed from eboot.elf at 0xF03C20.
void RBAchPersistentData_PS4::LogSplitVote(int vote) {
    if (vote >= 4) {
        _UnlockTrophy(TrophyId(47));
    }
}

// Reconstructed from eboot.elf at 0xF03C50.
void RBAchPersistentData_PS4::LogSongStagePresence(const RBAch::SongStagePresenceArgs& args) {
    if (_IsAutokickInBand()) {
        return;
    }
    if (args.mEvents->size() >= 5) {
        _UnlockTrophy(TrophyId(36));
    }
    if (args.mNumEvents >= 5) {
        _UnlockTrophy(TrophyId(37));
    }
}

// Reconstructed from eboot.elf at 0xF03D10.
void RBAchPersistentData_PS4::LogGigPerformed(const RBAch::GigPerformanceArgs& args) {
    if (!_IsAutokickInBand() && args.mResult > 0) {
        _UnlockTrophy(TrophyId(34));
        if (args.mResult >= 2) {
            _UnlockTrophy(TrophyId(35));
        }
    }
    if (args.mGameMode == 3 && args.mCount >= 5) {
        _UnlockTrophy(TrophyId(43));
    }
}

// Reconstructed from eboot.elf at 0xF03DE0.
void RBAchPersistentData_PS4::LogCampaignGigPerformed(const RBAch::CampaignGigPerformanceArgs& args) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    mCampaignData.LogCampaignGigPerformed(args);
    if (args.mCity != args.mHomeCity) {
        _UnlockTrophy(TrophyId(24));
    }
    if (args.mSpecialGig) {
        _UnlockTrophy(TrophyId(25));
    }
    if (mCampaignData.mFansEarned >= 150000) {
        _UnlockTrophy(TrophyId(26));
    }
    if (mCampaignData.mCashEarned >= 300000) {
        _UnlockTrophy(TrophyId(21));
    }
    if (mCampaignData.NumUniqueCampaignGigsAttempted() >= 50) {
        _UnlockTrophy(TrophyId(28));
    }
    if (args.mBonusSet && mCampaignData.NumUniqueBonusSetsAttempted() >= 10) {
        _UnlockTrophy(TrophyId(27));
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xF03F00.
void RBAchPersistentData_PS4::LogFanGigUnlocked(const RBAch::FanGigUnlockArgs& args) {
    if (!args.mUnlocked) {
        return;
    }
    ++mFanGigsUnlocked;
    MakeDirty();
    _UnlockTrophy(TrophyId(29));
    if (mFanGigsUnlocked >= 8) {
        _UnlockTrophy(TrophyId(30));
    }
}

// Reconstructed from eboot.elf at 0xF03FB0.
void RBAchPersistentData_PS4::LogCampaignCompleted() {
    _UnlockTrophy(TrophyId(31));
}

// Reconstructed from eboot.elf at 0xF03FD0.
void RBAchPersistentData_PS4::LogQuickplayCareerScore(long score) {
    if (score >= 10000000) {
        _UnlockTrophy(TrophyId(17));
    }
}

// Reconstructed from eboot.elf at 0xF04010.
void RBAchPersistentData_PS4::LogCashSpent(int cash) {
    mCashSpent += cash;
    MakeDirty();
    if (mCashSpent >= 100000) {
        _UnlockTrophy(TrophyId(32));
    }
}

// Reconstructed from eboot.elf at 0xF04090.
void RBAchPersistentData_PS4::LogRockShopItemAwarded() {
    _UnlockTrophy(TrophyId(33));
}

// Reconstructed from eboot.elf at 0xF040B0.
void RBAchPersistentData_PS4::LogChallengeWon() {
    _UnlockTrophy(TrophyId(53));
}

// Reconstructed from eboot.elf at 0xF040D0.
void RBAchPersistentData_PS4::LogChallengeWonTopTier() {
    _UnlockTrophy(TrophyId(54));
}

// Reconstructed from eboot.elf at 0xF040F0.
void RBAchPersistentData_PS4::LogPlayerLevel(int level) {
    if (level >= 10) {
        _UnlockTrophy(TrophyId(55));
        if (level >= 50) {
            _UnlockTrophy(TrophyId(56));
        }
    }
}

// Reconstructed from eboot.elf at 0xF04170.
void RBAchPersistentData_PS4::LogSkillRating(int skill) {
    if (skill >= 700) {
        _UnlockTrophy(TrophyId(57));
        if (skill >= 900) {
            _UnlockTrophy(TrophyId(58));
        }
    }
}

// Reconstructed from eboot.elf at 0xF04200.
void RBAchPersistentData_PS4::LogCalibrationPerformed() {
    _UnlockTrophy(TrophyId(44));
}

// Reconstructed from eboot.elf at 0xF04220.
void RBAchPersistentData_PS4::LogEntitlement16Owned() {
    _UnlockTrophy(TrophyId(59));
}

// Reconstructed from eboot.elf at 0xF04240.
void RBAchPersistentData_PS4::LogEntitlement15Owned() {
    _UnlockTrophy(TrophyId(60));
}

// Reconstructed from eboot.elf at 0xF04260.
void RBAchPersistentData_PS4::LogEntitlement17Owned() {
    _UnlockTrophy(TrophyId(61));
}

// Reconstructed from eboot.elf at 0xF044F0. The queue is read again after
// each trophy, outside the lock, so that trophies queued meanwhile are
// sent too.
int RBAchPersistentData_PS4::ThreadedTrophyUnlocker::ThreadStart() {
    CritSec& lock = mOwner->mTrophyLock;
    lock.Enter();
    for (unsigned long i = 0; i < mOwner->mPendingTrophies.size(); ++i) {
        const TrophyId trophy = mOwner->mPendingTrophies.begin()[i];
        lock.Exit();
        if (trophy == kNumTrophies) {
            return 0;
        }
        mOwner->_SendTrophyUnlock(trophy);
        lock.Enter();
    }
    mOwner->mPendingTrophies.clear();
    lock.Exit();
    return 0;
}

// Reconstructed from eboot.elf at 0xF045D0.
void RBAchPersistentData_PS4::_SendTrophyUnlock(TrophyId trophy) {
    static const long sHeap = MemFindHeap("profile");
    MemPushHeap(sHeap);
    if (!SCE_NP_TROPHY_FLAG_ISSET(trophy, &mUnlockedTrophies)) {
        SceNpTrophyId platinum = SCE_NP_TROPHY_INVALID_TROPHY_ID;
        const int result = sceNpTrophyUnlockTrophy(mTrophyContext, mTrophyHandle, trophy, &platinum);
        if (result >= 0 || result == SCE_NP_TROPHY_ERROR_TROPHY_ALREADY_UNLOCKED) {
            SCE_NP_TROPHY_FLAG_SET(trophy, &mUnlockedTrophies);
        }
    }
    MemPopHeap();
}

// Reconstructed from eboot.elf at 0xF046C0.
void RBAchPersistentData_PS4::ThreadedTrophyUnlocker::ThreadDone(int result) {
    static_cast<void>(result);
    CritSec& lock = mOwner->mTrophyLock;
    lock.Enter();
    mRunning = false;
    if (!mOwner->mPendingTrophies.empty()) {
        ThreadCall(this);
    }
    lock.Exit();
}
