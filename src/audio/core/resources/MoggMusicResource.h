#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"

// A multitrack Ogg song with its MIDI maps (audio/MoggMusicResource.o). The
// class has not been reconstructed; only the members the sound manager and
// MoggMusicGeneratorManager use are declared. LoadFile (0x5FA80) and Load
// (0x60770) register the resource with MoggMusicGeneratorManager and the
// destructor (0x5F710) unregisters it. The vtable is at 0x18E2918 and has 20 slots; the object
// is 144 bytes.
class MoggMusicResource : public Resource {
public:
    // Registers the class. Emitted in audio/SoundManager.o at 0x1DB0.
    static void Init();

    // Slot 13 at 0x60FA0: mSoundId. Name not in the reference map; it
    // follows MusicTimelineGeneratorManager::kMusicTimelineSoundId and is
    // weak. Slots 14-19 are not declared.
    virtual Symbol GetSoundId() const;

    // Field names are not in the reference map.
    // The song maps of the MIDI resource at +0x50, taken from its slot 19
    // by LoadFile; MoggMusicGenerator::GetSongMaps (0x4E300) returns them.
    // The name follows that method and is weak.
    void* mSongMaps;
    // The sound name under which the resource is registered and played.
    Symbol mSoundId;
    // The file name of the mogg (FileGetName of its path), which
    // MoggMusicGeneratorManager::Play passes to the Mogg generator manager.
    Symbol mMoggName;
    // The rest of the object; not reconstructed.
    unsigned char mSongState[0x90 - 0x48];
};

static_assert(offsetof(MoggMusicResource, mSongMaps) == 0x30);
static_assert(offsetof(MoggMusicResource, mSoundId) == 0x38);
static_assert(offsetof(MoggMusicResource, mMoggName) == 0x40);
static_assert(sizeof(MoggMusicResource) == 0x90);
