#pragma once

#include "utl/containers/Set.h"

class DingoJob;

// Asks the server for a pad's entitlements ("client_entitlements/get/");
// the reply reaches RBProfileMgr::OnEntitlementsJobDone. A DingoJob of 536
// bytes (vtable 0x198EA98). Newer than the map, which has no such job;
// declared only. Name not in the reference map.
class RBNetEntitlementsJob {
public:
    explicit RBNetEntitlementsJob(int padNum);  // 0xEF61D0

    // The job as RBNetMgr::ManageJob takes it. Name not in the reference
    // map.
    DingoJob* AsDingoJob() {
        return reinterpret_cast<DingoJob*>(this);
    }

    // DingoJob's state, not modelled.
    unsigned char mJobData[536];
};

static_assert(sizeof(RBNetEntitlementsJob) == 536);

// Sends a pad's known songs to the leaderboards
// ("leaderboards/player/update_songs/", "known_song_ids"); the reply
// reaches RBProfileMgr::OnSongUploadJobDone. A DingoJob of 536 bytes
// (vtable 0x198ECB8). Newer than the map; declared only. Name not in the
// reference map.
class RBNetUpdateSongsJob {
public:
    RBNetUpdateSongsJob(int padNum, const eastl::set<int>& songIds);  // 0xEF7DC0

    // The job as RBNetMgr::ManageJob takes it. Name not in the reference
    // map.
    DingoJob* AsDingoJob() {
        return reinterpret_cast<DingoJob*>(this);
    }

    // DingoJob's state, not modelled.
    unsigned char mJobData[536];
};

static_assert(sizeof(RBNetUpdateSongsJob) == 536);
