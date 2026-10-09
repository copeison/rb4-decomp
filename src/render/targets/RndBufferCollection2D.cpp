#include "render/targets/RndBufferCollection.h"

#include <new>

#include "render/system/RndDevice.h"
#include "render/system/RndFactory.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTextureArray2D.h"
#include "utl/containers/Std.h"

namespace {

// The description arrives freshly constructed.
void DescribeTarget(
    RndTextureBase::Description& desc,
    int type,
    const RndPixelFormat& format,
    const char* name,
    int attachment,
    unsigned int targetFlags) {
    desc.mType = type;
    desc.mRequestedFormat = format;
    desc.mTargetFlags = targetFlags;
    desc.mAttachmentIndex = attachment;
    desc.mName = name;
}

void DescribePixels(RndPixelData& pixels, const Vector2i& size, int dataFormat) {
    pixels.mSize.x = size.x;
    pixels.mSize.y = size.y;
    pixels.mSize.z = 1;
    pixels.mFormat = dataFormat;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6B40A0.
RndBufferCollection2D::RndBufferCollection2D(unsigned int flags, int targetMode)
    : RndBufferCollection(flags, targetMode) {}

// Reconstructed from eboot.elf at 0x6B40D0.
RndBufferCollection2D::~RndBufferCollection2D() {}

// Reconstructed from eboot.elf at 0x6B4100.
bool RndBufferCollection2D::_ValidateBackBufferImpl(
    const RndTextureBase& backBuffer) {
    return backBuffer._GetTypeImpl() ==
        RndTextureBase::kTexture2D;
}

// Reconstructed from eboot.elf at 0x6B4120.
RndTextureBase* RndBufferCollection2D::_AllocBufferImpl(
    const char* name,
    const RndPixelFormat& format,
    int dataFormat,
    const Vector2i& size,
    int attachment,
    unsigned int targetFlags,
    RndTextureBase* reuse) {
    RndTexture2D::Description desc;
    DescribeTarget(
        desc, RndTextureBase::kTexture2D, format, name, attachment, targetFlags);
    DescribePixels(desc.mPixels, size, dataFormat);
    desc.ResolveFormat(RndTextureBase::kTexture2D, -1);

    auto* texture = TheRndDevice()->mFactory->CreateTexture2D(desc);
    texture->SyncStatic(reuse);
    return texture;
}

// Reconstructed from eboot.elf at 0x6B41F0.
RndTextureBase* RndBufferCollection2D::_AllocBufferArrayImpl(
    const char* name,
    const RndPixelFormat& format,
    int dataFormat,
    const Vector2i& size,
    unsigned long count,
    int attachment,
    unsigned int targetFlags,
    RndTextureBase* reuse) {
    RndTextureArray2D::Description desc;
    DescribeTarget(
        desc,
        RndTextureBase::kTextureArray2D,
        format,
        name,
        attachment,
        targetFlags);

    desc.mPixels.resize(count);
    for (unsigned long i = 0; i < count; ++i) {
        DescribePixels(desc.mPixels[i], size, dataFormat);
    }
    desc.ResolveFormat(RndTextureBase::kTextureArray2D, -1);

    auto* texture = TheRndDevice()->mFactory->CreateTextureArray2D(desc);
    texture->SyncStatic(reuse);
    return texture;
}
