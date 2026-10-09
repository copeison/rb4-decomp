#pragma once

#include <cstddef>

#include "render/debug/overlays/RndOverlayTextBase.h"

// The "console" overlay: a command line with history and tab completion.
// The vtable is at 0x19393B0. Only the declarations are recovered; the
// members are only sized.
class RndConsoleOverlay : public RndOverlayTextBase {
public:
    RndConsoleOverlay();  // 0x6E0FE0
    ~RndConsoleOverlay() override;  // slots 0-1: 0x6E1070, 0x6E1250

    void Draw(RndContext& context, int y) override;              // slot 2: 0x6E1270
    bool HandleKeyboardMsg(const KeyboardKeyMsg& msg) override;  // slot 3: 0x6E1700
    void PrintHelp(TextStream& stream) override;                 // slot 4: 0x6E1710
    void _Print(TextStream& stream) override;                    // slot 8: 0x6E1730
    Hmx::Color _GetTextColor() override;                         // slot 9: 0x6E1810
    Hmx::Color _GetBackgroundColor() override;                   // slot 10: 0x6E18A0

    unsigned char mUnknown64[1416];  // Name not in the reference map.
};

static_assert(offsetof(RndConsoleOverlay, mUnknown64) == 64);
static_assert(sizeof(RndConsoleOverlay) == 1480);
