#include "render/debug/RndOverlay.h"

#include "render/debug/RndOverlayMgr.h"
#include "render/system/RndDevice.h"

// Reconstructed from eboot.elf at 0x6E5860. The overlay starts hidden.
RndOverlay::RndOverlay(const char* name, unsigned int flags)
    : mName(name), mFlags(flags), mShowing(false), mUpdateFrame(-1UL) {
    RndOverlayMgr::RegisterOverlay(*this);
}

// Reconstructed from eboot.elf at 0x6E58C0 (deleting variant at 0x6E5910).
RndOverlay::~RndOverlay() {
    RndOverlayMgr::UnregisterOverlay(*this);
}

// Reconstructed from eboot.elf at 0x6E5A90.
void RndOverlay::SetShowing(bool showing) {
    if (showing != mShowing) {
        mShowing = !mShowing;
        _HandleShowingChanged(showing);
    }
}

// Reconstructed from eboot.elf at 0x6E5AB0.
void RndOverlay::Update() {
    const unsigned long frame = TheRndDevice()->mFrameCount;
    if (frame != mUpdateFrame) {
        mUpdateFrame = frame;
        _Update();
    }
}
