#include "renderps4/textures/PS4TextureCube.h"

#include <cstdint>
#include <gnm/dataformats.h>
#include <gnm/platform.h>
#include <gpu_address.h>

#include "renderps4/system/PS4Device.h"
#include "renderps4/system/PS4RenderUtl.h"

// Reconstructed from eboot.elf at 0x8E6BA0.
PS4TextureCube::PS4TextureCube(const Description& desc)
    : RndTextureCube(desc),
      mGpuTexture(nullptr),
      mStorage(nullptr),
      mStencilStorage(nullptr),
      mRenderTarget(nullptr),
      mDepthTarget(nullptr) {}

// Reconstructed from eboot.elf at 0x8E6BE0. The deleting destructor at
// 0x8E6CC0 releases the texture through MemFree.
PS4TextureCube::~PS4TextureCube() {
    if (mRenderTarget != nullptr) {
        gPS4Device->DeferredDelete(mRenderTarget->getCmaskAddress());
        gPS4Device->DeferredDelete(mRenderTarget->getBaseAddress());
        delete mRenderTarget;
        mRenderTarget = nullptr;
    }
    gPS4Device->DeferredDelete(mStorage);
    gPS4Device->DeferredDelete(mStencilStorage);
    delete mGpuTexture;
    mGpuTexture = nullptr;
    delete mDepthTarget;
    mDepthTarget = nullptr;
}

// Reconstructed from eboot.elf at 0x8E71A0.
void PS4TextureCube::_SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int) {
    PS4RenderUtl::SelectTextureForVS(
        context, slot, mGpuTexture, WrapMode(), FilterMode());
}

// Reconstructed from eboot.elf at 0x8E71C0.
void PS4TextureCube::_SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForHS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E71E0.
void PS4TextureCube::_SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForDS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E7200.
void PS4TextureCube::_SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForGS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E7220.
void PS4TextureCube::_SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForPS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E7240.
void PS4TextureCube::_SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long) {
    PS4RenderUtl::SelectTextureForCS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E7180.
void PS4TextureCube::_SyncFromGpuImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8E7190.
void PS4TextureCube::_Slot20Impl() {}

// Reconstructed from eboot.elf at 0x8E6CE0.
void PS4TextureCube::_SyncStaticImpl(const RndTextureBase*) {
    if (mBaseDesc.mFormat.mUsage == kTextureUsageDepth) {
        _SyncDepthStencil();
    } else {
        _SyncRegular();
    }
}

// Inlined into _SyncStaticImpl at 0x8E6CE0. A depth cube is a six-slice depth
// target; the texture views it as a cube map in GPU-coherent memory.
void PS4TextureCube::_SyncDepthStencil() {
    const int dataFormat = mCube.mFaces[0].mFormat;
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
    spec.m_numSlices = RndPixelDataCube::kNumFaces;
    spec.m_zFormat = zFormat;
    spec.m_stencilFormat = stencilFormat;
    spec.m_tileModeHint = tileMode;
    spec.m_numFragments = sce::Gnm::kNumFragments1;
    spec.m_minGpuMode = gpuMode;
    mDepthTarget = new sce::Gnm::DepthRenderTarget();
    mDepthTarget->init(&spec);

    const auto zSizeAlign = mDepthTarget->getZSizeAlign();
    const auto stencilSizeAlign = mDepthTarget->getStencilSizeAlign();
    static long sGpuHeap = MemFindHeap("gpu");
    MemPushHeap(sGpuHeap);
    mStorage = MemAlloc(zSizeAlign.m_size, mBaseDesc.mName, static_cast<int>(zSizeAlign.m_align));
    mStencilStorage = MemAlloc(
        stencilSizeAlign.m_size, mBaseDesc.mName, static_cast<int>(stencilSizeAlign.m_align));
    MemPopHeap();

    mGpuTexture = new sce::Gnm::Texture();
    mGpuTexture->initFromDepthRenderTarget(mDepthTarget, true);
    mGpuTexture->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeGC);
}

// Inlined into _SyncStaticImpl at 0x8E6CE0. Each face is one slice; the first
// face sets the mip count. A render-target cube also gets a render target
// over the texture.
void PS4TextureCube::_SyncRegular() {
    const int dataFormat = mCube.mFaces[0].mFormat;
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
    spec.m_textureType = sce::Gnm::kTextureTypeCubemap;
    spec.m_width = mBaseDesc.mWidth;
    spec.m_height = mBaseDesc.mHeight;
    spec.m_depth = 1;
    spec.m_pitch = 0;
    spec.m_numSlices = 1;
    spec.m_format = PS4RenderUtl::GetDataFormat(dataFormat);
    spec.m_tileModeHint = tileMode;
    spec.m_numFragments = sce::Gnm::kNumFragments1;
    spec.m_numMipLevels = static_cast<std::uint32_t>(mCube.mFaces[0].GetNumMips() + 1);
    spec.m_minGpuMode = gpuMode;
    mGpuTexture = new sce::Gnm::Texture();
    mGpuTexture->init(&spec);

    const auto sizeAlign = mGpuTexture->getSizeAlign();
    static long sGpuHeap = MemFindHeap("gpu");
    MemPushHeap(sGpuHeap);
    mStorage = MemAlloc(sizeAlign.m_size, mBaseDesc.mName, static_cast<int>(sizeAlign.m_align));
    MemPopHeap();

    if (mCube.mFaces[0].mBuffer != nullptr) {
        for (int face = 0; face < RndPixelDataCube::kNumFaces; ++face) {
            const RndPixelData* level = &mCube.mFaces[face];
            for (unsigned long mip = 0; mip < mCube.mFaces[0].GetNumMips() + 1; ++mip) {
                sce::GpuAddress::TilingParameters tiling;
                tiling.initFromTexture(
                    mGpuTexture,
                    static_cast<std::uint32_t>(mip),
                    static_cast<std::uint32_t>(face));
                std::uint64_t offset;
                std::uint64_t size;
                sce::GpuAddress::computeTextureSurfaceOffsetAndSize(
                    &offset,
                    &size,
                    mGpuTexture,
                    static_cast<std::uint32_t>(mip),
                    static_cast<std::uint32_t>(face));
                sce::GpuAddress::tileSurface(
                    static_cast<unsigned char*>(mStorage) + offset, level->mBuffer, &tiling);
                level = level->mMip;
            }
        }
    }
    mGpuTexture->setBaseAddress(mStorage);

    if ((mBaseDesc.mFormat.mFlags & kPixelFormatRenderTarget) != 0) {
        mRenderTarget = new sce::Gnm::RenderTarget();
        mRenderTarget->initFromTexture(
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

// Reconstructed from eboot.elf at 0x8E7260. Only validates.
void PS4TextureCube::_GpuCopyFromImpl(RndContext&, RndShaderResource& source) {
    _ValidateGpuCopyFrom(source);
}
