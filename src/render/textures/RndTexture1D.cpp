#include "render/textures/RndTexture1D.h"

#include "render/system/RndFactory.h"

// Reconstructed from eboot.elf at 0x6F5A90.
RndTexture1D::Description::Description() {
    mType = kTexture1D;
}

// Reconstructed from eboot.elf at 0x6F5810.
RndTexture1D* RndTexture1D::New(Description& desc, const RndTexture1D* reuse) {
    desc.ResolveFormat(kTexture1D, -1);
    auto* texture = TheRndFactory()->CreateTexture1D(desc);
    texture->SyncStatic(reuse);
    return texture;
}

// Reconstructed from eboot.elf at 0x6F5870. The base fields are copied
// back into the base description after the size and format of the first
// mip level are recorded.
RndTexture1D::RndTexture1D(const Description& desc)
    : mDesc(desc),
      mPixels(desc.mPixels, desc.ShouldKeepPixelData()) {
    const auto& first = mPixels;
    mDesc.mDataFormat = first.mFormat;
    mDesc.mWidth = static_cast<unsigned int>(first.mSize.x);
    mDesc.mHeight = static_cast<unsigned int>(first.mSize.y);
    mDesc.mDepth = static_cast<unsigned int>(first.mSize.z);
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x6F59F0.
unsigned long RndTexture1D::_GetNumMipsImpl() const {
    return mPixels.GetNumMips();
}

// Reconstructed from eboot.elf at 0x6F5A00.
unsigned long RndTexture1D::_GetTotalBytesImpl() const {
    return mPixels.GetTotalBytes();
}

// Reconstructed from eboot.elf at 0x6F5A10.
void RndTexture1D::_SetRequestedFormatImpl(const RndPixelFormat& format) {
    mDesc.mRequestedFormat = format;
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x6F5A70.
void RndTexture1D::_FreePixelDataImpl() {
    mPixels.FreeBuffers();
}

// Reconstructed from eboot.elf at 0x6F5A80.
void RndTexture1D::_ValidateGpuCopyFrom(const RndShaderResource& source) {
    _ValidateGpuCopyFromBase(source);
}
