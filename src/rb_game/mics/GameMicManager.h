#pragma once

#include "mic/core/MicClientMapper.h"

// The game's microphones (rb_game/GameMicManager.o). Init creates the
// manager (0x9BF490) and registers it with the microphone mapper; Terminate
// (0x9BF600) deletes it. Only what the meta state calls is declared; the
// class is not reconstructed.
class GameMicManager {
public:
    // Whether a microphone is connected for the client. The map's
    // HasMic(MicClientID const&) const; this build asks the microphone
    // mapper for the client's microphone.
    bool HasMic(const MicClientID& client) const;  // 0x9BFC30
};

// The manager. At 0x1AE6CA0.
extern GameMicManager* TheGameMicManager;
