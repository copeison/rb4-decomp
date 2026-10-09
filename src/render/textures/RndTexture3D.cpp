#include "render/textures/RndTexture3D.h"

#include "render/system/RndFactory.h"

// Reconstructed from eboot.elf at 0x6F5ED0.
RndTexture3D::Description::Description() {
    mType = kTexture3D;
}

// Reconstructed from eboot.elf at 0x6F5C50.
RndTexture3D* RndTexture3D::New(Description& desc, const RndTexture3D* reuse) {
    desc.ResolveFormat(kTexture3D, -1);
    auto* texture = TheRndFactory()->CreateTexture3D(desc);
    texture->SyncStatic(reuse);
    return texture;
}

// Reconstructed from eboot.elf at 0x6F5CB0. The base fields are copied
// back into the base description after the size and format of the first
// mip level are recorded.
RndTexture3D::RndTexture3D(const Description& desc)
    : mDesc(desc),
      mPixels(desc.mPixels, desc.ShouldKeepPixelData()) {
    const auto& first = mPixels;
    mDesc.mDataFormat = first.mFormat;
    mDesc.mWidth = static_cast<unsigned int>(first.mSize.x);
    mDesc.mHeight = static_cast<unsigned int>(first.mSize.y);
    mDesc.mDepth = static_cast<unsigned int>(first.mSize.z);
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x6F5E40.
unsigned long RndTexture3D::_GetNumMipsImpl() const {
    return mPixels.GetNumMips();
}

// Reconstructed from eboot.elf at 0x6F5E50.
unsigned long RndTexture3D::_GetTotalBytesImpl() const {
    return mPixels.GetTotalBytes();
}

// Reconstructed from eboot.elf at 0x6F5E60.
void RndTexture3D::_SetRequestedFormatImpl(const RndPixelFormat& format) {
    mDesc.mRequestedFormat = format;
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x6F5EC0.
void RndTexture3D::_FreePixelDataImpl() {
    mPixels.FreeBuffers();
}
