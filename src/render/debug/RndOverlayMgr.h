#pragma once

#include <cstddef>

#include "render/debug/RndOverlay.h"
#include "utl/containers/LinkedList.h"
#include "utl/messages/MsgSink.h"
#include "utl/text/Symbol.h"

// The registry of debug overlays. Every RndOverlay links itself into one
// static list on construction. While a shown overlay takes keyboard input,
// the manager installs its keyboard override, which offers each key to
// those overlays first.
class RndOverlayMgr {
public:
    // Name not in the reference map.
    using OverlayList = LinkedList::List<RndOverlay, RndOverlay::ListNode>;

    // The keyboard override. The vtable is at 0x192B0F8.
    class OverlayKeyboardOverride : public MsgSink {
    public:
        OverlayKeyboardOverride();  // 0x5F9640
        // Slots 0-1 at 0x5F9140 and 0x5F9800.
        ~OverlayKeyboardOverride() override {}

        // Offers a "key" message to the shown keyboard overlays in list
        // order and returns 0 once one takes it; otherwise passes the
        // message on to the previous override or to the subscribers.
        DataNode Handle(DataArray* msg, bool warn) override;  // 0x5F9660

        // The override replaced while this one is installed. Name not in
        // the reference map.
        MsgSink* mPrevious;
    };

    // Creates the built-in overlays.
    static void Init();  // 0x5F9150
    // Draws the shown overlays upwards from the bottom margin, separated by
    // one-pixel lines, with an identity view-projection.
    static void DrawAll(RndContext& context);  // 0x5F92D0
    // Updates the shown overlays and installs or removes the keyboard
    // override as keyboard overlays are shown or hidden. No caller has been
    // located in this build. Name not in the reference map.
    static void Poll();  // 0x5F9230

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
    // The overlay list, at 0x1AA7C70.
    static OverlayList gOverlays;
    // Whether the keyboard override is installed, at 0x1AA7C80.
    static bool gKeyboardFocus;
    // At 0x1AA7C88.
    static OverlayKeyboardOverride gKeyboardOverride;
};

static_assert(offsetof(RndOverlayMgr::OverlayKeyboardOverride, mPrevious) == 8);
static_assert(sizeof(RndOverlayMgr::OverlayKeyboardOverride) == 16);
