#pragma once

#include <cstddef>

#include "render/textures/RndPixelData.h"
#include "render/textures/RndTextureBase.h"

// The base vtable is at 0x1939C88.
class RndTexture1D : public RndTextureBase {
public:
    class Description : public RndTextureBase::Description {
    public:
        Description();  // 0x6F5A90

        RndPixelData mPixels;
    };

    // Reconstructed from eboot.elf at 0x6F5810. Resolves the description,
    // creates the platform texture, and syncs it.
    static RndTexture1D* New(Description& desc, const RndTexture1D* reuse);

    explicit RndTexture1D(const Description& desc);  // 0x6F5870
    ~RndTexture1D() override {}                     // 0x6F5970, 0x6F59B0

    unsigned long _GetNumMipsImpl() const override;                    // 0x6F59F0
    unsigned long _GetTotalBytesImpl() const override;                 // 0x6F5A00
    void _SetRequestedFormatImpl(const RndPixelFormat& format) override;  // 0x6F5A10
    void _FreePixelDataImpl() override;                                // 0x6F5A70

    RndTextureBase::Description mDesc;  // Name not in the reference map.
    RndPixelData mPixels;  // Name not in the reference map.
};

static_assert(offsetof(RndTexture1D, mDesc) == 168);
static_assert(sizeof(RndTexture1D) == 392);
