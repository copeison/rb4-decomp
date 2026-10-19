#pragma once

// The conversions between a song's milliseconds, ticks and beats
// (audio/BeatSource.o; the map's SongMapBeatSource and the song sections
// implement it). Only the slots the animation code calls are declared; the
// class is not reconstructed. The slot order follows the property
// animations' calls (PropAnimResource::GetTimeFromFrame, 0x2FDD70), which
// pair slots 3 and 4 to turn milliseconds into beats and slots 5 and 2 to
// turn beats into milliseconds; the destructor in slots 0-1 is assumed.
class BeatSource {
public:
    // Slots 0-1.
    virtual ~BeatSource();
    // Slots 2-5.
    virtual float TickToMs(float tick) const = 0;
    virtual float MsToTick(float ms) const = 0;
    virtual float TickToBeat(float tick) const = 0;
    virtual float BeatToTick(float beat) const = 0;
};
