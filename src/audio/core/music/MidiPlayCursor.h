#pragma once

#include <cstddef>

// A cursor that reads MIDI events ahead of a song position and reports them
// to its virtual callbacks (audio/MidiPlayCursor.o). MidiMusicGenerator
// derives from it. The class has not been reconstructed; only its
// construction is declared. The vtable is at 0x18E5460 and has 12 slots;
// slots 2-11 (the map's OnMidiMessage, OnPreRollNoteOn, OnLoop and OnTempo
// callbacks among them) are not declared. The object is 600 bytes.
class MidiPlayCursor {
public:
    MidiPlayCursor();            // 0xAA200
    virtual ~MidiPlayCursor();   // slots 0-1: 0xAA270, 0xAA330

    // The state after the vtable; not reconstructed.
    unsigned char mCursorState[0x258 - 0x8];
};

static_assert(sizeof(MidiPlayCursor) == 0x258);
