#pragma once

#include <cstddef>
#include <cstdint>
#include <gnm/depthrendertarget.h>
#include <gnm/rendertarget.h>
#include <gnm/texture.h>

#include "render/textures/RndTextureArray2D.h"

// The vtable is at 0x195FB88.
class PS4TextureArray2D : public RndTextureArray2D {
public:
    explicit PS4TextureArray2D(const Description& desc);  // 0x8E5D40
    ~PS4TextureArray2D() override;                         // 0x8E5D80, 0x8E5E50

    void _SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E64C0
    void _SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E64E0
    void _SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E6500
    void _SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E6520
    void _SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E6540
    void _SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long extra) override;  // 0x8E6560
    void _GpuCopyFromImpl(RndContext& context, RndShaderResource& source) override;  // 0x8E6580
    void _SyncStaticImpl(const RndTextureBase* reuse) override;  // 0x8E5E70
    void _SyncDynamicImpl(RndContext& context) override;          // 0x8E6490
    void _SyncFromGpuImpl(RndContext& context) override;          // 0x8E64A0
    void _Slot20Impl() override;                                  // 0x8E64B0

    // The color and depth targets viewing one slice, or the targets as they
    // are for a slice of -1.
    const sce::Gnm::RenderTarget* GetRenderTarget(unsigned long slice) const {  // 0x8E6590
        if (slice != static_cast<unsigned long>(-1)) {
            mColorTarget->setArrayView(
                static_cast<std::uint32_t>(slice), static_cast<std::uint32_t>(slice));
        }
        return mColorTarget;
    }
    const sce::Gnm::DepthRenderTarget* GetDepthStencilTarget(unsigned long slice) const {  // 0x8E65C0
        if (slice != static_cast<unsigned long>(-1)) {
            mDepthTarget->setArrayView(
                static_cast<std::uint32_t>(slice), static_cast<std::uint32_t>(slice));
        }
        return mDepthTarget;
    }

    // Field names are not in the reference map.
    sce::Gnm::Texture* mGpuTexture;
    void* mStorage;
    void* mStencilStorage;
    void* mHtileStorage;
    sce::Gnm::RenderTarget* mColorTarget;
    sce::Gnm::DepthRenderTarget* mDepthTarget;

private:
    // Inlined into _SyncStaticImpl. The map has _SyncDepthStencil and
    // _SyncRegular with description, pixel-data and format parameters.
    void _SyncDepthStencil();
    void _SyncRegular();
};

static_assert(offsetof(PS4TextureArray2D, mGpuTexture) == 344);
static_assert(sizeof(PS4TextureArray2D) == 392);
