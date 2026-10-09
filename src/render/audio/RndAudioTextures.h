#pragma once

#include <cstddef>

#include "utl/containers/FixedVector.h"
#include "utl/containers/Vector.h"

class RndContext;
class RndTextureBase;

// Two textures of audio analysis results that shaders read: the FFT bins
// and the semitone filter bank, one row per AudioAnalysis slot. They are
// rebuilt when the requested widths change and refreshed every frame.
// RndDevice allocates one in Init and deletes it in Terminate. Name not in
// the reference map.
class RndAudioTextures {
public:
    RndAudioTextures();   // 0x457620
    ~RndAudioTextures();  // 0x4576A0

    // Rebuilds the textures when a requested width changed, then refreshes
    // them.
    void PrepareFrame(RndContext& context);  // 0x457780

    // Recreates the textures at the requested widths, filled with a
    // placeholder pattern. Name not in the reference map.
    void Rebuild();  // 0x4578B0
    // Copies the active slots' results into the textures and syncs them.
    // Name not in the reference map.
    void Update(RndContext& context);  // 0x458110
    // The widest FFT and semitone range any active slot requests. The
    // object is not read. Name not in the reference map.
    void GetRequestedWidths(FixedVector<int, 2>& widths) const;  // 0x4584B0

    // Field names are not in the reference map.
    FixedVector<RndTextureBase*, 2> mTextures;
    eastl::vector<float> mSamples;
};

static_assert(offsetof(RndAudioTextures, mTextures) == 0);
static_assert(offsetof(RndAudioTextures, mSamples) == 40);
static_assert(sizeof(RndAudioTextures) == 72);
