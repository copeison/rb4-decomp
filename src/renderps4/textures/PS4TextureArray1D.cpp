#include "renderps4/textures/PS4TextureArray1D.h"

#include <cstdint>
#include <gnm/platform.h>
#include <gpu_address.h>

#include "renderps4/system/PS4Device.h"
#include "renderps4/system/PS4RenderUtl.h"

// Reconstructed from eboot.elf at 0x8E5870.
PS4TextureArray1D::PS4TextureArray1D(const Description& desc)
    : RndTextureArray1D(desc),
      mGpuTexture(nullptr), mStorage(nullptr) {}

// Reconstructed from eboot.elf at 0x8E58A0. The deleting destructor at
// 0x8E5900 releases the texture through MemFree.
PS4TextureArray1D::~PS4TextureArray1D() {
    gPS4Device->DeferredDelete(mStorage);
    operator delete(mGpuTexture);
    mGpuTexture = nullptr;
}

// Reconstructed from eboot.elf at 0x8E5C50.
void PS4TextureArray1D::_SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int) {
    PS4RenderUtl::SelectTextureForVS(
        context, slot, mGpuTexture, WrapMode(), FilterMode());
}

// Reconstructed from eboot.elf at 0x8E5C70.
void PS4TextureArray1D::_SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForHS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E5C90.
void PS4TextureArray1D::_SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForDS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E5CB0.
void PS4TextureArray1D::_SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForGS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E5CD0.
void PS4TextureArray1D::_SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForPS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E5CF0.
void PS4TextureArray1D::_SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long) {
    PS4RenderUtl::SelectTextureForCS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E5C20.
void PS4TextureArray1D::_SyncDynamicImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8E5C30.
void PS4TextureArray1D::_SyncFromGpuImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8E5C40.
void PS4TextureArray1D::_Slot20Impl() {}

// Reconstructed from eboot.elf at 0x8E5960. Each array element is one slice;
// the first element sets the format and mip count.
void PS4TextureArray1D::_SyncStaticImpl(const RndTextureBase*) {
    const RndPixelData& first = mPixels.front();
    const int dataFormat = first.mFormat;
    const auto gpuMode = sce::Gnm::getGpuMode();
    sce::Gnm::TileMode tileMode;
    sce::GpuAddress::computeSurfaceTileMode(
        gpuMode,
        &tileMode,
        PS4RenderUtl::GetSurfaceType(mBaseDesc.mFormat),
        PS4RenderUtl::GetDataFormat(dataFormat),
        1);

    sce::Gnm::TextureSpec spec;
    spec.init();
    spec.m_textureType = sce::Gnm::kTextureType1dArray;
    spec.m_width = mBaseDesc.mWidth;
    spec.m_height = 1;
    spec.m_depth = 1;
    spec.m_pitch = 0;
    spec.m_numSlices = static_cast<std::uint32_t>(mPixels.size());
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
        for (unsigned long slice = 0; slice < mPixels.size(); ++slice) {
            const RndPixelData* level = &mPixels[slice];
            for (unsigned long mip = 0; mip < first.GetNumMips() + 1; ++mip) {
                sce::GpuAddress::TilingParameters tiling;
                tiling.initFromTexture(
                    mGpuTexture,
                    static_cast<std::uint32_t>(mip),
                    static_cast<std::uint32_t>(slice));
                std::uint64_t offset;
                std::uint64_t size;
                sce::GpuAddress::computeTextureSurfaceOffsetAndSize(
                    &offset,
                    &size,
                    mGpuTexture,
                    static_cast<std::uint32_t>(mip),
                    static_cast<std::uint32_t>(slice));
                sce::GpuAddress::tileSurface(
                    static_cast<unsigned char*>(mStorage) + offset, level->mBuffer, &tiling);
                level = level->mMip;
            }
        }
    }
    mGpuTexture->setBaseAddress(mStorage);
    mGpuTexture->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeRO);
}

// Reconstructed from eboot.elf at 0x8E5D10. Only validates.
void PS4TextureArray1D::_GpuCopyFromImpl(RndContext&, RndShaderResource& source) {
    _ValidateGpuCopyFrom(source);
}
