#include "renderps4/textures/PS4TextureArrayCube.h"

#include <cstdint>
#include <gnm/platform.h>
#include <gpu_address.h>

#include "renderps4/system/PS4Device.h"
#include "renderps4/system/PS4RenderUtl.h"

// Reconstructed from eboot.elf at 0x8E6640.
PS4TextureArrayCube::PS4TextureArrayCube(const Description& desc)
    : RndTextureArrayCube(desc),
      mGpuTexture(nullptr), mStorage(nullptr) {}

// Reconstructed from eboot.elf at 0x8E6670. The deleting destructor at
// 0x8E66D0 releases the texture through MemFree.
PS4TextureArrayCube::~PS4TextureArrayCube() {
    gPS4Device->DeferredDelete(mStorage);
    operator delete(mGpuTexture);
    mGpuTexture = nullptr;
}

// Reconstructed from eboot.elf at 0x8E6AB0.
void PS4TextureArrayCube::_SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int) {
    PS4RenderUtl::SelectTextureForVS(
        context, slot, mGpuTexture, WrapMode(), FilterMode());
}

// Reconstructed from eboot.elf at 0x8E6AD0.
void PS4TextureArrayCube::_SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForHS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6AF0.
void PS4TextureArrayCube::_SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForDS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6B10.
void PS4TextureArrayCube::_SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForGS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6B30.
void PS4TextureArrayCube::_SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForPS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6B50.
void PS4TextureArrayCube::_SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long) {
    PS4RenderUtl::SelectTextureForCS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6A90.
void PS4TextureArrayCube::_SyncFromGpuImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8E6AA0.
void PS4TextureArrayCube::_Slot20Impl() {}

// Reconstructed from eboot.elf at 0x8E6730. Each cube occupies six slices,
// one per face; the first face of the first cube sets the format and mip
// count.
void PS4TextureArrayCube::_SyncStaticImpl(const RndTextureBase*) {
    const RndPixelData& first = mCubes.front().mFaces[0];
    const int dataFormat = first.mFormat;
    const auto gpuMode = sce::Gnm::getGpuMode();
    sce::Gnm::TileMode tileMode;
    sce::GpuAddress::computeSurfaceTileMode(
        gpuMode,
        &tileMode,
        sce::GpuAddress::kSurfaceTypeTextureCubemap,
        PS4RenderUtl::GetDataFormat(dataFormat),
        1);

    sce::Gnm::TextureSpec spec;
    spec.init();
    spec.m_textureType = sce::Gnm::kTextureTypeCubemap;
    spec.m_width = mBaseDesc.mWidth;
    spec.m_height = mBaseDesc.mHeight;
    spec.m_depth = 1;
    spec.m_pitch = 0;
    spec.m_numSlices = static_cast<std::uint32_t>(mCubes.size());
    spec.m_format = PS4RenderUtl::GetDataFormat(dataFormat);
    spec.m_tileModeHint = tileMode;
    spec.m_numFragments = sce::Gnm::kNumFragments1;
    spec.m_numMipLevels = static_cast<std::uint32_t>(first.GetNumMips() + 1);
    spec.m_minGpuMode = gpuMode;
    mGpuTexture = new sce::Gnm::Texture();
    mGpuTexture->init(&spec);

    const auto sizeAlign = mGpuTexture->getSizeAlign();
    static long sGpuHeap = MemFindHeap("gpu");
    MemPushHeap(sGpuHeap);
    mStorage = MemAlloc(sizeAlign.m_size, mBaseDesc.mName, static_cast<int>(sizeAlign.m_align));
    MemPopHeap();

    if (first.mBuffer != nullptr) {
        for (unsigned long cube = 0; cube < mCubes.size(); ++cube) {
            for (int face = 0; face < RndPixelDataCube::kNumFaces; ++face) {
                const auto slice =
                    static_cast<std::uint32_t>(cube * RndPixelDataCube::kNumFaces + face);
                const RndPixelData* level = &mCubes[cube].mFaces[face];
                for (unsigned long mip = 0; mip < first.GetNumMips() + 1; ++mip) {
                    sce::GpuAddress::TilingParameters tiling;
                    tiling.initFromTexture(mGpuTexture, static_cast<std::uint32_t>(mip), slice);
                    std::uint64_t offset;
                    std::uint64_t size;
                    sce::GpuAddress::computeTextureSurfaceOffsetAndSize(
                        &offset, &size, mGpuTexture, static_cast<std::uint32_t>(mip), slice);
                    sce::GpuAddress::tileSurface(
                        static_cast<unsigned char*>(mStorage) + offset, level->mBuffer, &tiling);
                    level = level->mMip;
                }
            }
        }
    }
    mGpuTexture->setBaseAddress(mStorage);
    mGpuTexture->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeRO);
}

// Reconstructed from eboot.elf at 0x8E6B70. Only validates.
void PS4TextureArrayCube::_GpuCopyFromImpl(RndContext&, RndShaderResource& source) {
    _ValidateGpuCopyFrom(source);
}
