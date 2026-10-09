#pragma once

#include <cstddef>

#include "render/debug/overlays/RndOverlayTextBase.h"

// A text overlay listing timers, with keyboard selection. In this build it
// is the base of the CPU and GPU timer overlays. The vtable is at 0x19399A0
// and has twelve slots; slot 11 (0x6E2250) is not declared. Only the
// declarations are recovered; the members are only sized.
class RndTimersOverlay : public RndOverlayTextBase {
public:
    // Adds kFlagKeyboard, kFlagHasHelp and 0x20 to `flags`. The map's
    // signature is RndTimersOverlay().
    RndTimersOverlay(const char* name, unsigned int flags);  // 0x6E7850
    ~RndTimersOverlay() override;  // slots 0-1: 0x6E78C0, 0x6E78F0

    bool HandleKeyboardMsg(const KeyboardKeyMsg& msg) override;  // slot 3: 0x6E7D40
    void PrintHelp(TextStream& stream) override;                 // slot 4: 0x6E8400
    void _Update() override;                                     // slot 5: 0x6E79C0
    void _Print(TextStream& stream) override;                    // slot 8: 0x6E8420
    Hmx::Color _GetBackgroundColor() override;                   // slot 10: 0x6E8740

    // An intrusive list headed at 72, a vector and flags. Name not in the
    // reference map.
    unsigned char mUnknown64[96];
};

static_assert(offsetof(RndTimersOverlay, mUnknown64) == 64);
static_assert(sizeof(RndTimersOverlay) == 160);

// The "cpu_timers" overlay. Name not in the reference map. The vtable is at
// 0x1939510; slots 11 to 15 are not declared.
class RndCpuTimersOverlay : public RndTimersOverlay {
public:
    RndCpuTimersOverlay();  // 0x6E1F90
    ~RndCpuTimersOverlay() override;  // slots 0-1: 0x6E1FC0, 0x6E1FD0

    void PrintHelp(TextStream& stream) override;  // slot 4: 0x6E1FF0
};

static_assert(sizeof(RndCpuTimersOverlay) == 160);

// The "gpu_timers" overlay. Name not in the reference map. The vtable is at
// 0x19396E8; slots 11 to 15 are not declared.
class RndGpuTimersOverlay : public RndTimersOverlay {
public:
    RndGpuTimersOverlay();  // 0x6E2F40
    ~RndGpuTimersOverlay() override;  // slots 0-1: 0x6E2F70, 0x6E2F80

    void PrintHelp(TextStream& stream) override;        // slot 4: 0x6E2FC0
    void _HandleShowingChanged(bool showing) override;  // slot 6: 0x6E2FA0
};

static_assert(sizeof(RndGpuTimersOverlay) == 160);
