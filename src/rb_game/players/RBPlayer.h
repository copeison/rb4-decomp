#pragma once

#include <cstddef>

#include "rb_beatmatch/core/Defines.h"
#include "rb_beatmatch/tracks/TrackType.h"
#include "utl/messages/MsgSink.h"

class BandUser;
class RBPlayerController;

// A band user's gameplay player (rb_game/RBPlayer.o, about 35 KB in the map's
// build). Its exports go through SyncMsgSource (vtable 0x1968DE8, slots 4-6;
// the constructor at 0xA58940 points the lock at +0x5B8). Only the members
// the band users and the common phrase capturer use are declared; the class
// is not reconstructed.
class RBPlayer : public SyncMsgSource {
public:
    // Slots 0-1: 0xA58D70, 0xA58EC0.
    ~RBPlayer() override;
    // Slot 2.
    DataNode Handle(DataArray* msg, bool warn) override;  // 0xA5FDD0

    // Switches the player's tracks to the difficulty mid-song. The map's
    // name; the match rests on the band user's only call, so it is weak.
    void ChangeDifficulty(Difficulty difficulty);  // 0xA5F1B0
    // Exports "energy_phrase_hit" and fills the overdrive meter.
    void EnergyPhraseHit();  // 0xA5E5E0
    // Exports "energy_phrase_miss".
    void EnergyPhraseMiss(int phrase);  // 0xA5E870
    // A unison phrase another track failed. The phrase is not read. Name not
    // in the reference map; the evidence is weak.
    void UnisonPhraseMiss(int phrase);  // 0xA5EAA0
    // A unison phrase completed by the player alone or by the band.
    void SoloUnisonPhraseHit();  // 0xA5EB60
    void BandUnisonPhraseHit();  // 0xA5EC00

    // Field names are not in the reference map. The state before the
    // player's controller; not modelled.
    unsigned char mOpaque20[0x4F8 - 0x20];
    // The player's controller (vtable 0x19A8B30 for a local player, 26
    // slots): slot 8 sends a common phrase's last gem hit, slot 9 its
    // failure, slot 22 tells whether the player's user is local. Its class
    // is not modelled (the class name is inferred); see
    // docs/rb-game-phrases.md.
    RBPlayerController* mController;
    unsigned char mOpaque500[8];
    BandUser* mUser;
    TrackType mTrackType;
    // 4 keeps the player out of the common phrases. The name is weak.
    int mPlayerState;
    // The player's track index in the song.
    int mTrack;
};

static_assert(offsetof(RBPlayer, mController) == 0x4F8);
static_assert(offsetof(RBPlayer, mUser) == 0x508);
static_assert(offsetof(RBPlayer, mPlayerState) == 0x514);
static_assert(offsetof(RBPlayer, mTrack) == 0x518);
