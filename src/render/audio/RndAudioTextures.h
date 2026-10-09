#pragma once

#include <cstddef>

#include "utl/containers/FixedVector.h"
#include "utl/containers/Vector.h"

class RndContext;
class RndTextureBase;

// Two textures of audio analysis samples that shaders read, rebuilt when
// the requested widths change and refreshed every frame. RndDevice
// allocates one in Init and deletes it in Terminate. Name not in the
// reference map.
class RndAudioTextures {
public:
    RndAudioTextures();   // 0x457620
    ~RndAudioTextures();  // 0x4576A0

    // Rebuilds the textures when a requested width changed, then refreshes
    // them.
    void PrepareFrame(RndContext& context);  // 0x457780

    // Not reconstructed yet.
    void Rebuild();
    // Not reconstructed yet.
    void Update(RndContext& context);

    // Field names are not in the reference map.
    FixedVector<RndTextureBase*, 2> mTextures;
    eastl::vector<float> mSamples;
};

static_assert(offsetof(RndAudioTextures, mTextures) == 0);
static_assert(offsetof(RndAudioTextures, mSamples) == 40);
static_assert(sizeof(RndAudioTextures) == 72);
