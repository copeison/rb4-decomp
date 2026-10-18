#pragma once

#include "audio/core/resources/AudioSampleResource.h"
#include "entity/resources/Resource.h"

// A keyzone sample played from a ".mogg" file. The class is newer than the
// reference map; its name is the class id it registers. It has not been
// reconstructed; only the lookup FusionPatchCom uses is declared.
class MoggSampleResource : public AudioSampleResource {
public:
    // The loaded sample at the path, or a new one (56 bytes, vtable
    // 0x18E2AB0) bound to the path and loaded; null for an empty path.
    // Name not in the reference map. At 0x64F30.
    static ResourcePtr<MoggSampleResource> GetOrCreate(ResourcePath path);
};
