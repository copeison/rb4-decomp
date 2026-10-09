#pragma once

#include <cstddef>

#include "entity/resources/Resource.h"

class RndTexture1D;

// A 1D texture resource (render/RndTexture1DResource.o). Only the texture
// is modelled; the virtuals and the rest of the layout are not recovered.
// RndTextureUtl::CreateWaveformFloatTextureResource (0x6AF510) builds one
// (56 bytes, constructor 0x68F850) for a baked waveform.
class RndTexture1DResource : public Resource {
public:
    // Name not in the reference map.
    RndTexture1D* mTexture;
};

static_assert(offsetof(RndTexture1DResource, mTexture) == 48);
