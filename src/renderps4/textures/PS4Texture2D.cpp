#include "renderps4/textures/PS4Texture2D.h"

#include <cstdint>
#include <gnm/dataformats.h>
#include <gnm/platform.h>
#include <gpu_address.h>

#include "render/buffers/RndCShaderCopyBuffer.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "renderps4/system/PS4Device.h"
#include "renderps4/system/PS4RenderUtl.h"
#include "renderps4/video/PS4Window.h"

namespace {

// RndPixelFormat::mFlags bits the 2D texture reads. Names not in the
// reference map.
constexpr unsigned int kPixelFormatDynamic = 0x1;
constexpr unsigned int kPixelFormatGpuWritable = 0x10;

// The back buffers' engine data format, BGRA_UNorm8_sRGB, and count. Names
// not in the reference map.
constexpr int kBackBufferDataFormat = 12;
constexpr unsigned long kNumBackBuffers = 2;

unsigned char* AlignUp(void* address, unsigned int alignment) {
    const auto value = reinterpret_cast<std::uintptr_t>(address);
    return reinterpret_cast<unsigned char*>(
        (value + alignment - 1) & ~(static_cast<std::uintptr_t>(alignment) - 1));
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D62C0.
PS4Texture2D::PS4Texture2D(const Description& desc)
    : RndTexture2D(desc),
      mGpuTextures{nullptr, nullptr},
      mPlaneTexture(nullptr),
      mSizeAlign{},
      mStencilSizeAlign{},
      mHtileSizeAlign{},
      mActiveStorage(0),
      mRenderTargets{nullptr, nullptr},
      mDepthTarget(nullptr),
      mPendingPresentations(nullptr) {}

// Reconstructed from eboot.elf at 0x8D5E30. The storage takes over the back
// buffers' memory, and each buffer's texture views its render target.
PS4Texture2D* PS4Texture2D::CreateAsBackBuffer(sce::Gnm::RenderTarget** targets) {
    Description desc;
    desc.mName = "BackBuffer";
    desc.mRequestedFormat.mWrapMode = 1;
    desc.mRequestedFormat.mFilterMode = 1;
    desc.mRequestedFormat.mFlags = kPixelFormatRenderTarget;
    desc.mPixels.CreateEmpty(
        static_cast<int>(targets[0]->getWidth()),
        static_cast<int>(targets[0]->getHeight()),
        1,
        kBackBufferDataFormat);
    auto* texture = new PS4Texture2D(desc);
    desc.mPixels.Free();

    texture->mStorage = std::make_shared<Storage>();
    for (unsigned long buffer = 0; buffer < kNumBackBuffers; ++buffer) {
        auto* target = targets[buffer];
        texture->mRenderTargets[buffer] = target;
        texture->mGpuTextures[buffer] = new sce::Gnm::Texture();
        texture->mGpuTextures[buffer]->initFromRenderTarget(target, false);
        texture->mGpuTextures[buffer]->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeGC);
        texture->mStorage->mSurfaces[buffer] = target->getBaseAddress();
        texture->mStorage->mSurfaceSizes[buffer] = target->getColorSizeAlign().m_size;
    }
    texture->mPendingPresentations = new int[kNumBackBuffers]();
    return texture;
}

// Reconstructed from eboot.elf at 0x8D6310. The deleting destructor at
// 0x8D6440 releases the texture through MemFree.
PS4Texture2D::~PS4Texture2D() {
    for (auto*& target : mRenderTargets) {
        delete target;
        target = nullptr;
    }
    for (auto*& texture : mGpuTextures) {
        delete texture;
        texture = nullptr;
    }
    delete mPlaneTexture;
    mPlaneTexture = nullptr;
    delete mDepthTarget;
    mDepthTarget = nullptr;
    delete[] mPendingPresentations;
}

// Reconstructed from eboot.elf at 0x8D7260. The surfaces outlive the device
// only at shutdown, when they are freed directly.
PS4Texture2D::Storage::~Storage() {
    if (gPS4Device != nullptr) {
        gPS4Device->DeferredDelete(mSurfaces[0]);
        gPS4Device->DeferredDelete(mSurfaces[1]);
        gPS4Device->DeferredDelete(mStencil);
        gPS4Device->DeferredDelete(mHtile);
    } else {
        MemFree(mSurfaces[0]);
        MemFree(mSurfaces[1]);
        MemFree(mStencil);
        MemFree(mHtile);
    }
}

// Inlined into every stage select (0x8D6E40-0x8D713F). The stencil-plane
// view is chosen by flag; a render target follows the main window's
// presented buffer when it has a target for it, and other textures follow
// their active storage.
sce::Gnm::Texture* PS4Texture2D::SelectView(unsigned int flags) const {
    if ((flags & PS4RenderUtl::kSelectStencilPlane) != 0) {
        return mPlaneTexture;
    }
    unsigned long index;
    if (mRenderTargets[0] != nullptr) {
        index = static_cast<PS4Window*>(gPS4Device->mMainWindow)->mActiveBuffer;
        if (mRenderTargets[index] == nullptr) {
            index = 0;
        }
    } else {
        index = mActiveStorage;
    }
    return mGpuTextures[index];
}

// Reconstructed from eboot.elf at 0x8D6E40.
void PS4Texture2D::_SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForVS(
        context, slot, SelectView(flags), WrapMode(), FilterMode());
}

