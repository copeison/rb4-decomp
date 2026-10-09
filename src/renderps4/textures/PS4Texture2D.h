#pragma once

#include <cstddef>
#include <memory>
#include <gnm/texture.h>
#include <gnm/depthrendertarget.h>
#include <gnm/rendertarget.h>

#include "render/textures/RndTexture2D.h"

class RndContext;

// The vtable is at 0x195EC70.
class PS4Texture2D : public RndTexture2D {
public:
    explicit PS4Texture2D(const Description& desc);  // 0x8D62C0

    // Wraps the two video-out back buffers in one double-buffered texture.
    // Name from the map; this build returns the new texture.
    static PS4Texture2D* CreateAsBackBuffer(sce::Gnm::RenderTarget** targets);  // 0x8D5E30
    ~PS4Texture2D() override;                         // 0x8D6310, 0x8D6440

    void _SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8D6E40
    void _SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8D6F10
    void _SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8D6F80
    void _SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8D6FF0
    void _SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8D7060
    void _SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long extra) override;  // 0x8D70D0
    // Not yet reconstructed.
    void _GpuCopyFromImpl(RndContext& context, RndShaderResource& source) override;  // 0x8D7140
    void _SyncStaticImpl(const RndTextureBase* reuse) override;  // 0x8D6460
    void _SyncDynamicImpl(RndContext& context) override;          // 0x8D6D10
    void _SyncFromGpuImpl(RndContext& context) override;          // 0x8D6E20
    void _Slot20Impl() override;                                  // 0x8D6E30

    // Reconstructed from eboot.elf at 0x8D71E0: the active frame's target,
    // falling back to the first. Name from the map's GetRenderTarget.
    const sce::Gnm::RenderTarget* GetRenderTarget() const;
    // Reconstructed from eboot.elf at 0x8D7240.
    const sce::Gnm::DepthRenderTarget* GetDepthStencilTarget() const {
        return mDepthTarget;
    }
    // Presentation counts per back buffer. Names not in the reference map.
    void AddPendingPresentation(unsigned long buffer) {
        ++mPendingPresentations[buffer];
    }
    void CompletePendingPresentation(unsigned long buffer) {
        --mPendingPresentations[buffer];
    }

    // The GPU memory of the texture: one surface per buffer, or the depth,
    // stencil and HTILE surfaces of a depth texture. Shared between textures
    // that reuse each other's memory. Names not in the reference map.
    struct Storage {
        ~Storage();  // 0x8D7260

        void* mSurfaces[2];
        unsigned long mSurfaceSizes[2];
        void* mStencil;
        unsigned long mStencilSize;
        void* mHtile;
        unsigned long mHtileSize;
    };

    // Field names are not in the reference map. A depth texture keeps its
    // depth view in mGpuTextures[0] and its stencil view in mPlaneTexture;
    // the three size fields then describe the depth, stencil and HTILE
    // surfaces.
    sce::Gnm::Texture* mGpuTextures[2];
    sce::Gnm::Texture* mPlaneTexture;
    sce::Gnm::SizeAlign mSizeAlign;
    sce::Gnm::SizeAlign mStencilSizeAlign;
    sce::Gnm::SizeAlign mHtileSizeAlign;
    unsigned long mActiveStorage;
    // The binary's SDK 2.500 shared_ptr is 16 bytes and leaves +480 unused;
    // SDK 5.500's is 24 bytes and covers it.
    std::shared_ptr<Storage> mStorage;
    sce::Gnm::RenderTarget* mRenderTargets[2];
    sce::Gnm::DepthRenderTarget* mDepthTarget;
    int* mPendingPresentations;

private:
    // Inlined into _SyncStaticImpl at 0x8D6460. The map has
    // _SyncDepthStencil(RndPixelData const&, RndPixelFormat const&) and
    // _SyncRegular(RndPixelData const&, RndPixelFormat const&).
    void _SyncDepthStencil();
    void _SyncRegular(const RndTextureBase* reuse);
    // Inlined into the stage selects. Name not in the reference map.
    sce::Gnm::Texture* SelectView(unsigned int flags) const;
};

static_assert(sizeof(PS4Texture2D::Storage) == 64);
static_assert(offsetof(PS4Texture2D, mSizeAlign) == 432);
static_assert(offsetof(PS4Texture2D, mActiveStorage) == 456);
static_assert(offsetof(PS4Texture2D, mStorage) == 464);
static_assert(offsetof(PS4Texture2D, mRenderTargets) == 488);
static_assert(offsetof(PS4Texture2D, mPendingPresentations) == 512);
static_assert(offsetof(PS4Texture2D, mGpuTextures) == 408);
static_assert(sizeof(PS4Texture2D) == 520);
