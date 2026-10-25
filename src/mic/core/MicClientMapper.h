#pragma once

#include "utl/containers/Vector.h"

// The assignment of connected microphones to players (mic/MicClientMapper.o).
// Only the client id, the manager interface and the members the game calls
// are declared; the class is not reconstructed.

// A client of the microphones: a player, and the microphone it is bound to
// or -1 to look the player's microphone up (MicClientMapper::
// GetMicIDForClientID, 0x8EF330). The map names the type; the field names
// are not in the reference map and their evidence is weak.
struct MicClientID {
    int mPlayerID;
    int mMicID;
};

static_assert(sizeof(MicClientID) == 8);

// What the microphone mapper tells about the microphones: the game's
// microphone manager implements it (GameMicManager). The map names the type
// (MicClientMapper::SetMicManager(MicManagerInterface*)); the method names
// are GameMicManager's in the map. Five slots; declared only.
class MicManagerInterface {
public:
    // Slots 0-1, inline.
    virtual ~MicManagerInterface() {}
    // Slot 2: the connected microphones changed.
    virtual void HandleMicsChanged() = 0;
    // Slot 3: turns the microphones' playback on or off.
    virtual void SetPlayback(bool playback) = 0;
    // Slot 4: the energy of the client's microphone.
    virtual float GetEnergyForMic(const MicClientID& client) = 0;
};

// The assignment of connected microphones to players (mic/MicClientMapper.o;
// constructor 0x8EEEF0, 80 bytes). Only the members the game's microphone
// manager calls are declared.
class MicClientMapper {
public:
    void SetMicManager(MicManagerInterface* manager);  // 0x8EF1E0
    // The client's microphone, or -1.
    int GetMicIDForClientID(const MicClientID& client) const;  // 0x8EF330
    // Appends the ids of the connected microphones.
    void GetAllConnectedMics(eastl::vector<int>& mics) const;  // 0x8EF410
};

// The mapper, created at startup (0x8EC9C0). At 0x1ADF358.
extern MicClientMapper* theMicClientMapper;
