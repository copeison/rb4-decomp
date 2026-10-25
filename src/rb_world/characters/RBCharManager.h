#pragma once

#include "utl/text/Symbol.h"

class BandUser;
class UserGuid;

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
    // Pushes the user's character to the characters of the user's
    // instrument.
    void UpdateCharsForUser(const BandUser* user, const bool& force);  // 0xFC7FC0
    // Marks the prefab used by the user; false when another user has it.
    bool TryToMarkPrefabInUse(const Symbol& name, const UserGuid& user);  // 0xFC9200
    void ClearPrefabInUse(const Symbol& name, const UserGuid& user);  // 0xFC9330
    // Picks a random free prefab for the user and marks it used; false when
    // none is free.
    bool TryToGetRandomPrefabAndMarkInUse(const UserGuid& user, Symbol& name);  // 0xFC9420
    bool UserIsUsingPrefabChar(const UserGuid& user, const Symbol& name);  // 0xFC95D0
};

// The manager, created by RBCharManager::MakeTheCharManager. At 0x1B4B858.
extern RBCharManager* theCharManager;
