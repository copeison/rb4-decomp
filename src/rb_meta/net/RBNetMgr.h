#pragma once

class DingoJob;

// The game's server manager (rb_meta/RBNetMgr.o): it runs the jobs that
// talk to the game's web services. Only the member the profile manager
// uses is declared; the class is not reconstructed.
class RBNetMgr {
public:
    // Queues the job under the manager's lock, with a 60-second timeout
    // (the job's +512); the manager starts it and deletes it when it ends.
    void ManageJob(DingoJob* job);  // 0xD212D0
};

// The manager, a static object. At 0x1B15288.
extern RBNetMgr theRBNetMgr;
