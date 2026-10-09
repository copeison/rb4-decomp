#include "render/debug/RndOverlayMgr.h"

#include "os/joypads/Keyboard.h"
#include "render/context/RndContext.h"
#include "render/debug/RndOverlayOptionsCom.h"
#include "render/drawing/RndDrawUtl.h"
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
    if (theRndOverlayOpts != nullptr) {
        theRndOverlayOpts->mOverlaysChanged = true;
    }
}

}  // namespace

// Static initializer at 0x5F9810; the destructor is at 0x5F9120.
RndOverlayMgr::OverlayList RndOverlayMgr::gOverlays;
bool RndOverlayMgr::gKeyboardFocus = false;
// Constructed by the same static initializer.
RndOverlayMgr::OverlayKeyboardOverride RndOverlayMgr::gKeyboardOverride;

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

// Reconstructed from eboot.elf at 0x5F9230. The override's previous sink
// is restored when the last keyboard overlay is hidden.
void RndOverlayMgr::Poll() {
    bool keyboard = false;
    for (auto it = gOverlays.begin(); it != gOverlays.end(); ++it) {
        if (it->IsShowing()) {
            if ((it->GetFlags() & RndOverlay::kFlagKeyboard) != 0) {
                keyboard = true;
            }
            it->Update();
        }
    }
    if (keyboard != gKeyboardFocus) {
        gKeyboardFocus = keyboard;
        if (keyboard) {
            gKeyboardOverride.mPrevious = KeyboardOverride(&gKeyboardOverride);
        } else {
            KeyboardOverride(gKeyboardOverride.mPrevious);
            gKeyboardOverride.mPrevious = nullptr;
        }
    }
}

// Reconstructed from eboot.elf at 0x5F92D0. Each overlay returns the line
// above itself.
void RndOverlayMgr::DrawAll(RndContext& context) {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Overlay Mgr");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());
    const bool identity = context.mUsingIdentityViewProjection;
    context.SetUsingIdentityViewProjection(true);

    int y = static_cast<int>(context.mViewportSize.y) - GetMarginInPixels();
    bool first = true;
    for (auto it = gOverlays.begin(); it != gOverlays.end(); ++it) {
        if (!it->IsShowing()) {
            continue;
        }
        if (!first) {
            RndDrawUtl::Line2DParams params;
            params.mCoordinateMode = RndDrawUtl::kCoordinatePixels;
            const float lineY = static_cast<float>(y);
            const Segment2D separator = {{0.0F, lineY}, {context.mViewportSize.x, lineY}};
            RndDrawUtl::DrawLine2D(context, separator, params);
            --y;
        }
        first = false;
        y = it->Draw(context, y);
    }

    context.SetUsingIdentityViewProjection(identity);
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

// Reconstructed from eboot.elf at 0x5F9640.
RndOverlayMgr::OverlayKeyboardOverride::OverlayKeyboardOverride() : mPrevious(nullptr) {}

// Reconstructed from eboot.elf at 0x5F9660. The previous override is read
// through gKeyboardOverride rather than this object, as the binary does.
DataNode RndOverlayMgr::OverlayKeyboardOverride::Handle(DataArray* msg, bool warn) {
    static_cast<void>(warn);
    if (msg->Sym(1) == KeyboardKeyMsg::Event()) {
        for (auto it = gOverlays.begin(); it != gOverlays.end(); ++it) {
            if (it->IsShowing() && (it->GetFlags() & RndOverlay::kFlagKeyboard) != 0) {
                KeyboardKeyMsg keyMsg(msg);
                const bool handled = it->HandleKeyboardMsg(keyMsg);
                if (handled) {
                    return DataNode(0);
                }
            }
        }
    }
    if (gKeyboardOverride.mPrevious == nullptr) {
        KeyboardSendMsgBypassOverride(msg);
        return DataNode(0);
    }
    return gKeyboardOverride.mPrevious->Handle(msg, true);
}
