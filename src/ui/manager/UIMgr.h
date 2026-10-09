#pragma once

// The UI layout manager (ui/UIMgr.o). Only the methods the main loop calls
// are declared so far; the layout is not reconstructed.

// The map gives GetCurrentLayout no return type, so the layout type is a
// placeholder. It holds the pending skip-draw frame count at +0x4D8. Name not
// in the reference map.
class UILayout;

class UIMgr {
public:
    // Advances layout transitions and emits the transition lifecycle events.
    void Poll();  // 0x8C8900
    // Returns the active layout, or nullptr while layout access is disabled.
    UILayout* GetCurrentLayout() const;  // 0x8CA4C0
    // Builds and submits the UI rendering for the active layout.
    void Draw();  // 0x8C9A80
};

extern UIMgr* theUI;
