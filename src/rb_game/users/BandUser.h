#pragma once

#include "rb_beatmatch/tracks/TrackType.h"

// A player of the band (rb_game/BandUser.o). Only the members the
// reconstructed meta code calls are declared; the class is not
// reconstructed.
class BandUser {
public:
    // The track the user plays.
    TrackType GetTrackType() const;  // 0xA3F060
    // The user's band slot, looked up by the user's id in theBandUserMgr
    // (BandUserMgr::GetSlot(UserGuid const&) const).
    unsigned long GetSlot() const;  // 0xA3F490
};
