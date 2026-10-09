#pragma once

#include <cstddef>

#include "render/textures/RndPixelData.h"
#include "utl/containers/Vector.h"
#include "render/textures/RndTextureBase.h"

// The base vtable is not referenced by a recovered constructor.
class RndTextureArray2D : public RndTextureBase {
public:
    class Description : public RndTextureBase::Description {
    public:
        Description();  // 0x698770

        // Owned by the description and copied by the texture.
        eastl::vector<RndPixelData> mPixels;
    };

    // Reconstructed from eboot.elf at 0x698100. Resolves the description,
    // creates the platform texture, and syncs it.
    static RndTextureArray2D* New(Description& desc);

    explicit RndTextureArray2D(const Description& desc);  // 0x698160
    ~RndTextureArray2D() override {}                     // 0x6984C0, 0x698540

    unsigned long _GetNumMipsImpl() const override;                    // 0x6985D0
    unsigned long _GetTotalBytesImpl() const override;                 // 0x6985E0
    void _SetRequestedFormatImpl(const RndPixelFormat& format) override;  // 0x698630
    void _FreePixelDataImpl() override;                                // 0x698690

    // Defers to _ValidateGpuCopyFromBase.
    void _ValidateGpuCopyFrom(const RndShaderResource& source);  // 0x6985C0

    RndTextureBase::Description mDesc;  // Name not in the reference map.
    eastl::vector<RndPixelData> mPixels;  // Name not in the reference map.

private:
    // Checks that every element matches the first. Name not in the reference
    // map.
    bool ValidateElements() const;
};

static_assert(offsetof(RndTextureArray2D, mDesc) == 168);
static_assert(sizeof(RndTextureArray2D) == 344);
