#include "render/textures/RndTextureArray2D.h"

#include "render/system/RndFactory.h"

// Reconstructed from eboot.elf at 0x698770.
RndTextureArray2D::Description::Description() {
    mType = kTextureArray2D;
}

// Reconstructed from eboot.elf at 0x698100.
RndTextureArray2D* RndTextureArray2D::New(Description& desc) {
    desc.ResolveFormat(kTextureArray2D, -1);
    auto* texture = TheRndFactory()->CreateTextureArray2D(desc);
    texture->SyncStatic(nullptr);
    return texture;
}

// Reconstructed from eboot.elf at 0x698160. The base fields are copied
// back into the base description after the size and format of the first
// mip level are recorded.
RndTextureArray2D::RndTextureArray2D(const Description& desc)
    : mDesc(desc) {
    mPixels.reserve(desc.mPixels.size());
    for (const RndPixelData& pixels : desc.mPixels) {
        mPixels.emplace_back(pixels, desc.ShouldKeepPixelData());
    }
    ValidateElements();
    mResourceIndex = 0;
    const auto& first = mPixels.front();
    mDesc.mDataFormat = first.mFormat;
    mDesc.mWidth = static_cast<unsigned int>(first.mSize.x);
    mDesc.mHeight = static_cast<unsigned int>(first.mSize.y);
    mDesc.mDepth = static_cast<unsigned int>(first.mSize.z);
    mDesc.mArraySize = mPixels.size();
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x6985D0.
unsigned long RndTextureArray2D::_GetNumMipsImpl() const {
    return mPixels.front().GetNumMips();
}

// Reconstructed from eboot.elf at 0x6985E0.
unsigned long RndTextureArray2D::_GetTotalBytesImpl() const {
    return mPixels.empty() ? 0 : mPixels.size() * mPixels.front().GetTotalBytes();
}

// Reconstructed from eboot.elf at 0x698630.
void RndTextureArray2D::_SetRequestedFormatImpl(const RndPixelFormat& format) {
    mDesc.mRequestedFormat = format;
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x698690.
void RndTextureArray2D::_FreePixelDataImpl() {
    for (auto& pixels : mPixels) {
        pixels.FreeBuffers();
    }
}

// Reconstructed from eboot.elf at 0x698910. At most 2048 single-depth layers,
// all matching the first layer's size, format, and mip count.
bool RndTextureArray2D::ValidateElements() const {
    if (mPixels.empty() || mPixels.mpBegin == nullptr) {
        return false;
    }
    if (mPixels.size() > 2048) {
        return false;
    }
    const auto& first = mPixels.front();
    if (first.mSize.z != 1) {
        return false;
    }
    const auto numMips = first.GetNumMips();
    for (const auto& pixels : mPixels) {
        if (pixels.mSize.x != first.mSize.x || pixels.mSize.y != first.mSize.y ||
            pixels.mSize.z != first.mSize.z || pixels.mFormat != first.mFormat ||
            pixels.GetNumMips() != numMips) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x6985C0.
void RndTextureArray2D::_ValidateGpuCopyFrom(const RndShaderResource& source) {
    _ValidateGpuCopyFromBase(source);
}
