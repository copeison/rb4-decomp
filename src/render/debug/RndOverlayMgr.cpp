#include "render/debug/RndOverlayMgr.h"

#include "render/debug/RndOverlayOptionsCom.h"
#include "render/debug/overlays/RndAudioOverlay.h"
#include "render/debug/overlays/RndCheatsOverlay.h"
#include "render/debug/overlays/RndConsoleOverlay.h"
#include "render/debug/overlays/RndFramerateOverlay.h"
#include "render/debug/overlays/RndMemOverlay.h"
#include "render/debug/overlays/RndOverlayGraphBase.h"
#include "render/debug/overlays/RndTimersOverlay.h"

namespace {

// Tells the options component to rebuild its overlay list. Inlined into
// RegisterOverlay and UnregisterOverlay; name not in the reference map.
void MarkOverlaysChanged() {
    if (RndOverlayOptionsCom::sInstance != nullptr) {
        RndOverlayOptionsCom::sInstance->mOverlaysChanged = true;
    }
}

}  // namespace

// Static initializer at 0x5F9810; the destructor is at 0x5F9120.
RndOverlayMgr::OverlayList RndOverlayMgr::gOverlays;

// Reconstructed from eboot.elf at 0x5F9150. The overlays register
// themselves on construction and live for the rest of the program; the
// timer graphs find their timers overlays by name, so those come first.
void RndOverlayMgr::Init() {
    new RndConsoleOverlay;
    new RndFramerateOverlay;
    new RndFramerateGraphOverlay;
    new RndCpuTimersOverlay;
    new RndCpuTimerGraphOverlay;
    new RndGpuTimersOverlay;
    new RndGpuTimerGraphOverlay;
    new RndMemOverlay;
    new RndCheatsOverlay;
    new RndAudioOverlay;
}

// Reconstructed from eboot.elf at 0x5F9210.
int RndOverlayMgr::GetMarginInPixels() {
    return 30;
}

// Reconstructed from eboot.elf at 0x5F9220.
int RndOverlayMgr::GetVerticalSpacingInPixels() {
    return 5;
}

// Reconstructed from eboot.elf at 0x5F9520.
void RndOverlayMgr::RegisterOverlay(RndOverlay& overlay) {
    gOverlays.push_back(overlay);
    MarkOverlaysChanged();
}

// Reconstructed from eboot.elf at 0x5F9560.
void RndOverlayMgr::UnregisterOverlay(RndOverlay& overlay) {
    gOverlays.remove(overlay);
    MarkOverlaysChanged();
}

// Reconstructed from eboot.elf at 0x5F95A0.
RndOverlay* RndOverlayMgr::GetOverlay(Symbol name) {
    for (auto it = gOverlays.begin(); it != gOverlays.end(); ++it) {
        if (it->mName == name) {
            return &*it;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x5F95E0.
RndOverlay* RndOverlayMgr::TryGetOverlay(Symbol name) {
    for (auto it = gOverlays.begin(); it != gOverlays.end(); ++it) {
        if (it->mName == name) {
            return &*it;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x5F9620.
RndOverlayMgr::OverlayList::iterator RndOverlayMgr::Begin() {
    return gOverlays.begin();
}

// Reconstructed from eboot.elf at 0x5F9630.
RndOverlayMgr::OverlayList::iterator RndOverlayMgr::End() {
    return gOverlays.end();
}
