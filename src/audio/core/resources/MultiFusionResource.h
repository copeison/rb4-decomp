#pragma once

#include <cstddef>

#include "audio/core/instruments/InstrumentGenerator.h"
#include "audio/core/resources/FusionPatchResource.h"
#include "entity/resources/Resource.h"
#include "utl/text/Symbol.h"

// A set of Fusion patches played on the 16 channels of a MultiFusion
// generator (audio/MultiFusionResource.o). The class is newer than the
// reference map and is not reconstructed; only its registration and the
// members MultiFusionGenerator::SetResource reads are declared. Its loaders
// (0x65D0E, 0x6610D) register it with MultiFusionGeneratorManager. The
// vtable is at 0x18E2B20; the object is 320 bytes.
class MultiFusionResource : public Resource {
public:
    static constexpr int kNumChannels = 16;  // Name not in the reference map.

    // One channel's patch. Names not in the reference map.
    struct Channel {
        ResourcePtr<FusionPatchResource> mPatch;
        // How the channel follows channel 0 when mSlaveChannels is set; the
        // constructor (inlined into the factory at 0x9620) sets 2.
        InstrumentSlaveType mSlaveType;
    };

    // Registers the class. Emitted in audio/SoundManager.o at 0x1AD0.
    static void Init();

    // Field names are not in the reference map.
    // The sound name under which the resource is registered and played.
    Symbol mSoundId;
    Channel mChannels[kNumChannels];
    // Makes the other channels' instruments slaves of channel 0's.
    bool mSlaveChannels;
};

static_assert(sizeof(MultiFusionResource::Channel) == 16);
static_assert(offsetof(MultiFusionResource, mSoundId) == 0x30);
static_assert(offsetof(MultiFusionResource, mChannels) == 0x38);
static_assert(offsetof(MultiFusionResource, mSlaveChannels) == 0x138);
static_assert(sizeof(MultiFusionResource) == 0x140);
