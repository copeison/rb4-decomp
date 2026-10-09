#pragma once

// The component that lists the overlays in the debug options. Its Component
// base and its methods have not been reconstructed; only the flag that
// RndOverlayMgr sets is declared.
class RndOverlayOptionsCom {
public:
    // The live component, set at 0x5F9890 and cleared at 0x5F9880; null
    // while none exists. At 0x1AA7F18. Name not in the reference map.
    static RndOverlayOptionsCom* sInstance;

    // Field names are not in the reference map.
    unsigned char mUnknown0[23];
    // Set when an overlay registers or unregisters; _Init (0x5F9AD0)
    // clears it as it rebuilds the overlay options.
    bool mOverlaysChanged;
};
