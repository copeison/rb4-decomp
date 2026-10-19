#pragma once

#include "utl/containers/Vector.h"

class BandUser;

// The band's users and their slots (rb_game/BandUserMgr.o). Only the
// members the reconstructed meta code calls are declared; the class is not
// reconstructed.
class BandUserMgr {
public:
    // Every band user. The map has the no-argument GetBandUsers(); this
    // build returns the list, which it fills through the flagged overload
    // (0xA41110) with the flags 8.
    eastl::vector<BandUser*> GetBandUsers() const;  // 0xA40DC0
};

extern BandUserMgr* theBandUserMgr;  // 0x1AEF5D8
