#include "render/textures/RndTexture2D.h"

#include "render/system/RndFactory.h"
#include "utl/streams/BinStream.h"

// Reconstructed from eboot.elf at 0x690330.
RndTexture2D::Description::Description() {
    mType = kTexture2D;
}

// Reconstructed from eboot.elf at 0x68FE80.
RndTexture2D* RndTexture2D::New(Description& desc) {
    desc.ResolveFormat(kTexture2D, -1);
    auto* texture = TheRndFactory()->CreateTexture2D(desc);
    texture->SyncStatic(nullptr);
    return texture;
}

// Reconstructed from eboot.elf at 0x6900B0. The base fields are copied
// back into the base description after the size and format of the first
// mip level are recorded.
RndTexture2D::RndTexture2D(const Description& desc)
    : mDesc(desc),
      mPixels(desc.mPixels, desc.ShouldKeepPixelData()),
      mLinkedTexture(nullptr),
      mLinkedIndex(-1) {
    mResourceIndex = 0;
    const auto& first = mPixels;
    mDesc.mDataFormat = first.mFormat;
    mDesc.mWidth = static_cast<unsigned int>(first.mSize.x);
    mDesc.mHeight = static_cast<unsigned int>(first.mSize.y);
    mDesc.mDepth = static_cast<unsigned int>(first.mSize.z);
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x690270.
unsigned long RndTexture2D::_GetNumMipsImpl() const {
    return mPixels.GetNumMips();
}

// Reconstructed from eboot.elf at 0x690280.
unsigned long RndTexture2D::_GetTotalBytesImpl() const {
    return mPixels.GetTotalBytes();
}

// Reconstructed from eboot.elf at 0x690290.
void RndTexture2D::_SetRequestedFormatImpl(const RndPixelFormat& format) {
    mDesc.mRequestedFormat = format;
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x6902F0.
void RndTexture2D::_FreePixelDataImpl() {
    mPixels.FreeBuffers();
}

// Reconstructed from eboot.elf at 0x690310.
RndTextureBase* RndTexture2D::_GetLinkedTextureImpl(long& index) {
    if (mLinkedTexture == nullptr) {
        return this;
    }
    index = mLinkedIndex;
    return mLinkedTexture;
}

// Reconstructed from eboot.elf at 0x690300.
void RndTexture2D::_LoadBuffersImpl(BinStream& stream) {
    mPixels.LoadBuffers(stream);
}

void RndTexture2D::SetLinkedTexture(RndTextureBase* texture, long index) {
    mLinkedTexture = texture;
    mLinkedIndex = index;
}

// Reconstructed from eboot.elf at 0x690260.
void RndTexture2D::_ValidateGpuCopyFrom(const RndShaderResource& source) {
    _ValidateGpuCopyFromBase(source);
}
