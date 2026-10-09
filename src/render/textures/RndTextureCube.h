#pragma once

#include <cstddef>

#include "render/textures/RndPixelDataCube.h"
#include "render/textures/RndTextureBase.h"

// The base vtable is at 0x1935CD0.
class RndTextureCube : public RndTextureBase {
public:
    class Description : public RndTextureBase::Description {
    public:
        Description();  // 0x6A1030

        RndPixelDataCube mCube;
    };

    // Reconstructed from eboot.elf at 0x6A0D10. Resolves the description,
    // creates the platform texture, and syncs it.
    static RndTextureCube* New(Description& desc, const RndTextureCube* reuse);

    explicit RndTextureCube(const Description& desc);  // 0x6A0DA0
    ~RndTextureCube() override {}                     // 0x6A0EA0, 0x6A0F10

    unsigned long _GetNumMipsImpl() const override;                    // 0x6A0FA0
    unsigned long _GetTotalBytesImpl() const override;                 // 0x6A0FB0
    void _SetRequestedFormatImpl(const RndPixelFormat& format) override;  // 0x6A0FC0
    void _FreePixelDataImpl() override;                                // 0x6A1020

    RndTextureBase::Description mDesc;  // Name not in the reference map.
    RndPixelDataCube mCube;  // Name not in the reference map.
};

static_assert(offsetof(RndTextureCube, mDesc) == 168);
static_assert(sizeof(RndTextureCube) == 792);
