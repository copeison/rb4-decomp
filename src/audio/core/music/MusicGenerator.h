#pragma once

#include <cstddef>

#include "audio/core/generators/AudioGenerator.h"

// The base of the music generators, which follow a song's timeline
// (audio/MusicGenerator.o). MidiMusicGenerator, MoggMusicGenerator and
// MusicTimelineGenerator derive from it. The class has not been
// reconstructed; only its construction and the AudioGenerator slots it
// implements are declared. Its own virtuals from slot 32 on (the map's
// SetSyncMaster, MasterAdvanced, section and track queries) are not
// declared. The vtable is at 0x18E18A8; the object is 344 bytes.
class MusicGenerator : public AudioGenerator {
public:
    // At 0x52780. The map emits it in audio/MidiMusicGenerator.o.
    MusicGenerator();

    void Pause() override;                 // slot 0: 0x53800
    void Continue() override;              // slot 1: 0x53890
    void Stop() override;                  // slot 2: 0x55D10
    // Slot 6 at 0x46A70, emitted with the MidiMusic generator.
    float GetLengthMs() const override;
    // Slots 8-9 at 0x46AA0 and 0x46AF0, emitted with the MidiMusic
    // generator.
    void SetSpeed(float speed, bool immediate) override;
    float GetSpeed(bool* changing) override;
    bool IsMusic() override;               // slot 16: 0x46BD0
    ~MusicGenerator() override;            // slots 20-21: 0x52C00, 0x52DA0
    void Kill() override;                  // slot 29: 0x55E00

    // The state after the AudioGenerator base; not reconstructed.
    unsigned char mMusicState[0x158 - 0x50];
};

static_assert(sizeof(MusicGenerator) == 0x158);
