#pragma once

#include "persistence/profiles/ProfileMgr.h"

// The game's profile manager (rb_meta/RBProfileMgr.o), built on ProfileMgr;
// its vtable is at 0x197DEB0. Only the per-frame update is declared so far.
class RBProfileMgr : public ProfileMgr {
public:
    // Updates profile state and reacts to save/load failures and layout
    // transitions.
    void Poll();  // 0xD5C230
};

extern RBProfileMgr* theRBProfileMgr;
