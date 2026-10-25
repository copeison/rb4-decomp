// RBMiniLeaderboardResults's code (0xCB9FE0 to 0xCBD7AF, after
// RBUpsellScreen's object; the map has no name for it). Besides the members
// below it holds the score results' JSON readers that
// OnScoreSubmissionsComplete calls (0xCBB900 for a mini leaderboard, read
// three times; 0xCBBE10 and 0xCBBFD0 for the pad's user; 0xCBC430 and
// 0xCBCB80), two uncalled helpers that raise an int at +32 and at +16 of
// their object to the argument (0xCBA660, 0xCBA670), the static initializer
// at 0xCBD470 (the shared header globals every rb_meta object initializes;
// not modelled) and template helpers from 0xCBD4B0.
//
// Not reconstructed: OnScoreSubmissionsComplete (0xCBA6C0) and the readers,
// which need the JSON library and the job.
#include "rb_meta/leaderboards/RBMiniLeaderboardResults.h"

#include "rb_meta/net/RBNetMgr.h"
#include "utl/data/DataArray.h"
#include "utl/text/Str.h"

// Reconstructed from eboot.elf at 0xCB9FE0.
eastl::vector<AllowedValue<int>> GetAllowedMiniLeaderboardTypeValues() {
    eastl::vector<AllowedValue<int>> values;
    values.push_back(
        AllowedValue<int>{kMiniLeaderboardTypeInvalid, String("kMiniLeaderboardTypeInvalid"), String("")});
    values.push_back(
        AllowedValue<int>{kMiniLeaderboardRankPoints, String("kMiniLeaderboardRankPoints"), String("")});
    values.push_back(AllowedValue<int>{kMiniLeaderboardAllTime, String("kMiniLeaderboardAllTime"), String("")});
    return values;
}

// Reconstructed from eboot.elf at 0xCBA1E0.
RBMiniLeaderboardResults::RBMiniLeaderboardResults(int padNum)
    : mPadNum(padNum),
      mSubmissionsStarted(false),
      mResults(),
      mCompleteSink(),
      mStartedSink(),
      mResultsLock() {
    static Symbol complete;
    if (complete == Symbol()) {
        complete = Symbol("score_submissions_complete");
    }
    theRBNetMgr.AddSink(&mCompleteSink, this, complete, Symbol());
    static Symbol started;
    if (started == Symbol()) {
        started = Symbol("score_submissions_started");
    }
    theRBNetMgr.AddSink(&mStartedSink, this, started, Symbol());
}

// Reconstructed from eboot.elf at 0xCBA3C0. The deleting destructor is at
// 0xCBA590.
RBMiniLeaderboardResults::~RBMiniLeaderboardResults() {}

// Reconstructed from eboot.elf at 0xCBA5B0.
void RBMiniLeaderboardResults::ClearResults() {
    mResults.mNumMissionsCompleted = 0;
    mResults.mMissionsCompleted.clear();
}

// Reconstructed from eboot.elf at 0xCBA620.
void RBMiniLeaderboardResults::ResetResults() {
    mResultsLock.Enter();
    mResults.Reset();
    mResultsLock.Exit();
}

// Reconstructed from eboot.elf at 0xCBA680.
void RBMiniLeaderboardResults::OnScoreSubmissionsStarted() {
    mResultsLock.Enter();
    mResults.Reset();
    mResultsLock.Exit();
    mSubmissionsStarted = true;
}

// Reconstructed from eboot.elf at 0xCBD2B0.
DataNode RBMiniLeaderboardResults::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol complete;
    if (complete == Symbol()) {
        complete = Symbol("score_submissions_complete");
    }
    if (type == complete) {
        OnScoreSubmissionsComplete(msg->Int(2) != 0, static_cast<DingoJob*>(msg->Node(3).LiteralSink(msg)));
        return DataNode(0);
    }
    static Symbol started;
    if (started == Symbol()) {
        started = Symbol("score_submissions_started");
    }
    if (type == started) {
        OnScoreSubmissionsStarted();
        return DataNode(0);
    }
    return DataNode();
}
