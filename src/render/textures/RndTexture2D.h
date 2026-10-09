#pragma once

#include <cstddef>

#include "render/textures/RndPixelData.h"
#include "render/textures/RndTextureBase.h"

// The base vtable is at 0x19349C0.
class RndTexture2D : public RndTextureBase {
public:
    class Description : public RndTextureBase::Description {
    public:
        Description();  // 0x690330

        RndPixelData mPixels;
    };

    // Reconstructed from eboot.elf at 0x68FE80. Resolves the description,
    // creates the platform texture, and syncs it.
    static RndTexture2D* New(Description& desc);

    explicit RndTexture2D(const Description& desc);  // 0x6900B0
    ~RndTexture2D() override {}                     // 0x6901D0, 0x690210

    unsigned long _GetNumMipsImpl() const override;                    // 0x690270
    unsigned long _GetTotalBytesImpl() const override;                 // 0x690280
    void _SetRequestedFormatImpl(const RndPixelFormat& format) override;  // 0x690290
    void _FreePixelDataImpl() override;                                // 0x6902F0

    RndTextureBase* _GetLinkedTextureImpl(long& index) override;  // 0x690310
    void _LoadBuffersImpl(BinStream& stream) override;              // 0x690300

    // Reconstructed from eboot.elf at 0x690250. Name not in the reference
    // map.
    void SetLinkedTexture(RndTextureBase* texture, long index);

    RndTextureBase::Description mDesc;  // Name not in the reference map.
    RndPixelData mPixels;  // Name not in the reference map.
    RndTextureBase* mLinkedTexture;  // Name not in the reference map.
    long mLinkedIndex;               // Name not in the reference map.
};

static_assert(offsetof(RndTexture2D, mDesc) == 168);
static_assert(offsetof(RndTexture2D, mLinkedTexture) == 392);
static_assert(sizeof(RndTexture2D) == 408);
