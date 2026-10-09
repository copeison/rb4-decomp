#pragma once

#include <cstddef>
#include <gnm/texture.h>
#include <gnm/depthrendertarget.h>
#include <gnm/rendertarget.h>

#include "render/textures/RndTextureCube.h"

class RndContext;

// The vtable is at 0x195FCF8.
class PS4TextureCube : public RndTextureCube {
public:
    explicit PS4TextureCube(const Description& desc);  // 0x8E6BA0
    ~PS4TextureCube() override;                         // 0x8E6BE0, 0x8E6CC0

    void _SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E71A0
    void _SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E71C0
    void _SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E71E0
    void _SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E7200
    void _SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E7220
    void _SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long extra) override;  // 0x8E7240
    // Not yet reconstructed.
    void _GpuCopyFromImpl(RndContext& context, RndShaderResource& source) override;  // 0x8E7260
    void _SyncStaticImpl(const RndTextureBase* reuse) override;  // 0x8E6CE0
    void _SyncDynamicImpl(RndContext& context) override;          // 0x8E7170
    void _SyncFromGpuImpl(RndContext& context) override;          // 0x8E7180
    void _Slot20Impl() override;                                  // 0x8E7190

    // Reconstructed from eboot.elf at 0x8E7270 and 0x8E7280.
    const sce::Gnm::RenderTarget* GetRenderTarget() const {
        return mRenderTarget;
    }
    const sce::Gnm::DepthRenderTarget* GetDepthStencilTarget() const {
        return mDepthTarget;
    }

    // Field names are not in the reference map. A depth cube keeps its
    // depth and stencil surfaces in mStorage and mStencilStorage.
    sce::Gnm::Texture* mGpuTexture;
    void* mStorage;
    void* mStencilStorage;
    sce::Gnm::RenderTarget* mRenderTarget;
    sce::Gnm::DepthRenderTarget* mDepthTarget;

private:
    // Both are inlined into _SyncStaticImpl. The map has them on
    // PS4Texture2D and PS4TextureArray2D only. Names not in the reference
    // map.
    void _SyncDepthStencil();
    void _SyncRegular();
};

static_assert(offsetof(PS4TextureCube, mGpuTexture) == 792);
static_assert(sizeof(PS4TextureCube) == 832);
