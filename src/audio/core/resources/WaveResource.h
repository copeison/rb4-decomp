#pragma once

#include "audio/core/resources/AudioSampleResource.h"
#include "utl/text/Symbol.h"

// A sample loaded from a ".wav" file (audio/WaveResource.o). The class has
// not been reconstructed; only the class id FusionPatchCom loads keyzone
// samples with is declared.
class WaveResource : public AudioSampleResource {
public:
    // The class id, created on first use and inlined into its users, such
    // as Resource::GetOrLoad<WaveResource> at 0x84720. The local static is
    // at 0x19C5E90.
    static Symbol Id() {
        static Symbol id;
        if (id == Symbol()) {
            id = Symbol("WaveResource");
        }
        return id;
    }
};
