#pragma once

#include <cstddef>

// The base of every entity component (entity/Component.o). Its vtable and
// methods are not reconstructed; only the flag that the renderer's default
// lighting sets on its light and light-probe components is declared.
class Component {
public:
    // Field names are not in the reference map.
    unsigned char mUnknown0[22];
    // Cleared to switch the component off. RndDefaults::_LoadLighting
    // (0x6BEF40) and _SyncEnabledLights (0x6BFA60) write it on RndLightCom
    // and RndLightProbeCom components. Name not in the reference map.
    bool mEnabled;
};

static_assert(offsetof(Component, mEnabled) == 22);
