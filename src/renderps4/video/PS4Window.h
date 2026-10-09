#pragma once

#include <cstddef>

#include "render/system/RndWindow.h"
#include "renderps4/video/OrbisGpuRenderTarget.h"

class PS4Device;
class PS4Texture2D;

// The PS4 main window: two video-out back buffers wrapped in one texture
// and presented alternately. The vtable is at 0x195F708.
class PS4Window : public RndBufferedWindow {
public:
    PS4Window();             // 0x8E24A0
    ~PS4Window() override;   // 0x8E2860, 0x8E2870

    // Flips to the other back buffer; returns whether it is the second.
    bool AdvanceFrame();     // 0x8E2890

    // Allocates the back-buffer render targets, wraps them in a texture and
    // registers them with video output. Inlined into the constructor at
    // 0x8E24A0 in this binary.
    void _InitBuffers();

    // Back-buffer helpers, declared only. Names not in the reference map.
    static rb4::OrbisBackBufferSpecification _BackBufferSpecification(
        const PS4Device& device,
        rb4::OrbisBackBufferDataFormat dataFormat);
    static void _InitRenderTarget(
        rb4::OrbisGpuRenderTarget& target,
        const rb4::OrbisBackBufferSpecification& specification);
    static rb4::OrbisSizeAlign _RenderTargetSizeAlign(
        const rb4::OrbisGpuRenderTarget& target);
    static void* _AllocateBackBuffer(
        std::size_t size,
        const char* name,
        std::size_t alignment);
    static void _SetRenderTargetStorage(
        rb4::OrbisGpuRenderTarget& target,
        void* allocation);
    static void _DisableAuxiliarySurfaces(rb4::OrbisGpuRenderTarget& target);
    static PS4Texture2D* _WrapBackBufferTextures(
        rb4::OrbisGpuRenderTarget* targets,
        std::size_t targetCount);
    void _AttachBackBufferTexture(PS4Texture2D& texture);
    static void _RegisterBackBuffers(
        PS4Device& device,
        const rb4::OrbisGpuRenderTarget* targets,
        std::size_t targetCount);

    unsigned long mActiveBuffer;  // Name not in the reference map.
};

static_assert(offsetof(PS4Window, mActiveBuffer) == 32);
static_assert(sizeof(PS4Window) == 40);
