#pragma once

#include "render/debug/overlays/RndOverlayTextBase.h"

// The "audio" overlay. Name not in the reference map. The vtable is at
// 0x19392E0. Only the declarations are recovered.
class RndAudioOverlay : public RndOverlayTextBase {
public:
    RndAudioOverlay();  // 0x6E0E60
    ~RndAudioOverlay() override;  // slots 0-1: 0x6E0EA0, 0x6E0EB0

    void _Print(TextStream& stream) override;  // slot 8: 0x6E0E90
};

static_assert(sizeof(RndAudioOverlay) == 64);
