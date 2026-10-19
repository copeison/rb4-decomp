#pragma once

// The assignment of connected microphones to players (mic/MicClientMapper.o).
// Only the client id is declared; the class is not reconstructed.

// A client of the microphones: a player, and the microphone it is bound to
// or -1 to look the player's microphone up (MicClientMapper::
// GetMicIDForClientID, 0x8EF330). The map names the type; the field names
// are not in the reference map and their evidence is weak.
struct MicClientID {
    int mPlayerID;
    int mMicID;
};

static_assert(sizeof(MicClientID) == 8);
