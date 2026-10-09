#include "render/debug/RndOverlayMgr.h"

#include "render/debug/RndOverlayOptionsCom.h"

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
RndOverlayMgr::OverlayList RndOverlayMgr::sOverlays;

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
    sOverlays.push_back(overlay);
    MarkOverlaysChanged();
}

// Reconstructed from eboot.elf at 0x5F9560.
void RndOverlayMgr::UnregisterOverlay(RndOverlay& overlay) {
    sOverlays.remove(overlay);
    MarkOverlaysChanged();
}

// Reconstructed from eboot.elf at 0x5F95A0.
RndOverlay* RndOverlayMgr::GetOverlay(Symbol name) {
    for (auto it = sOverlays.begin(); it != sOverlays.end(); ++it) {
        if (it->mName == name) {
            return &*it;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x5F95E0.
RndOverlay* RndOverlayMgr::TryGetOverlay(Symbol name) {
    for (auto it = sOverlays.begin(); it != sOverlays.end(); ++it) {
        if (it->mName == name) {
            return &*it;
        }
    }
    return nullptr;
}

// Reconstructed from eboot.elf at 0x5F9620.
RndOverlayMgr::OverlayList::iterator RndOverlayMgr::Begin() {
    return sOverlays.begin();
}

// Reconstructed from eboot.elf at 0x5F9630.
RndOverlayMgr::OverlayList::iterator RndOverlayMgr::End() {
    return sOverlays.end();
}
