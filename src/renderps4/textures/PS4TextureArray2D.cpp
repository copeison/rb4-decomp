#include "renderps4/textures/PS4TextureArray2D.h"

#include <cstdint>
#include <gnm/dataformats.h>
#include <gnm/platform.h>
#include <gpu_address.h>

#include "renderps4/system/PS4Device.h"
#include "renderps4/system/PS4RenderUtl.h"

// Reconstructed from eboot.elf at 0x8E5D40.
PS4TextureArray2D::PS4TextureArray2D(const Description& desc)
    : RndTextureArray2D(desc),
      mGpuTexture(nullptr),
      mStorage(nullptr),
      mStencilStorage(nullptr),
      mHtileStorage(nullptr),
      mColorTarget(nullptr),
      mDepthTarget(nullptr) {}

// Reconstructed from eboot.elf at 0x8E5D80. The deleting destructor at
// 0x8E5E50 releases the texture through MemFree.
PS4TextureArray2D::~PS4TextureArray2D() {
    gPS4Device->DeferredDelete(mStorage);
    gPS4Device->DeferredDelete(mStencilStorage);
    gPS4Device->DeferredDelete(mHtileStorage);
    if (mColorTarget != nullptr) {
        gPS4Device->DeferredDelete(mColorTarget->getCmaskAddress());
        delete mColorTarget;
        mColorTarget = nullptr;
    }
    delete mGpuTexture;
    mGpuTexture = nullptr;
    delete mDepthTarget;
    mDepthTarget = nullptr;
}

// Reconstructed from eboot.elf at 0x8E64C0.
void PS4TextureArray2D::_SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int) {
    PS4RenderUtl::SelectTextureForVS(
        context, slot, mGpuTexture, WrapMode(), FilterMode());
}

// Reconstructed from eboot.elf at 0x8E64E0.
void PS4TextureArray2D::_SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForHS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6500.
void PS4TextureArray2D::_SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForDS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6520.
void PS4TextureArray2D::_SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForGS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6540.
void PS4TextureArray2D::_SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForPS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6560.
void PS4TextureArray2D::_SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long) {
    PS4RenderUtl::SelectTextureForCS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E64A0.
void PS4TextureArray2D::_SyncFromGpuImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8E64B0.
void PS4TextureArray2D::_Slot20Impl() {}

// Reconstructed from eboot.elf at 0x8E5E70.
void PS4TextureArray2D::_SyncStaticImpl(const RndTextureBase*) {
    if (mBaseDesc.mFormat.mUsage == kTextureUsageDepth) {
        _SyncDepthStencil();
    } else {
        _SyncRegular();
    }
}

// Inlined into _SyncStaticImpl at 0x8E5E70. Each array element is one slice
// of an HTILE-accelerated depth target, viewed through a GPU-coherent
// texture.
void PS4TextureArray2D::_SyncDepthStencil() {
    const int dataFormat = mPixels.front().mFormat;
    const auto gpuMode = sce::Gnm::getGpuMode();
    const auto zFormat = PS4RenderUtl::GetZFormat(dataFormat);
    const auto stencilFormat = PS4RenderUtl::GetStencilFormat(dataFormat);
    sce::Gnm::TileMode tileMode;
    sce::GpuAddress::computeSurfaceTileMode(
        gpuMode,
        &tileMode,
        stencilFormat == sce::Gnm::kStencilInvalid
            ? sce::GpuAddress::kSurfaceTypeDepthOnlyTarget
            : sce::GpuAddress::kSurfaceTypeDepthTarget,
        sce::Gnm::DataFormat::build(zFormat),
        1);

    sce::Gnm::DepthRenderTargetSpec spec;
    spec.init();
    spec.m_width = mBaseDesc.mWidth;
    spec.m_height = mBaseDesc.mHeight;
    spec.m_pitch = 0;
    spec.m_numSlices = static_cast<std::uint32_t>(mPixels.size());
    spec.m_zFormat = zFormat;
    spec.m_stencilFormat = stencilFormat;
    spec.m_tileModeHint = tileMode;
    spec.m_numFragments = sce::Gnm::kNumFragments1;
    spec.m_flags.enableHtileAcceleration = 1;
    spec.m_minGpuMode = gpuMode;
    mDepthTarget = new sce::Gnm::DepthRenderTarget();
    mDepthTarget->init(&spec);

    const auto zSizeAlign = mDepthTarget->getZSizeAlign();
    static long sGpuHeap = MemFindHeap("gpu");
    MemPushHeap(sGpuHeap);
    mStorage = MemAlloc(zSizeAlign.m_size, mBaseDesc.mName, static_cast<int>(zSizeAlign.m_align));
    if (stencilFormat != sce::Gnm::kStencilInvalid) {
        const auto stencilSizeAlign = mDepthTarget->getStencilSizeAlign();
        mStencilStorage = MemAlloc(
            stencilSizeAlign.m_size, mBaseDesc.mName, static_cast<int>(stencilSizeAlign.m_align));
    }
    const auto htileSizeAlign = mDepthTarget->getHtileSizeAlign();
    mHtileStorage = MemAlloc(
        htileSizeAlign.m_size, mBaseDesc.mName, static_cast<int>(htileSizeAlign.m_align));
    MemPopHeap();

    mDepthTarget->setZReadAddress(mStorage);
    mDepthTarget->setZWriteAddress(mStorage);
    if (stencilFormat != sce::Gnm::kStencilInvalid) {
        mDepthTarget->setStencilReadAddress(mStencilStorage);
        mDepthTarget->setStencilWriteAddress(mStencilStorage);
    }
    mDepthTarget->setHtileAddress(mHtileStorage);
    mDepthTarget->setHtileAccelerationEnable(true);
    mDepthTarget->setZCompareBase(sce::Gnm::kZCompareBaseZMin);

    mGpuTexture = new sce::Gnm::Texture();
    mGpuTexture->initFromDepthRenderTarget(mDepthTarget, false);
    mGpuTexture->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeGC);
}

// Inlined into _SyncStaticImpl at 0x8E5E70. Each array element is one slice;
// the first element sets the mip count. A render-target array also gets a
// render target over the texture.
void PS4TextureArray2D::_SyncRegular() {
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
    spec.m_textureType = sce::Gnm::kTextureType2dArray;
    spec.m_width = mBaseDesc.mWidth;
    spec.m_height = mBaseDesc.mHeight;
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

    if ((mBaseDesc.mFormat.mFlags & kPixelFormatRenderTarget) != 0) {
        mColorTarget = new sce::Gnm::RenderTarget();
        mColorTarget->initFromTexture(
            mGpuTexture,
            0,
            static_cast<sce::Gnm::NumSamples>(mGpuTexture->getNumFragments()),
            nullptr,
            nullptr);
        mGpuTexture->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeGC);
    } else {
        mGpuTexture->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeRO);
    }
}

// Reconstructed from eboot.elf at 0x8E6580. Only validates.
void PS4TextureArray2D::_GpuCopyFromImpl(RndContext&, RndShaderResource& source) {
    _ValidateGpuCopyFrom(source);
}
