#pragma once

#include "utl/containers/Vector.h"
#include "utl/messages/MsgSink.h"

class RBPlayer;

// The game in progress (rb_game/Game.o). Only the members the band users
// and the common phrase capturer use are declared; the class is not
// reconstructed. The base follows the game's exports through slot 5
// (MsgSource::Export); the evidence for it is weak.
class Game : public MsgSource {
public:
    const eastl::vector<RBPlayer*>& GetActivePlayers() const;  // 0xA45F40
    // The player of the track; `fail` reports a missing one.
    RBPlayer* GetPlayerFromTrack(int track, bool fail) const;  // 0xA485E0
    // The overshell's pause; the result is not read by its callers.
    bool OvershellSetPaused(bool paused);  // 0xA48300
};

// The game, while one exists. At 0x1AEF770 and 0x1AEF778 (the same object;
// the map has both names).
extern Game* theGame;
extern Game* theMutableGame;
