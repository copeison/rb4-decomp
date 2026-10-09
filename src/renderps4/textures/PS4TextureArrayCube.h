#pragma once

#include <cstddef>
#include <gnm/texture.h>

#include "render/textures/RndTextureArrayCube.h"

// The vtable is at 0x195FC40.
class PS4TextureArrayCube : public RndTextureArrayCube {
public:
    explicit PS4TextureArrayCube(const Description& desc);  // 0x8E6640
    ~PS4TextureArrayCube() override;                         // 0x8E6670, 0x8E66D0

    void _SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E6AB0
    void _SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E6AD0
    void _SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E6AF0
    void _SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E6B10
    void _SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E6B30
    void _SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long extra) override;  // 0x8E6B50
    void _GpuCopyFromImpl(RndContext& context, RndShaderResource& source) override;  // 0x8E6B70
    // The map has _SyncStaticImpl(); this build's slot takes a texture to
    // reuse.
    void _SyncStaticImpl(const RndTextureBase* reuse) override;  // 0x8E6730
    void _SyncDynamicImpl(RndContext& context) override;          // 0x8E6A80
    void _SyncFromGpuImpl(RndContext& context) override;          // 0x8E6A90
    void _Slot20Impl() override;                                  // 0x8E6AA0

    sce::Gnm::Texture* mGpuTexture;  // Name not in the reference map.
    void* mStorage;     // Name not in the reference map.
};

static_assert(offsetof(PS4TextureArrayCube, mGpuTexture) == 344);
static_assert(sizeof(PS4TextureArrayCube) == 360);
