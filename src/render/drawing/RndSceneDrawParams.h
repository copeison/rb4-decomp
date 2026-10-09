#pragma once

#include <cstddef>

// Per-scene drawing options passed through the scene drawer. Only the
// fields the reconstructed code reads are recovered; the rest of the layout
// and the constructor are not reconstructed yet.
class RndSceneDrawParams {
public:
    RndSceneDrawParams();  // Not reconstructed yet.

    // Field names are not in the reference map.
    // Not decoded yet. The scene drawer (0x41AA10) builds the parameters on
    // its stack with a zeroed first word followed by a sub-object (built at
    // 0x434A00) whose first word is an inline-storage pointer, so this
    // holds that word and the sub-object's start. The name is a guess.
    unsigned char mHeader[16];
    // Index of the partial-framerate scene's light-accumulation buffer.
    unsigned long mSceneContext;
};

static_assert(offsetof(RndSceneDrawParams, mSceneContext) == 16);
