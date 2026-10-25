#pragma once

#include "rb_beatmatch/core/Defines.h"

// A band user's gameplay player (rb_game/RBPlayer.o, about 35 KB in the map's
// build). Only the member the band users call is declared; the class is not
// reconstructed.
class RBPlayer {
public:
    // Switches the player's tracks to the difficulty mid-song. The map's
    // name; the match rests on the band user's only call, so it is weak.
    void ChangeDifficulty(Difficulty difficulty);  // 0xA5F1B0
};
