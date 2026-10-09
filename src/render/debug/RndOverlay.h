#pragma once

#include <cstddef>

#include "utl/containers/LinkedList.h"
#include "utl/text/Str.h"
#include "utl/text/Symbol.h"

class KeyboardKeyMsg;
class RndContext;
class TextStream;

// A debug overlay drawn over the frame, found by name in the overlay list.
// Constructing one registers it with RndOverlayMgr; destroying it
// unregisters it.
class RndOverlay {
public:
    // Overlay flags. Names not in the reference map.
    enum Flags : unsigned int {
        // While showing, the overlay takes keyboard input
        // (RndOverlayMgr's per-frame update at 0x5F9230).
        kFlagKeyboard = 1,
        // PrintHelp prints something; overlay_help skips the others.
        kFlagHasHelp = 2,
        // Read by the options component at 0x5F9AD0.
        kFlagUnknown8 = 8,
    };

    // Reaches the link of an overlay in the overlay list.
    class ListNode {
    public:
        static LinkedList::Node& ToNode(RndOverlay& overlay) {
            return overlay.mListNode;
        }
        static RndOverlay* FromNode(LinkedList::Node* node) {
            return reinterpret_cast<RndOverlay*>(
                reinterpret_cast<char*>(node) - offsetof(RndOverlay, mListNode));
        }
    };

    // The map's signature is RndOverlay(char const*, char const*, unsigned
    // int); this build has no second string.
    RndOverlay(const char* name, unsigned int flags);  // 0x6E5860

    virtual ~RndOverlay();  // slots 0-1: 0x6E58C0, 0x6E5910
    // Slot 2: pure. The map's subclasses implement it as
    // Draw(RndContext&, int).
    virtual void Draw(RndContext& context, int y) = 0;
    // Slot 3 at 0x6D16D0: returns whether the overlay consumed the key.
    virtual bool HandleKeyboardMsg(const KeyboardKeyMsg& msg) {
        static_cast<void>(msg);
        return false;
    }
    // Slot 4 at 0x6D16E0.
    virtual void PrintHelp(TextStream& stream) {
        static_cast<void>(stream);
    }
    // Slot 5 at 0x6D16F0: called at most once a frame by Update. Name not
    // in the reference map.
    virtual void _Update() {}
    // Slot 6 at 0x6D1700: called by SetShowing with the new state.
    virtual void _HandleShowingChanged(bool showing) {
        static_cast<void>(showing);
    }
    // Slot 7 at 0x6D1710; no override in this build. Name not in the
    // reference map.
    virtual void _Unknown7() {}

    // Shows or hides the overlay, notifying _HandleShowingChanged on a
    // change. Name not in the reference map.
    void SetShowing(bool showing);  // 0x6E5A90
    // Runs _Update once for the current frame. Name not in the reference
    // map.
    void Update();  // 0x6E5AB0

    bool IsShowing() const {  // Name not in the reference map.
        return mShowing;
    }
    Symbol GetName() const {  // Name not in the reference map.
        return mName;
    }
    unsigned int GetFlags() const {  // Name not in the reference map.
        return mFlags;
    }

    // Not located in this build; _OnToggleOverlay inlines a toggle through
    // SetShowing.
    void ToggleShowing();
    // Not located in this build.
    void FilterShowing();

    // Field names are not in the reference map.
    Symbol mName;
    // Display name; when empty the name is shown with underscores as
    // spaces (0x6E5960).
    String mDisplayName;
    unsigned int mFlags;
    bool mShowing;
    // RndDevice frame count of the last Update.
    unsigned long mUpdateFrame;
    LinkedList::Node mListNode;
};

static_assert(offsetof(RndOverlay, mName) == 8);
static_assert(offsetof(RndOverlay, mDisplayName) == 16);
static_assert(offsetof(RndOverlay, mFlags) == 32);
static_assert(offsetof(RndOverlay, mShowing) == 36);
static_assert(offsetof(RndOverlay, mUpdateFrame) == 40);
static_assert(offsetof(RndOverlay, mListNode) == 48);
static_assert(sizeof(RndOverlay) == 64);
