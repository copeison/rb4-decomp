#pragma once

#include "render/debug/overlays/RndOverlayTextBase.h"

// The "cheats" overlay, shown from construction. The vtable is at
// 0x1939348. Only the declarations are recovered.
class RndCheatsOverlay : public RndOverlayTextBase {
public:
    RndCheatsOverlay();  // 0x6E0EF0
    ~RndCheatsOverlay() override;  // slots 0-1: 0x6E0F30, 0x6E0F40

    void Draw(RndContext& context, int y) override;  // slot 2: 0x6E0F60
    void _Print(TextStream& stream) override;        // slot 8: 0x6E0F90
};

static_assert(sizeof(RndCheatsOverlay) == 64);
