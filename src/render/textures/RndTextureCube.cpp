#include "render/textures/RndTextureCube.h"

#include "render/system/RndFactory.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"

// Reconstructed from eboot.elf at 0x6A1030.
RndTextureCube::Description::Description() {
    mType = kTextureCube;
}

// Reconstructed from eboot.elf at 0x6A0D10.
RndTextureCube* RndTextureCube::New(Description& desc, const RndTextureCube* reuse) {
    desc.ResolveFormat(kTextureCube, -1);
    desc.mCube.IsValid();
    auto* texture = TheRndFactory()->CreateTextureCube(desc);
    const auto& settings =
        *TheRndDevice()->mSettings;
    if (texture->mBaseDesc.mFormat.mUsage != kTextureUsageTiledLighting ||
        (texture->mBaseDesc.mFormat.mFlags & 2U) != 0 ||
        !settings.mUseTiledLighting) {
        texture->SyncStatic(reuse);
    }
    return texture;
}

// Reconstructed from eboot.elf at 0x6A0DA0. The base fields are copied
// back into the base description after the size and format of the first
// mip level are recorded.
RndTextureCube::RndTextureCube(const Description& desc)
    : mDesc(desc),
      mCube(desc.mCube, desc.ShouldKeepPixelData()) {
    mResourceIndex = 2;
    const auto& first = mCube.mFaces[0];
    mDesc.mDataFormat = first.mFormat;
    mDesc.mWidth = static_cast<unsigned int>(first.mSize.x);
    mDesc.mHeight = static_cast<unsigned int>(first.mSize.y);
    mDesc.mDepth = static_cast<unsigned int>(first.mSize.z);
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x6A0FA0.
unsigned long RndTextureCube::_GetNumMipsImpl() const {
    return mCube.mFaces[0].GetNumMips();
}

// Reconstructed from eboot.elf at 0x6A0FB0.
unsigned long RndTextureCube::_GetTotalBytesImpl() const {
    return mCube.GetTotalBytes();
}

// Reconstructed from eboot.elf at 0x6A0FC0.
void RndTextureCube::_SetRequestedFormatImpl(const RndPixelFormat& format) {
    mDesc.mRequestedFormat = format;
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x6A1020.
void RndTextureCube::_FreePixelDataImpl() {
    mCube.FreeBuffers();
}
