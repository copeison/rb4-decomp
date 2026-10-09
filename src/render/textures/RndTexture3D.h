#pragma once

#include <cstddef>

#include "render/textures/RndPixelData.h"
#include "render/textures/RndTextureBase.h"

// The base vtable is at 0x1939D40.
class RndTexture3D : public RndTextureBase {
public:
    class Description : public RndTextureBase::Description {
    public:
        Description();  // 0x6F5ED0

        RndPixelData mPixels;
    };

    // Reconstructed from eboot.elf at 0x6F5C50. Resolves the description,
    // creates the platform texture, and syncs it.
    static RndTexture3D* New(Description& desc, const RndTexture3D* reuse);

    explicit RndTexture3D(const Description& desc);  // 0x6F5CB0
    ~RndTexture3D() override {}                     // 0x6F5DB0, 0x6F5DF0

    unsigned long _GetNumMipsImpl() const override;                    // 0x6F5E40
    unsigned long _GetTotalBytesImpl() const override;                 // 0x6F5E50
    void _SetRequestedFormatImpl(const RndPixelFormat& format) override;  // 0x6F5E60
    void _FreePixelDataImpl() override;                                // 0x6F5EC0

    // Defers to _ValidateGpuCopyFromBase.
    void _ValidateGpuCopyFrom(const RndShaderResource& source);  // 0x6F5E30

    RndTextureBase::Description mDesc;  // Name not in the reference map.
    RndPixelData mPixels;  // Name not in the reference map.
};

static_assert(offsetof(RndTexture3D, mDesc) == 168);
static_assert(sizeof(RndTexture3D) == 392);
