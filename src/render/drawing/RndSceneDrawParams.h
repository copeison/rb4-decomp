#pragma once

#include <cstddef>

// Per-scene drawing options passed through the scene drawer. Only the
// fields the reconstructed code reads are recovered; the rest of the layout
// and the constructor are not reconstructed yet.
class RndSceneDrawParams {
public:
    RndSceneDrawParams();  // Not reconstructed yet.

    // Field names are not in the reference map.
    unsigned char mUnknown0[16];
    // Index of the partial-framerate scene's light-accumulation buffer.
    unsigned long mSceneContext;
};

static_assert(offsetof(RndSceneDrawParams, mSceneContext) == 16);
