#pragma once

#include "entity/resources/Resource.h"

// A MIDI song bound to the instrument patches that play it
// (audio/MidiMusicResource.o). The class has not been reconstructed; only
// the members the sound manager and MidiMusicGeneratorManager use are
// declared. Its constructor (0x5D320, the map's
// MidiMusicResource(MidiFileResource*, Symbol)) stores the symbol at +0x38
// and registers the resource with MidiMusicGeneratorManager; the destructor
// (0x5D480) unregisters it. The vtable is at 0x18E2830 and has 20 slots.
class MidiMusicResource : public Resource {
public:
    // Registers the class. Emitted in audio/SoundManager.o at 0x1F20.
    static void Init();

    // Slot 13 at 0x5EE90: the sound name under which the resource is
    // registered and played, stored at +0x38. Name not in the reference
    // map; it follows MusicTimelineGeneratorManager::kMusicTimelineSoundId
    // and is weak. Slots 14-19 are not declared.
    virtual Symbol GetSoundId() const;
};
