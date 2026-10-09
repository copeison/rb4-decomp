#include "render/textures/RndTextureArray1D.h"

#include "render/system/RndFactory.h"

// Reconstructed from eboot.elf at 0x6972D0.
RndTextureArray1D::Description::Description() {
    mType = kTextureArray1D;
}

// Reconstructed from eboot.elf at 0x696BA0.
RndTextureArray1D* RndTextureArray1D::New(Description& desc, const RndTextureArray1D* reuse) {
    desc.ResolveFormat(kTextureArray1D, -1);
    auto* texture = TheRndFactory()->CreateTextureArray1D(desc);
    texture->SyncStatic(reuse);
    return texture;
}

// Reconstructed from eboot.elf at 0x696C00. The base fields are copied
// back into the base description after the size and format of the first
// mip level are recorded.
RndTextureArray1D::RndTextureArray1D(const Description& desc)
    : mDesc(desc) {
    const auto count =
        static_cast<unsigned long>(desc.mPixels.mEnd - desc.mPixels.mBegin);
    mPixels.reserve(count);
    for (auto* pixels = desc.mPixels.mBegin; pixels != desc.mPixels.mEnd; ++pixels) {
        mPixels.emplace_back(*pixels, desc.ShouldKeepPixelData());
    }
    ValidateElements();
    const auto& first = mPixels.front();
    mDesc.mDataFormat = first.mFormat;
    mDesc.mWidth = static_cast<unsigned int>(first.mSize.x);
    mDesc.mHeight = static_cast<unsigned int>(first.mSize.y);
    mDesc.mDepth = static_cast<unsigned int>(first.mSize.z);
    mDesc.mArraySize = mPixels.size();
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x697120.
unsigned long RndTextureArray1D::_GetNumMipsImpl() const {
    return mPixels.front().GetNumMips();
}

// Reconstructed from eboot.elf at 0x697130.
unsigned long RndTextureArray1D::_GetTotalBytesImpl() const {
    return mPixels.empty() ? 0 : mPixels.size() * mPixels.front().GetTotalBytes();
}

// Reconstructed from eboot.elf at 0x697180.
void RndTextureArray1D::_SetRequestedFormatImpl(const RndPixelFormat& format) {
    mDesc.mRequestedFormat = format;
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x6971E0.
void RndTextureArray1D::_FreePixelDataImpl() {
    for (auto& pixels : mPixels) {
        pixels.FreeBuffers();
    }
}

// Reconstructed from the validation tail at 0x696D8E. Every layer must be one
// texel high and deep and match the first layer's width, format, and mip
// count; the binary discards the result.
bool RndTextureArray1D::ValidateElements() const {
    if (mPixels.empty()) {
        return true;
    }
    const auto& first = mPixels.front();
    if (first.mSize.y != 1 || first.mSize.z != 1) {
        return false;
    }
    const auto numMips = first.GetNumMips();
    for (const auto& pixels : mPixels) {
        if (pixels.mSize.y != 1 || pixels.mSize.z != 1 ||
            pixels.mSize.x != first.mSize.x || pixels.mFormat != first.mFormat ||
            pixels.GetNumMips() != numMips) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x6972C0.
void RndTextureArray1D::_ValidateGpuCopyFrom(const RndShaderResource& source) {
    _ValidateGpuCopyFromBase(source);
}
