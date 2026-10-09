#pragma once

#include <cstddef>
#include <gnm/texture.h>

#include "render/textures/RndTextureArray1D.h"

// The vtable is at 0x195FAD0.
class PS4TextureArray1D : public RndTextureArray1D {
public:
    explicit PS4TextureArray1D(const Description& desc);  // 0x8E5870
    ~PS4TextureArray1D() override;                         // 0x8E58A0, 0x8E5900

    void _SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E5C50
    void _SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E5C70
    void _SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E5C90
    void _SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E5CB0
    void _SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E5CD0
    void _SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long extra) override;  // 0x8E5CF0
    void _GpuCopyFromImpl(RndContext& context, RndShaderResource& source) override;  // 0x8E5D10
    // The map has _SyncStaticImpl(); this build's slot takes a texture to
    // reuse.
    void _SyncStaticImpl(const RndTextureBase* reuse) override;  // 0x8E5960
    void _SyncDynamicImpl(RndContext& context) override;          // 0x8E5C20
    void _SyncFromGpuImpl(RndContext& context) override;          // 0x8E5C30
    void _Slot20Impl() override;                                  // 0x8E5C40

    sce::Gnm::Texture* mGpuTexture;  // Name not in the reference map.
    void* mStorage;     // Name not in the reference map.
};

static_assert(offsetof(PS4TextureArray1D, mGpuTexture) == 344);
static_assert(sizeof(PS4TextureArray1D) == 360);
