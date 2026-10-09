#pragma once

#include "entity/resources/Resource.h"

class FusionPatchCom;

// A loaded Fusion patch file (audio/FusionPatchResource.o). The class has
// not been reconstructed; only the member FusionSampler uses is declared.
class FusionPatchResource : public Resource {
public:
    // The patch component of the resource's entity. At 0x5B830.
    FusionPatchCom* GetPatch() const;
};
