#pragma once

#include <cstddef>
#include <cstdint>
#include <gnm/dataformats.h>
#include <gnm/gpumem.h>
#include <gnm/rendertarget.h>

#include "render/system/RndWindow.h"

class PS4Device;
class PS4Texture2D;

// Back-buffer surface parameters passed from _BackBufferSpecification to
// _InitRenderTarget. Declared only; it is narrower than the SDK's
// sce::Gnm::RenderTargetSpec. Name not in the reference map.
struct OrbisBackBufferSpecification {
    std::uint32_t mWidth;
    std::uint32_t mHeight;
    sce::Gnm::DataFormat mDataFormat;
    std::uint32_t mTileMode;
    std::uint32_t mGpuMode;
};

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

    // Allocates the back-buffer render targets, wraps them in a texture and
    // registers them with video output. Inlined into the constructor at
    // 0x8E24A0 in this binary.
    void _InitBuffers();

    // Back-buffer helpers, declared only. Names not in the reference map.
    static OrbisBackBufferSpecification _BackBufferSpecification(
        const PS4Device& device,
        sce::Gnm::DataFormat dataFormat);
    static void _InitRenderTarget(
        sce::Gnm::RenderTarget& target,
        const OrbisBackBufferSpecification& specification);
    static sce::Gnm::SizeAlign _RenderTargetSizeAlign(
        const sce::Gnm::RenderTarget& target);
    static void* _AllocateBackBuffer(
        std::size_t size,
        const char* name,
        std::size_t alignment);
    static void _SetRenderTargetStorage(
        sce::Gnm::RenderTarget& target,
        void* allocation);
    static void _DisableAuxiliarySurfaces(sce::Gnm::RenderTarget& target);
    static PS4Texture2D* _WrapBackBufferTextures(
        sce::Gnm::RenderTarget* targets,
        std::size_t targetCount);
    void _AttachBackBufferTexture(PS4Texture2D& texture);
    static void _RegisterBackBuffers(
        PS4Device& device,
        const sce::Gnm::RenderTarget* targets,
        std::size_t targetCount);

    unsigned long mActiveBuffer;  // Name not in the reference map.
};

static_assert(offsetof(PS4Window, mActiveBuffer) == 32);
static_assert(sizeof(PS4Window) == 40);
