#pragma once

#include <cstddef>

#include "entity/props/PropMetadata.h"
#include "os/threading/CritSec.h"
#include "utl/containers/List.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

class DataArray;
class DingoJob;

// The mini leaderboards the score submission results show. Name not in the
// reference map; the enumerator names are the texts of
// GetAllowedMiniLeaderboardTypeValues.
enum MiniLeaderboardType {
    kMiniLeaderboardTypeInvalid = -1,
    kMiniLeaderboardRankPoints = 0,
    kMiniLeaderboardAllTime = 1,
};

// The allowed values of a mini leaderboard type property.
eastl::vector<AllowedValue<int>> GetAllowedMiniLeaderboardTypeValues();  // 0xCB9FE0

// What the server returned for a pad's score submissions: the missions
// completed, the experience earned and its breakdown and multipliers, and
// the mini leaderboards (552 bytes). Its constructor (0xC4E100) and Reset
// (0xC4E2E0) are in the results screen's object (RBResultsStatsCom's,
// 0xC4E100-0xC4EAFB); its destructor is implicit. Name and field names not
// in the reference map; the parts nothing reconstructed reads are opaque.
struct RBScoreSubmissionResults {
    // An experience multiplier ("multipliers": "type", "multiplier").
    struct Multiplier {
        Symbol mType;
        float mMultiplier;
    };
    // A line of the experience breakdown ("xp_breakdown": "text", "value",
    // "popup_text").
    struct XpBreakdownLine {
        Symbol mText;
        Symbol mValue;
        Symbol mPopupText;
    };

    RBScoreSubmissionResults();  // 0xC4E100
    // Empties every result.
    void Reset();  // 0xC4E2E0

    unsigned char mSummary[44];  // Not reconstructed.
    // The size of "missions_completed".
    int mNumMissionsCompleted;
    eastl::list<Symbol> mMissionsCompleted;
    unsigned char mExperience[192];  // Not reconstructed.
    eastl::vector<Multiplier> mMultipliers;
    eastl::vector<XpBreakdownLine> mXpBreakdown;
    unsigned char mXpTotal[24];  // Not reconstructed.
    // A list the results fill from their entries; its contents are not
    // identified (weak element type).
    eastl::list<Symbol> mItems;
    unsigned char mLeaderboards[160];  // Not reconstructed.
};

static_assert(offsetof(RBScoreSubmissionResults, mNumMissionsCompleted) == 44);
static_assert(offsetof(RBScoreSubmissionResults, mMissionsCompleted) == 48);
static_assert(offsetof(RBScoreSubmissionResults, mMultipliers) == 272);
static_assert(offsetof(RBScoreSubmissionResults, mXpBreakdown) == 304);
static_assert(offsetof(RBScoreSubmissionResults, mItems) == 360);
static_assert(sizeof(RBScoreSubmissionResults) == 552);

// The results of a pad's recent score submissions for the mini leaderboards
// (rb_meta code at 0xCB9FE0-0xCBD7AF, after RBUpsellScreen; newer than the
// map, which has neither the object nor the class): a MsgSink that follows
// theRBNetMgr's "score_submissions_started" and
// "score_submissions_complete". The vtable at 0x1978960 (address point
// 0x1978970) has MsgSink's four slots; the object is 664 bytes. RBProfile
// keeps one (RBProfile::mMiniLeaderboardResults). The class name and the
// names below are not in the reference map; the evidence for them is weak.
class RBMiniLeaderboardResults : public MsgSink {
public:
    // A subscription that leaves its source when it is destroyed, after the
    // members declared later. Name not in the reference map.
    struct OwnedEventSink : MsgSource::EventSinkElem {
        OwnedEventSink() : MsgSource::EventSinkElem() {}
        ~OwnedEventSink() {
            if (mLink.mNext != &mLink && mLink.mPrev != &mLink) {
                mSource->RemoveSink(this);
            }
        }
    };

    explicit RBMiniLeaderboardResults(int padNum);  // 0xCBA1E0
    // Slots 0-1: 0xCBA3C0, 0xCBA590.
    ~RBMiniLeaderboardResults() override;
    // Slot 2: "score_submissions_complete" (node 2 the success, node 3 the
    // job) goes to OnScoreSubmissionsComplete, and
    // "score_submissions_started" to OnScoreSubmissionsStarted.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xCBD2B0

    // Forgets the missions completed. RBProfileMgr's
    // _UpdateProfileDataFromGameState calls it.
    void ClearResults();  // 0xCBA5B0
    // Empties the results under the lock.
    void ResetResults();  // 0xCBA620
    // Empties the results and marks the submissions started. Handle inlines
    // it.
    void OnScoreSubmissionsStarted();  // 0xCBA680
    // Reads the job's "score_results" JSON for the pad's user: the
    // missions, experience, multipliers, breakdown and mini leaderboards,
    // and the achievements for the player's level and skill. Not
    // reconstructed: the JSON library and the job are not modelled.
    void OnScoreSubmissionsComplete(bool success, DingoJob* job);  // 0xCBA6C0

    int mPadNum;
    // Set by "score_submissions_started".
    bool mSubmissionsStarted;
    RBScoreSubmissionResults mResults;
    OwnedEventSink mCompleteSink;
    OwnedEventSink mStartedSink;
    // Guards mResults.
    CritSec mResultsLock;
};

static_assert(offsetof(RBMiniLeaderboardResults, mPadNum) == 8);
static_assert(offsetof(RBMiniLeaderboardResults, mSubmissionsStarted) == 12);
static_assert(offsetof(RBMiniLeaderboardResults, mResults) == 16);
static_assert(offsetof(RBMiniLeaderboardResults, mCompleteSink) == 568);
static_assert(offsetof(RBMiniLeaderboardResults, mStartedSink) == 608);
static_assert(offsetof(RBMiniLeaderboardResults, mResultsLock) == 648);
static_assert(sizeof(RBMiniLeaderboardResults) == 664);