// Reconstructed from eboot.elf at 0x8D6F10.
void PS4Texture2D::_SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForHS(
        context, slot, SelectView(flags), WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8D6F80.
void PS4Texture2D::_SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForDS(
        context, slot, SelectView(flags), WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8D6FF0.
void PS4Texture2D::_SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForGS(
        context, slot, SelectView(flags), WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8D7060.
void PS4Texture2D::_SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForPS(
        context, slot, SelectView(flags), WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8D70D0.
void PS4Texture2D::_SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long) {
    PS4RenderUtl::SelectTextureForCS(
        context, slot, SelectView(flags), WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8D6E20.
void PS4Texture2D::_SyncFromGpuImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8D6E30.
void PS4Texture2D::_Slot20Impl() {}

// Reconstructed from eboot.elf at 0x8D6460. Depth textures get depth,
// stencil, and HTILE storage; color storage may share a compatible texture's
// allocation.
void PS4Texture2D::_SyncStaticImpl(const RndTextureBase* reuse) {
    if (mBaseDesc.mFormat.mUsage == kTextureUsageDepth) {
        _SyncDepthStencil();
    } else {
        _SyncRegular(reuse);
    }
}

// Reconstructed from eboot.elf at 0x8D6D10. Uploads the pixels into the
// other buffer and makes it the active one.
void PS4Texture2D::_SyncDynamicImpl(RndContext&) {
    mActiveStorage = (mActiveStorage & 1U) == 0 ? 1 : 0;
    const RndPixelData* level = &mPixels;
    for (unsigned long mip = 0; mip < mPixels.GetNumMips() + 1; ++mip) {
        auto* texture = mGpuTextures[mActiveStorage];
        sce::GpuAddress::TilingParameters tiling;
        tiling.initFromTexture(texture, static_cast<std::uint32_t>(mip), 0);
        std::uint64_t offset;
        std::uint64_t size;
        sce::GpuAddress::computeTextureSurfaceOffsetAndSize(
            &offset, &size, texture, static_cast<std::uint32_t>(mip), 0);
        auto* surface = AlignUp(mStorage->mSurfaces[mActiveStorage], mSizeAlign.m_align);
        sce::GpuAddress::tileSurface(surface + offset, level->mBuffer, &tiling);
        level = level->mMip;
    }
}

const sce::Gnm::RenderTarget* PS4Texture2D::GetRenderTarget() const {
    const auto frame =
        static_cast<const PS4Window*>(gPS4Device->mMainWindow)->mActiveBuffer;
    auto* target = mRenderTargets[frame];
    return target != nullptr ? target : mRenderTargets[0];
}

// Inlined into _SyncStaticImpl at 0x8D6460. A depth texture is an
// HTILE-accelerated depth target with fresh storage, viewed through a depth
// texture and a stencil-plane texture.
void PS4Texture2D::_SyncDepthStencil() {
    const int dataFormat = mPixels.mFormat;
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
    spec.m_numSlices = 1;
    spec.m_zFormat = zFormat;
    spec.m_stencilFormat = stencilFormat;
    spec.m_tileModeHint = tileMode;
    spec.m_numFragments = sce::Gnm::kNumFragments1;
    spec.m_flags.enableHtileAcceleration = 1;
    spec.m_minGpuMode = gpuMode;
    mDepthTarget = new sce::Gnm::DepthRenderTarget();
    mDepthTarget->init(&spec);
    mSizeAlign = mDepthTarget->getZSizeAlign();
    mStencilSizeAlign = mDepthTarget->getStencilSizeAlign();
    mHtileSizeAlign = mDepthTarget->getHtileSizeAlign();

    mStorage = std::make_shared<Storage>();
    static long sGpuHeap = MemFindHeap("gpu");
    MemPushHeap(sGpuHeap);
    mStorage->mSurfaces[0] =
        MemAlloc(mSizeAlign.m_size, mBaseDesc.mName, static_cast<int>(mSizeAlign.m_align));
    mStorage->mSurfaceSizes[0] = mSizeAlign.m_size;
    mStorage->mStencil = MemAlloc(
        mStencilSizeAlign.m_size, mBaseDesc.mName, static_cast<int>(mStencilSizeAlign.m_align));
    mStorage->mStencilSize = mStencilSizeAlign.m_size;
    mStorage->mHtile = MemAlloc(
        mHtileSizeAlign.m_size, mBaseDesc.mName, static_cast<int>(mHtileSizeAlign.m_align));
    mStorage->mHtileSize = mHtileSizeAlign.m_size;
    MemPopHeap();

    auto* depth = AlignUp(mStorage->mSurfaces[0], mSizeAlign.m_align);
    auto* stencil = AlignUp(mStorage->mStencil, mStencilSizeAlign.m_align);
    mDepthTarget->setZReadAddress(depth);
    mDepthTarget->setZWriteAddress(depth);
    mDepthTarget->setStencilReadAddress(stencil);
    mDepthTarget->setStencilWriteAddress(stencil);
    mDepthTarget->setHtileAddress(AlignUp(mStorage->mHtile, mHtileSizeAlign.m_align));
    mDepthTarget->setHtileAccelerationEnable(true);
    mDepthTarget->setZCompareBase(sce::Gnm::kZCompareBaseZMin);

    mGpuTextures[0] = new sce::Gnm::Texture();
    mGpuTextures[0]->initFromDepthRenderTarget(mDepthTarget, false);
    mGpuTextures[0]->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeGC);
    mPlaneTexture = new sce::Gnm::Texture();
    mPlaneTexture->initFromStencilTarget(mDepthTarget, sce::Gnm::kTextureChannelTypeUInt, false);
    mPlaneTexture->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeGC);
}

// Inlined into _SyncStaticImpl at 0x8D6460. Dynamic textures get two buffers.
// A texture to reuse lends its storage when every buffer fits in it.
// Render targets also get a render target per buffer; render targets and
// GPU-writable textures use GPU-coherent memory.
void PS4Texture2D::_SyncRegular(const RndTextureBase* reuse) {
    const int dataFormat = mPixels.mFormat;
    const auto gpuMode = sce::Gnm::getGpuMode();
    sce::Gnm::TileMode tileMode;
    sce::GpuAddress::computeSurfaceTileMode(
        gpuMode,
        &tileMode,
        PS4RenderUtl::GetSurfaceType(mBaseDesc.mFormat),
        PS4RenderUtl::GetDataFormat(dataFormat),
        1);

    const unsigned long numBuffers = (mBaseDesc.mFormat.mFlags & kPixelFormatDynamic) + 1;
    sce::Gnm::TextureSpec spec;
    spec.init();
    spec.m_textureType = sce::Gnm::kTextureType2d;
    spec.m_width = mBaseDesc.mWidth;
    spec.m_height = mBaseDesc.mHeight;
    spec.m_depth = 1;
    spec.m_pitch = 0;
    spec.m_numSlices = 1;
    spec.m_format = PS4RenderUtl::GetDataFormat(dataFormat);
    spec.m_tileModeHint = tileMode;
    spec.m_numFragments = sce::Gnm::kNumFragments1;
    spec.m_numMipLevels = static_cast<std::uint32_t>(mPixels.GetNumMips() + 1);
    spec.m_minGpuMode = gpuMode;

    const auto* lender = static_cast<const PS4Texture2D*>(reuse);
    for (unsigned long buffer = 0; buffer < numBuffers; ++buffer) {
        mGpuTextures[buffer] = new sce::Gnm::Texture();
        mGpuTextures[buffer]->init(&spec);
        mSizeAlign = mGpuTextures[buffer]->getSizeAlign();
        if (lender != nullptr) {
            auto* surface = static_cast<unsigned char*>(lender->mStorage->mSurfaces[buffer]);
            if (AlignUp(surface, mSizeAlign.m_align) + mSizeAlign.m_size >
                surface + lender->mStorage->mSurfaceSizes[buffer]) {
                lender = nullptr;
            }
        }
    }
    if (lender != nullptr) {
        mStorage = lender->mStorage;
    } else {
        mStorage = std::make_shared<Storage>();
    }

    for (unsigned long buffer = 0; buffer < numBuffers; ++buffer) {
        if (mStorage->mSurfaces[buffer] == nullptr) {
            static long sGpuHeap = MemFindHeap("gpu");
            MemPushHeap(sGpuHeap);
            mStorage->mSurfaces[buffer] =
                MemAlloc(mSizeAlign.m_size, mBaseDesc.mName, static_cast<int>(mSizeAlign.m_align));
            mStorage->mSurfaceSizes[buffer] = mSizeAlign.m_size;
            MemPopHeap();
        }
        auto* surface = AlignUp(mStorage->mSurfaces[buffer], mSizeAlign.m_align);
        if (mPixels.mBuffer != nullptr) {
            const RndPixelData* level = &mPixels;
            for (unsigned long mip = 0; mip < mPixels.GetNumMips() + 1; ++mip) {
                sce::GpuAddress::TilingParameters tiling;
                tiling.initFromTexture(mGpuTextures[buffer], static_cast<std::uint32_t>(mip), 0);
                std::uint64_t offset;
                std::uint64_t size;
                sce::GpuAddress::computeTextureSurfaceOffsetAndSize(
                    &offset, &size, mGpuTextures[buffer], static_cast<std::uint32_t>(mip), 0);
                sce::GpuAddress::tileSurface(surface + offset, level->mBuffer, &tiling);
                level = level->mMip;
            }
        }
        mGpuTextures[buffer]->setBaseAddress(surface);

        const auto flags = mBaseDesc.mFormat.mFlags;
        if ((flags & (kPixelFormatRenderTarget | kPixelFormatGpuWritable)) == kPixelFormatRenderTarget) {
            mRenderTargets[buffer] = new sce::Gnm::RenderTarget();
            mRenderTargets[buffer]->initFromTexture(
                mGpuTextures[buffer],
                0,
                static_cast<sce::Gnm::NumSamples>(mGpuTextures[buffer]->getNumFragments()),
                nullptr,
                nullptr);
            mGpuTextures[buffer]->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeGC);
        } else if ((flags & kPixelFormatGpuWritable) != 0) {
            mGpuTextures[buffer]->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeGC);
        } else {
            mGpuTextures[buffer]->setResourceMemoryType(sce::Gnm::kResourceMemoryTypeRO);
        }
    }
}

// Reconstructed from eboot.elf at 0x8D7140. Copies the source into this texture
// with the float4 copy shader.
void PS4Texture2D::_GpuCopyFromImpl(RndContext& context, RndShaderResource& source) {
    _ValidateGpuCopyFrom(source);
    RndCShaderCopyBuffer::Params params{};
    params.mSrcTexture = static_cast<RndTextureBase*>(&source);
    params.mDestTexture = this;
    params.mNumericType = kShaderNumericFloat4;
    TheRndDevice()->mShaderMgr.mCopyBufferCShader->Dispatch(context, params);
}
