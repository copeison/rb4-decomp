#pragma once

#include "render/debug/overlays/RndOverlayTextBase.h"

// The "audio" overlay: the sound manager's generator statistics. Name not
// in the reference map; the overlay's object comes first among the overlay
// objects in this build. The vtable is at 0x19392E0.
class RndAudioOverlay : public RndOverlayTextBase {
public:
    RndAudioOverlay();  // 0x6E0E60
    // Slots 0-1: 0x6E0EA0, 0x6E0EB0.
    ~RndAudioOverlay() override;

    void _Print(TextStream& stream) override;  // slot 8: 0x6E0E90
};

static_assert(sizeof(RndAudioOverlay) == 64);
