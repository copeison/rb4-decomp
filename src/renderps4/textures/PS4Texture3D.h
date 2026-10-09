#pragma once

#include <cstddef>
#include <gnm/texture.h>

#include "render/textures/RndTexture3D.h"

// The vtable is at 0x195FA18.
class PS4Texture3D : public RndTexture3D {
public:
    explicit PS4Texture3D(const Description& desc);  // 0x8E53C0
    ~PS4Texture3D() override;                         // 0x8E53F0, 0x8E5450

    void _SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E5770
    void _SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E5790
    void _SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E57B0
    void _SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E57D0
    void _SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;  // 0x8E57F0
    void _SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long extra) override;  // 0x8E5810
    // Not yet reconstructed.
    void _GpuCopyFromImpl(RndContext& context, RndShaderResource& source) override;  // 0x8E5830
    void _SyncStaticImpl(const RndTextureBase* reuse) override;  // 0x8E54B0  // not yet reconstructed
    void _SyncDynamicImpl(RndContext& context) override;          // 0x8E5740
    void _SyncFromGpuImpl(RndContext& context) override;          // 0x8E5750
    void _Slot20Impl() override;                                  // 0x8E5760

    sce::Gnm::Texture* mGpuTexture;  // Name not in the reference map.
    void* mStorage;     // Name not in the reference map.
};

static_assert(offsetof(PS4Texture3D, mGpuTexture) == 392);
static_assert(sizeof(PS4Texture3D) == 408);
