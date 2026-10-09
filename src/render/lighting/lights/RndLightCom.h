#pragma once

#include <cstddef>

#include "entity/core/Component.h"
#include "utl/text/Symbol.h"

// The base of the light components. Its methods are not reconstructed; the
// renderer's defaults find lights through it and switch them with the
// component's enabled flag.
class RndLightCom : public Component {
public:
    // The class's second symbol, "Light", stored by the static initializer
    // at 0x470530 beside sId (0x1A873B0). Base-class lookups compare it
    // with GameObject::ComIndex::mBaseId. Name not in the reference map.
    static Symbol sClassName;  // 0x1A873B8

    // Placed in the Component base's tail padding. Cleared to switch the
    // light off; RndDefaults::_LoadLighting (0x6BEF40) and
    // _SyncEnabledLights (0x6BFA60) write it. Name not in the reference map.
    bool mEnabled;
};

static_assert(offsetof(RndLightCom, mEnabled) == 22);
