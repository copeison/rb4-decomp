// RBSongLeaderboard's object (0xB515E0 to 0xB530FB; the map has no name for
// it). Besides the members below it holds Entry's constructor (0xB515E0)
// and Board's (0xB51770), a merge of a board's rows with the player's own
// (0xB51CF0, its signature not recovered; it sorts with the helper at
// 0xB52C10), and the static initializer at 0xB530C0, which sets the shared
// header globals every rb_meta object initializes and is not modelled.
//
// Not reconstructed: Entry::Read (0xB51610), RequestLeaderboard (0xB52470)
// and OnLeaderboardJobComplete (0xB52610), which need the JSON library and
// the leaderboard job.
#include "rb_meta/leaderboards/RBSongLeaderboard.h"

#include "utl/data/DataArray.h"

// Reconstructed from eboot.elf at 0xB517C0.
RBSongLeaderboard::RBSongLeaderboard(RBProfile* profile, int songId, bool friends)
    : mBoards(kAllScores),
      mProfile(profile),
      mSongId(songId),
      mFriends(friends),
      mJobCompleteSink() {}

// Reconstructed from eboot.elf at 0xB51A50. The deleting destructor is at
// 0xB51B20.
RBSongLeaderboard::~RBSongLeaderboard() {
    LinkedList::Node& link = mJobCompleteSink.mLink;
    if (link.mNext != &link && link.mPrev != &link) {
        mJobCompleteSink.mSource->RemoveSink(&mJobCompleteSink);
    }
}

// Reconstructed from eboot.elf at 0xB51BE0.
bool RBSongLeaderboard::IsRequestPending(ScoreType type) const {
    return mBoards.begin()[type].mRequestPending;
}

// Reconstructed from eboot.elf at 0xB51BF0.
bool RBSongLeaderboard::HasResults(ScoreType type) const {
    const DateTime& updated = mBoards.begin()[type].mUpdated;
    return updated.mYear != 0 || updated.mSec != 0 || updated.mMin != 0 || updated.mHour != 0 ||
        updated.mDay != 0 || updated.mMonth != 0;
}

// Reconstructed from eboot.elf at 0xB51C30.
eastl::vector<RBSongLeaderboard::Entry> RBSongLeaderboard::GetEntries(ScoreType type) const {
    return mBoards.begin()[type].mEntries;
}

// Reconstructed from eboot.elf at 0xB52AF0.
DataNode RBSongLeaderboard::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    const Symbol type = msg->Sym(1);
    static Symbol jobComplete;
    if (jobComplete == Symbol()) {
        jobComplete = Symbol("leaderboard_job_complete");
    }
    if (type != jobComplete) {
        return DataNode();
    }
    OnLeaderboardJobComplete(msg->Int(2) != 0, static_cast<DingoJob*>(msg->Node(3).LiteralSink(msg)));
    return DataNode(0);
}
