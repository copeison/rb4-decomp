#pragma once

#include "rb_meta/state/RBMetaStateCom.h"

// The character menus' session (rb_meta/RBCharacterMenuMgr.o): the player
// customizing a character in the rock shop and the layout the session
// started from. Its members are static. Only what the meta state calls is
// declared; the class is not reconstructed.
class RBCharacterMenuMgr {
public:
    // Whether a character session is under way.
    static bool SessionIsActive();  // 0xE5B160
    // The layout the session started from.
    static RBUILayout GetPreviousLayout();  // 0xE5BAC0
};
