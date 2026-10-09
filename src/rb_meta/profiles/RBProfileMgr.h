#pragma once

// The game's profile manager (rb_meta/RBProfileMgr.o). Only the per-frame
// update is declared so far.
class RBProfileMgr {
public:
    // Updates profile state and reacts to save/load failures and layout
    // transitions.
    void Poll();  // 0xD5C230
};

extern RBProfileMgr* theRBProfileMgr;
