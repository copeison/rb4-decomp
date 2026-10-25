#pragma once

#include "rb_game/players/RBPlayer.h"

// A player of a gem track (rb_game/RBGemPlayer.o; constructor 0xA4D7F0,
// vtable 0x19689C0). Only the member the common phrase capturer calls is
// declared; the class is not reconstructed.
class RBGemPlayer : public RBPlayer {
public:
    // Whether the player hit the gem (its result flags, +0x5E0, have 0x9).
    bool HasHitGem(unsigned long gem) const;  // 0xA56290
};
