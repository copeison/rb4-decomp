#pragma once

#include <cstddef>
#include <gnm/rendertarget.h>

#include "render/system/RndWindow.h"

// The back buffers are 64-byte Gnm render-target register blocks.
static_assert(sizeof(sce::Gnm::RenderTarget) == 64);

// The PS4 main window: two video-out back buffers wrapped in one texture
// and presented alternately. The vtable is at 0x195F708.
class PS4Window : public RndBufferedWindow {
public:
    PS4Window();             // 0x8E24A0
    ~PS4Window() override;   // 0x8E2860, 0x8E2870

    // Flips to the other back buffer; returns whether it is the second.
    bool AdvanceFrame();     // 0x8E2890

    unsigned long mActiveBuffer;  // Name not in the reference map.
};

static_assert(offsetof(PS4Window, mActiveBuffer) == 32);
static_assert(sizeof(PS4Window) == 40);
