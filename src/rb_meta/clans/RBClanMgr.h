#pragma once

// The manager of the Rivals clans (bands of players): it runs the clan jobs
// ("clan_create_job_complete", "clan_event_leaderboard_job_complete", ...)
// and keeps the weekly challenge state. It is newer than the map; the class
// and global names are inferred. The 4656-byte object is created at
// 0xB0CE20 and deleted at 0xB0CE50. Only what the meta state reads is
// declared; the class is not reconstructed.
class RBClanMgr {
public:
    // The state before the flag below; not reconstructed.
    unsigned char mClanState[0x571];
    // Set while the song library shows only the songs of a challenge
    // ("is_library_filtered_for_challenge"). The evidence for the name is
    // weak.
    bool mLibraryFilteredForChallenge;
};

static_assert(__builtin_offsetof(RBClanMgr, mLibraryFilteredForChallenge) == 0x571);

// The manager. At 0x1AFA328.
extern RBClanMgr* theRBClanMgr;
