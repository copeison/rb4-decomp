#pragma once

#include <cstddef>

#include "persistence/profiles/ProfileMgr.h"
#include "rb_meta/profiles/RBSystemOptions.h"

// The game's profile manager (rb_meta/RBProfileMgr.o), built on ProfileMgr;
// its vtable is at 0x197DEB0. Its GetSystemOptions (0xD62560) returns
// mSystemOptions and its GetSystemSaveFileName (0xD62570)
// "rb_system_settings.bin". Only the per-frame update and the system
// options are declared so far.
class RBProfileMgr : public ProfileMgr {
public:
    // Updates profile state and reacts to save/load failures and layout
    // transitions.
    void Poll();  // 0xD5C230

    // The console's options. Name not in the reference map.
    RBSystemOptions mSystemOptions;
};

static_assert(offsetof(RBProfileMgr, mSystemOptions) == 344);

extern RBProfileMgr* theRBProfileMgr;  // 0x1B1F7F0
