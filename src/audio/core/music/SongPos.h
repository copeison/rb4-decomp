#pragma once

#include <cstddef>

// A position in a song's measures, beats and ticks (audio/SongPos.o). The
// map's SongPos::Set(float, float, int, int, int, int, int) fills the seven
// fields in order. The class is not reconstructed; only its layout and Set
// are declared, for the emitter component. Field names are not in
// the reference map: the emitter's "bar", "beat" and "tick" properties read
// the last three (each plus one), and the names of the first four follow
// Set's argument types only, so they are weak.
struct SongPos {
    // Stores the fields in order. At 0xBC740.
    void Set(
        float totalTick,
        float totalBeat,
        int beatsPerMeasure,
        int ticksPerBeat,
        int measure,
        int beat,
        int tick);

    float mTotalTick;
    float mTotalBeat;
    int mBeatsPerMeasure;
    int mTicksPerBeat;
    int mMeasure;
    int mBeat;
    int mTick;
};

static_assert(offsetof(SongPos, mMeasure) == 16);
static_assert(sizeof(SongPos) == 28);
