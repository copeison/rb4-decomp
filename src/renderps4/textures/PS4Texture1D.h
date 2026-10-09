#pragma once

#include <cstddef>
#include <gnm/texture.h>

#include "render/textures/RndTexture1D.h"

// The vtable is at 0x195F960.
class PS4Texture1D : public RndTexture1D {
public:
    explicit PS4Texture1D(const Description& desc);  // 0x8E4F60
    ~PS4Texture1D() override;                         // 0x8E4F90, 0x8E4FF0

    void _SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E52D0
    void _SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E52F0
    void _SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E5310
    void _SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E5330
    void _SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E5350
    void _SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long extra) override;  // 0x8E5370
    // Not yet reconstructed.
    void _GpuCopyFromImpl(RndContext& context, RndShaderResource& source) override;  // 0x8E5390
    void _SyncStaticImpl(const RndTextureBase* reuse) override;  // 0x8E5050  // not yet reconstructed
    void _SyncDynamicImpl(RndContext& context) override;          // 0x8E52A0
    void _SyncFromGpuImpl(RndContext& context) override;          // 0x8E52B0
    void _Slot20Impl() override;                                  // 0x8E52C0

    sce::Gnm::Texture* mGpuTexture;  // Name not in the reference map.
    void* mStorage;     // Name not in the reference map.
};

static_assert(offsetof(PS4Texture1D, mGpuTexture) == 392);
static_assert(sizeof(PS4Texture1D) == 408);
