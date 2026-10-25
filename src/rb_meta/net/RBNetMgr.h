#pragma once

#include "utl/messages/MsgSink.h"

class DingoJob;

// The game's server manager (rb_meta/RBNetMgr.o): it runs the jobs that
// talk to the game's web services, and its SyncMsgSource base (offset 0)
// reports their events, such as "score_submissions_started",
// "score_submissions_complete" and "leaderboard_job_complete"
// (RBMiniLeaderboardResults and RBSongLeaderboard subscribe through
// SyncMsgSource::AddSink at the manager's address). Only the members its
// users need are declared; the class is not reconstructed.
class RBNetMgr : public SyncMsgSource {
public:
    // Queues the job under the manager's lock, with a 60-second timeout
    // (the job's +512); the manager starts it and deletes it when it ends.
    void ManageJob(DingoJob* job);  // 0xD212D0
};

// The manager, a static object. At 0x1B15288.
extern RBNetMgr theRBNetMgr;
