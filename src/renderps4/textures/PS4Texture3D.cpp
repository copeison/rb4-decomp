#include "renderps4/textures/PS4Texture3D.h"

#include <cstdint>
#include <gnm/platform.h>
#include <gpu_address.h>

#include "renderps4/system/PS4Device.h"
#include "renderps4/system/PS4RenderUtl.h"

// Reconstructed from eboot.elf at 0x8E53C0.
PS4Texture3D::PS4Texture3D(const Description& desc)
    : RndTexture3D(desc),
      mGpuTexture(nullptr), mStorage(nullptr) {}

// Reconstructed from eboot.elf at 0x8E53F0. The deleting destructor at
// 0x8E5450 releases the texture through MemFree.
PS4Texture3D::~PS4Texture3D() {
    gPS4Device->DeferredDelete(mStorage);
    operator delete(mGpuTexture);
    mGpuTexture = nullptr;
}

// Reconstructed from eboot.elf at 0x8E5770.
void PS4Texture3D::_SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int) {
    PS4RenderUtl::SelectTextureForVS(
        context, slot, mGpuTexture, WrapMode(), FilterMode());
}

// Reconstructed from eboot.elf at 0x8E5790.
void PS4Texture3D::_SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForHS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E57B0.
void PS4Texture3D::_SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForDS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E57D0.
void PS4Texture3D::_SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForGS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E57F0.
void PS4Texture3D::_SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForPS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E5810.
void PS4Texture3D::_SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long) {
    PS4RenderUtl::SelectTextureForCS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E5740.
void PS4Texture3D::_SyncDynamicImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8E5750.
void PS4Texture3D::_SyncFromGpuImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8E5760.
void PS4Texture3D::_Slot20Impl() {}

// Reconstructed from eboot.elf at 0x8E54B0. Like the 1D texture, but a
// texture to reuse lends its GPU storage, and render targets use GPU-coherent
// memory.
void PS4Texture3D::_SyncStaticImpl(const RndTextureBase* reuse) {
    const int dataFormat = mPixels.mFormat;
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
    spec.m_textureType = sce::Gnm::kTextureType3d;
    spec.m_width = mBaseDesc.mWidth;
    spec.m_height = mBaseDesc.mHeight;
    spec.m_depth = mBaseDesc.mDepth;
    spec.m_pitch = 0;
    spec.m_numSlices = 1;
    spec.m_format = PS4RenderUtl::GetDataFormat(dataFormat);
    spec.m_tileModeHint = tileMode;
    spec.m_numFragments = sce::Gnm::kNumFragments1;
    spec.m_numMipLevels = static_cast<std::uint32_t>(mPixels.GetNumMips() + 1);
    spec.m_minGpuMode = gpuMode;
    mGpuTexture = new sce::Gnm::Texture();
    mGpuTexture->init(&spec);

    const auto sizeAlign = mGpuTexture->getSizeAlign();
    if (reuse != nullptr) {
        mStorage = static_cast<const PS4Texture3D*>(reuse)->mStorage;
    } else {
        static long sGpuHeap = MemFindHeap("gpu");
        MemPushHeap(sGpuHeap);
        mStorage = MemAlloc(sizeAlign.m_size, mBaseDesc.mName, static_cast<int>(sizeAlign.m_align));
        MemPopHeap();
    }

    if (mPixels.mBuffer != nullptr) {
        const RndPixelData* level = &mPixels;
        for (unsigned long mip = 0; mip < mPixels.GetNumMips() + 1; ++mip) {
            sce::GpuAddress::TilingParameters tiling;
            tiling.initFromTexture(mGpuTexture, static_cast<std::uint32_t>(mip), 0);
            std::uint64_t offset;
            std::uint64_t size;
            sce::GpuAddress::computeTextureSurfaceOffsetAndSize(
                &offset, &size, mGpuTexture, static_cast<std::uint32_t>(mip), 0);
            sce::GpuAddress::tileSurface(
                static_cast<unsigned char*>(mStorage) + offset, level->mBuffer, &tiling);
            level = level->mMip;
        }
    }
    mGpuTexture->setBaseAddress(mStorage);
    mGpuTexture->setResourceMemoryType(
        (mBaseDesc.mFormat.mFlags & kPixelFormatRenderTarget) != 0
            ? sce::Gnm::kResourceMemoryTypeGC
            : sce::Gnm::kResourceMemoryTypeRO);
}

// Reconstructed from eboot.elf at 0x8E5830. Only validates.
void PS4Texture3D::_GpuCopyFromImpl(RndContext&, RndShaderResource& source) {
    _ValidateGpuCopyFrom(source);
}
