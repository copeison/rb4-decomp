#pragma once

#include <cstddef>

#include "render/debug/overlays/RndOverlayTextBase.h"

// The "framerate" overlay. The vtable is at 0x1939610. Only the declarations
// are recovered; the members are only sized.
class RndFramerateOverlay : public RndOverlayTextBase {
public:
    RndFramerateOverlay();  // 0x6E27A0
    ~RndFramerateOverlay() override;  // slots 0-1: 0x6E2830, 0x6E2840

    bool HandleKeyboardMsg(const KeyboardKeyMsg& msg) override;  // slot 3: 0x6E2860
    void PrintHelp(TextStream& stream) override;                 // slot 4: 0x6E2940
    void _HandleShowingChanged(bool showing) override;           // slot 6: 0x6E2910
    void _Unknown7() override;                                   // slot 7: 0x6E2960
    void _Print(TextStream& stream) override;                    // slot 8: 0x6E2A60
    Hmx::Color _GetBackgroundColor() override;                   // slot 10: 0x6E2CA0

    // Two flags and the "cpu" budget category. Name not in the reference
    // map.
    unsigned char mUnknown64[16];
};

static_assert(offsetof(RndFramerateOverlay, mUnknown64) == 64);
static_assert(sizeof(RndFramerateOverlay) == 80);
