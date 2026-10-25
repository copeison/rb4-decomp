#pragma once

#include "utl/text/Symbol.h"

namespace RBChar {
struct RBCharDesc;
}

// The character manager (rb_world/RBCharManager.o): the prefab characters,
// the per-instrument outfits and which users use which prefab. Only what
// the profile's characters read is declared; the class is not
// reconstructed.
class RBCharManager {
public:
    // The prefab's description, or null when no prefab has the name or the
    // prefabs are not enumerated yet.
    const RBChar::RBCharDesc* GetPrefabCharDesc(Symbol name) const;  // 0xFC3DC0
};

// The manager, created by RBCharManager::MakeTheCharManager. At 0x1B4B858.
extern RBCharManager* theCharManager;
