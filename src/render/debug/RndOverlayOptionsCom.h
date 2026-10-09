#pragma once

#include <cstddef>

#include "entity/core/Component.h"

// The component that lists the overlays in the debug options. Its methods
// have not been reconstructed; only the flag that RndOverlayMgr sets is
// declared.
class RndOverlayOptionsCom : public Component {
public:
    // The live component, set at 0x5F9890 and cleared at 0x5F9880; null
    // while none exists. At 0x1AA7F18. Name not in the reference map.
    static RndOverlayOptionsCom* sInstance;

    // Field names are not in the reference map.
    // The flag of the class's "Options" base (constructor 0x46A870, vtable
    // 0x1904718), which is not modelled: 0x469CB0 copies a global switch
    // into every "Options" component. The name rests on that and is weak.
    bool mOptionsSwitch;
    // Set when an overlay registers or unregisters; _Init (0x5F9AD0) clears
    // it as it rebuilds the overlay options.
    bool mOverlaysChanged;
};

static_assert(offsetof(RndOverlayOptionsCom, mOverlaysChanged) == 23);
