#pragma once

#include <cstddef>

// A gem of a track's gem list (rb_beatmatch/GameGem.o; 24 bytes in the
// lists SongDB::GetGems returns). Only the member the phrase capturer
// calls is declared; the class is not reconstructed.
class GameGem {
public:
    // Whether the band slot's player plays the gem (the gem's player mask at
    // +0x15).
    bool PlayableBy(unsigned long slot) const;  // 0x91C350

    // Not modelled.
    unsigned char mOpaque[24];
};

static_assert(sizeof(GameGem) == 24);
