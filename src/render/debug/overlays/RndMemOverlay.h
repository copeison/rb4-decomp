#pragma once

#include "render/debug/overlays/RndOverlayTextBase.h"

// The "memory" overlay. The constructor names it with an empty string and
// then sets its display name. The vtable is at 0x19397E8. Only the
// declarations are recovered.
class RndMemOverlay : public RndOverlayTextBase {
public:
    RndMemOverlay();  // 0x6E54A0
    ~RndMemOverlay() override;  // slots 0-1: 0x6E5520, 0x6E5530

    void _Print(TextStream& stream) override;  // slot 8: 0x6E5550
};

static_assert(sizeof(RndMemOverlay) == 64);
