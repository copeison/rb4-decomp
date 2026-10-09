#pragma once

#include <cstddef>

#include "render/textures/RndPixelData.h"
#include "utl/containers/Vector.h"
#include "render/textures/RndTextureBase.h"

// The base vtable is at 0x19350E0.
class RndTextureArray1D : public RndTextureBase {
public:
    // The caller's element list. Name not in the reference map.
    template <typename T>
    struct Range {
        const T* mBegin;
        const T* mEnd;
        const T* mCapacity;
    };

    class Description : public RndTextureBase::Description {
    public:
        Description();  // 0x6972D0

        Range<RndPixelData> mPixels;
    };

    // Reconstructed from eboot.elf at 0x696BA0. Resolves the description,
    // creates the platform texture, and syncs it.
    static RndTextureArray1D* New(Description& desc, const RndTextureArray1D* reuse);

    explicit RndTextureArray1D(const Description& desc);  // 0x696C00
    ~RndTextureArray1D() override {}                     // 0x697020, 0x6970A0

    unsigned long _GetNumMipsImpl() const override;                    // 0x697120
    unsigned long _GetTotalBytesImpl() const override;                 // 0x697130
    void _SetRequestedFormatImpl(const RndPixelFormat& format) override;  // 0x697180
    void _FreePixelDataImpl() override;                                // 0x6971E0

    RndTextureBase::Description mDesc;  // Name not in the reference map.
    eastl::vector<RndPixelData> mPixels;  // Name not in the reference map.

private:
    // Checks that every element matches the first. Name not in the reference
    // map.
    bool ValidateElements() const;
};

static_assert(offsetof(RndTextureArray1D, mDesc) == 168);
static_assert(sizeof(RndTextureArray1D) == 344);
