#pragma once

#include <cstddef>

#include "rb_game/core/Defines.h"
#include "utl/containers/Vector.h"
#include "utl/data/DataNode.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"
#include "utl/time/DateTime.h"

class DataArray;
class DingoJob;
class RBProfile;

namespace Json {
class Value;
}

// A song's leaderboards as the profile caches them, one board per score type
// for the global or the friends' board: the entries the server returned
// around the player, when they arrived, and whether a request is out. A
// request runs as a server job through theRBNetMgr, which reports
// "leaderboard_job_complete" to the board.
//
// The code is the object at 0xB515E0-0xB530FB, between RBFailSceneCom's and
// RBIGSOnboarding.o's (the map's rb_meta order). The map has neither the
// object nor the class; the class name is inferred from its use (RBProfile's
// GetSongLeaderboard and GetFriendsSongLeaderboard) and is weak. The vtable
// at 0x196F568 has MsgSink's four slots (address point 0x196F578; Handle at
// 0x196F588); the object is 96 bytes. RBProfile keeps them in
// eastl::map<int, RBSongLeaderboard>.
class RBSongLeaderboard : public MsgSink {
public:
    // One leaderboard row (56 bytes), read from the server's JSON. Name and
    // field names not in the reference map.
    struct Entry {
        // Emitted at 0xB515E0.
        Entry()
            : mUsername(),
              mPlatformUid(),
              mRank(0),
              mPosition(0),
              mDifficulty(0),
              mScore(0),
              mPercentage(0),
              mIsYou(false) {}

        // Reads "username", "platform_uid", "rank", "difficulty", "score",
        // "percentage" (clamped to 0-100) and "is_you" from the row's JSON.
        // Not reconstructed: the JSON library is not modelled.
        void Read(const Json::Value& row, int position);  // 0xB51610

        Symbol mUsername;
        Symbol mPlatformUid;
        long mRank;
        // The row's index in the server's list.
        long mPosition;
        int mDifficulty;
        long mScore;
        unsigned char mPercentage;
        bool mIsYou;
    };

    // The rows of one score type (40 bytes). Name and field names not in
    // the reference map.
    struct Board {
        // Emitted at 0xB51770; the constructor inlines it.
        Board() : mUpdated(), mRequestPending(false) {}

        eastl::vector<Entry> mEntries;
        // When the rows arrived; all zero before.
        DateTime mUpdated;
        bool mRequestPending;
    };

    // The boards of the score types, kAllScores of them.
    RBSongLeaderboard(RBProfile* profile, int songId, bool friends);  // 0xB517C0
    // Copies the boards and starts without a subscription. Inlined into
    // RBProfile's leaderboard caches (0xD55800, 0xD55DC0), which copy the
    // boards with vector<Board>'s copy constructor (0xD59660). Not in the
    // reference map.
    RBSongLeaderboard(const RBSongLeaderboard& other)
        : MsgSink(other),
          mBoards(other.mBoards),
          mProfile(other.mProfile),
          mSongId(other.mSongId),
          mFriends(other.mFriends),
          mJobCompleteSink() {}
    // Slots 0-1: 0xB51A50, 0xB51B20. Leaves "leaderboard_job_complete".
    ~RBSongLeaderboard() override;

    // Slot 2: "leaderboard_job_complete" (node 2 the success, node 3 the
    // job) goes to OnLeaderboardJobComplete.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xB52AF0

    // Names below are not in the reference map.
    bool IsRequestPending(ScoreType type) const;  // 0xB51BE0
    // Whether the board's rows arrived.
    bool HasResults(ScoreType type) const;  // 0xB51BF0
    // A copy of the board's rows.
    eastl::vector<Entry> GetEntries(ScoreType type) const;  // 0xB51C30
    // Asks the server for the board, unless a request is out: marks it
    // pending, subscribes to theRBNetMgr's "leaderboard_job_complete" and
    // queues the job (0x258 bytes, constructor 0xD1FA60) with the profile's
    // user, the song and the score type. Not reconstructed: the job and its
    // arguments are not modelled.
    void RequestLeaderboard(ScoreType type);  // 0xB52470
    // Reads the job's "leaderboard" JSON into the board's rows, marks the
    // player's own row from the profile's platform name, stamps the time
    // and leaves the event. Not reconstructed: the job and the JSON library
    // are not modelled.
    void OnLeaderboardJobComplete(bool success, DingoJob* job);  // 0xB52610

    // Field names are not in the reference map.
    eastl::vector<Board> mBoards;
    RBProfile* mProfile;
    int mSongId;
    bool mFriends;
    MsgSource::EventSinkElem mJobCompleteSink;
};

static_assert(sizeof(RBSongLeaderboard::Entry) == 56);
static_assert(offsetof(RBSongLeaderboard::Entry, mDifficulty) == 32);
static_assert(offsetof(RBSongLeaderboard::Entry, mPercentage) == 48);
static_assert(sizeof(RBSongLeaderboard::Board) == 40);
static_assert(offsetof(RBSongLeaderboard::Board, mUpdated) == 32);
static_assert(offsetof(RBSongLeaderboard::Board, mRequestPending) == 38);
static_assert(offsetof(RBSongLeaderboard, mBoards) == 8);
static_assert(offsetof(RBSongLeaderboard, mProfile) == 40);
static_assert(offsetof(RBSongLeaderboard, mSongId) == 48);
static_assert(offsetof(RBSongLeaderboard, mFriends) == 52);
static_assert(offsetof(RBSongLeaderboard, mJobCompleteSink) == 56);
static_assert(sizeof(RBSongLeaderboard) == 96);
