#pragma once

#include "render/debug/overlays/RndOverlayTextBase.h"

// The "mem" overlay, displayed as "memory": the heap overview, the
// device's memory counters and the FMOD memory usage. The vtable is at
// 0x19397E8.
class RndMemOverlay : public RndOverlayTextBase {
public:
    RndMemOverlay();  // 0x6E54A0
    // Slots 0-1: 0x6E5520, 0x6E5530.
    ~RndMemOverlay() override;

    void _Print(TextStream& stream) override;  // slot 8: 0x6E5550
};

static_assert(sizeof(RndMemOverlay) == 64);
