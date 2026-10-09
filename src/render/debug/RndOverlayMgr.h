#pragma once

#include "render/debug/RndOverlay.h"
#include "utl/containers/LinkedList.h"
#include "utl/text/Symbol.h"

// The registry of debug overlays. Every RndOverlay links itself into one
// static list on construction. The list operations and the layout constants
// are reconstructed; Init and DrawAll are declared from the map but not
// located.
class RndOverlayMgr {
public:
    // Name not in the reference map.
    using OverlayList = LinkedList::List<RndOverlay, RndOverlay::ListNode>;

    static void Init();
    static void DrawAll(RndContext& context);

    static int GetMarginInPixels();           // 0x5F9210
    static int GetVerticalSpacingInPixels();  // 0x5F9220

    // Appends the overlay to the list.
    static void RegisterOverlay(RndOverlay& overlay);    // 0x5F9520
    static void UnregisterOverlay(RndOverlay& overlay);  // 0x5F9560
    // The overlay with the name. The failure report is compiled out, so a
    // missing name returns null like TryGetOverlay.
    static RndOverlay* GetOverlay(Symbol name);     // 0x5F95A0
    static RndOverlay* TryGetOverlay(Symbol name);  // 0x5F95E0

    static OverlayList::iterator Begin();  // 0x5F9620
    static OverlayList::iterator End();    // 0x5F9630

private:
    // The overlay list, at 0x1AA7C70. Name not in the reference map.
    static OverlayList sOverlays;
};
