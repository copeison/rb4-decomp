#pragma once

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// Tracks the player characters a UI layout shows
// (rb_meta/RBCharacterLayoutTrackerCom.o). Only the members the layout
// component uses are declared; the class is not reconstructed.
class RBCharacterLayoutTrackerCom : public Component {
public:
    // Whether a player character's entity is still loading. The body tests
    // a field at +40 before asking 0xB0C020.
    bool AnyPlayerEntityIsLoading() const;  // 0xE5AAA0

    // "RBCharacterLayoutTrackerCom".
    static Symbol sId;  // 0x1B328F8
};
