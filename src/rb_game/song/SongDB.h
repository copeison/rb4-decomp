#pragma once

#include "utl/containers/Vector.h"

class GameGem;
class RBPlayer;

// The song's gem and phrase database (rb_game/SongDB.o). Only the members
// the common phrase capturer calls are declared, with the map's names; the
// class is not reconstructed.
class SongDB {
public:
    // The track's gems.
    const eastl::vector<GameGem>& GetGems(int track) const;  // 0xA6D7E0
    // The number of tracks. The binary returns 64 bits.
    unsigned long GetNumTracks() const;  // 0xA6D720
    // The common phrase the track's gem belongs to, or -1.
    int GetPhraseID(int track, unsigned long gem) const;  // 0xA6DCA0
    bool IsUnisonPhrase(int phrase) const;  // 0xA6DCE0
    // The tracks of the active players that share the common phrase, as a
    // bit mask.
    int GetCommonPhraseTracks(unsigned long phrase) const;  // 0xA6DDE0
    // The common phrase the player's track is in at the tick, or -1.
    int GetCommonPhraseID(const RBPlayer& player, int tick) const;  // 0xA6DF70
};

// The song database. At 0x1AEFCE0.
extern SongDB* theSongDB;
