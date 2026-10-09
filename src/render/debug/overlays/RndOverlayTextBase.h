#pragma once

#include "math/color/Color.h"
#include "render/debug/RndOverlay.h"

class TextStream;

// An overlay that prints text. Draw renders what _Print writes, below the
// given line. The vtable is at 0x19398A0. Only the declarations are
// recovered.
class RndOverlayTextBase : public RndOverlay {
public:
    // The map's signature is RndOverlayTextBase(char const*, char const*,
    // unsigned int); this build has no second string.
    RndOverlayTextBase(const char* name, unsigned int flags);  // 0x6E5B00
    ~RndOverlayTextBase() override;  // slots 0-1: 0x6E5B30, 0x6E5B40

    void Draw(RndContext& context, int y) override;  // slot 2: 0x6E5B60

    // Slot 8: writes the overlay's text.
    virtual void _Print(TextStream& stream) = 0;
    // Slot 9 at 0x6D1720: white.
    virtual Hmx::Color _GetTextColor();
    // Slot 10 at 0x6E5D90: a shared default color.
    virtual Hmx::Color _GetBackgroundColor();
};

static_assert(sizeof(RndOverlayTextBase) == 64);
