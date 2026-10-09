#include "render/textures/RndTextureArrayCube.h"

#include "render/system/RndFactory.h"

// Reconstructed from eboot.elf at 0x69AF00.
RndTextureArrayCube::Description::Description() {
    mType = kTextureArrayCube;
}

// Reconstructed from eboot.elf at 0x69AA60.
RndTextureArrayCube* RndTextureArrayCube::New(Description& desc, const RndTextureArrayCube* reuse) {
    desc.ResolveFormat(kTextureArrayCube, -1);
    auto* texture = TheRndFactory()->CreateTextureArrayCube(desc);
    texture->SyncStatic(reuse);
    return texture;
}

// Reconstructed from eboot.elf at 0x69AAC0. The base fields are copied
// back into the base description after the size and format of the first
// mip level are recorded.
RndTextureArrayCube::RndTextureArrayCube(const Description& desc) {
    InitDescription(desc, desc.ShouldKeepPixelData());
    ValidateElements();
    mResourceIndex = 2;
    const auto& first = mCubes.front().mFaces[0];
    mDesc.mDataFormat = first.mFormat;
    mDesc.mWidth = static_cast<unsigned int>(first.mSize.x);
    mDesc.mHeight = static_cast<unsigned int>(first.mSize.y);
    mDesc.mDepth = static_cast<unsigned int>(first.mSize.z);
    mDesc.mArraySize = mCubes.size();
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x69AD60.
unsigned long RndTextureArrayCube::_GetNumMipsImpl() const {
    return mCubes.front().mFaces[0].GetNumMips();
}

// Reconstructed from eboot.elf at 0x69AD70.
unsigned long RndTextureArrayCube::_GetTotalBytesImpl() const {
    return mCubes.empty() ? 0 : mCubes.size() * mCubes.front().GetTotalBytes();
}

// Reconstructed from eboot.elf at 0x69ADC0.
void RndTextureArrayCube::_SetRequestedFormatImpl(const RndPixelFormat& format) {
    mDesc.mRequestedFormat = format;
    SetBaseDesc(mDesc);
}

// Reconstructed from eboot.elf at 0x69AE20.
void RndTextureArrayCube::_FreePixelDataImpl() {
    for (auto& cube : mCubes) {
        cube.FreeBuffers();
    }
}

void RndTextureArrayCube::InitDescription(const Description& desc, bool keepPixels) {
    mDesc = desc;
    mCubes.reserve(desc.mCubes.size());
    for (const RndPixelDataCube& cube : desc.mCubes) {
        mCubes.emplace_back(cube, keepPixels);
    }
}

// Reconstructed from eboot.elf at 0x69ABB0. At most 341 cubes, each valid and
// matching the first cube's size, format, and mip count.
bool RndTextureArrayCube::ValidateElements() const {
    if (mCubes.empty() || mCubes.size() > 341 || !mCubes.front().IsValid()) {
        return false;
    }
    const auto& first = mCubes.front().mFaces[0];
    const auto numMips = first.GetNumMips();
    for (const auto& cube : mCubes) {
        const auto& face = cube.mFaces[0];
        if (face.mSize.x != first.mSize.x || face.mSize.y != first.mSize.y ||
            face.mSize.z != first.mSize.z || face.mFormat != first.mFormat ||
            face.GetNumMips() != numMips) {
            return false;
        }
    }
    return true;
}

// Reconstructed from eboot.elf at 0x69AD50.
void RndTextureArrayCube::_ValidateGpuCopyFrom(const RndShaderResource& source) {
    _ValidateGpuCopyFromBase(source);
}
