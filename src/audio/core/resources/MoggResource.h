#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

// A loaded Mogg file (audio/MoggResource.o). The class has not been
// reconstructed; only the members MoggGeneratorManager reads are declared.
class MoggResource : public Resource {
public:
    // Registers the resource type. Inline; emitted in audio/SoundManager.o
    // at 0x1680.
    static void Init();

    // Field names are not in the reference map. The loaders set both (for
    // example at 0x61A1C) and then call
    // MoggGeneratorManager::RegisterMoggResource.
    // The file's path, which the Mogg generator opens.
    Symbol mFilePath;
    // The file's name without its directory (FileGetName), the sound name
    // that plays the file.
    Symbol mSoundName;
};

static_assert(offsetof(MoggResource, mFilePath) == 48);
static_assert(offsetof(MoggResource, mSoundName) == 56);
