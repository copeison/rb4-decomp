#pragma once

#include <cstddef>

#include "render/textures/RndPixelDataCube.h"
#include "utl/containers/Vector.h"
#include "render/textures/RndTextureBase.h"

// The base vtable is at 0x19355C0.
class RndTextureArrayCube : public RndTextureBase {
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
        Description();  // 0x69AF00

        Range<RndPixelDataCube> mCubes;
    };

    // Reconstructed from eboot.elf at 0x69AA60. Resolves the description,
    // creates the platform texture, and syncs it.
    static RndTextureArrayCube* New(Description& desc, const RndTextureArrayCube* reuse);

    explicit RndTextureArrayCube(const Description& desc);  // 0x69AAC0
    ~RndTextureArrayCube() override {}                     // 0x69ACD0, 0x69AD10

    unsigned long _GetNumMipsImpl() const override;                    // 0x69AD60
    unsigned long _GetTotalBytesImpl() const override;                 // 0x69AD70
    void _SetRequestedFormatImpl(const RndPixelFormat& format) override;  // 0x69ADC0
    void _FreePixelDataImpl() override;                                // 0x69AE20

    // Defers to _ValidateGpuCopyFromBase.
    void _ValidateGpuCopyFrom(const RndShaderResource& source);  // 0x69AD50

    RndTextureBase::Description mDesc;  // Name not in the reference map.
    eastl::vector<RndPixelDataCube> mCubes;  // Name not in the reference map.

private:
    // Checks that every element matches the first. Name not in the reference
    // map.
    bool ValidateElements() const;
    // Reconstructed from eboot.elf at 0x69AF50, which builds the texture's
    // description and cube vector. Name not in the reference map.
    void InitDescription(const Description& desc, bool keepPixels);
};

static_assert(offsetof(RndTextureArrayCube, mDesc) == 168);
static_assert(sizeof(RndTextureArrayCube) == 344);
