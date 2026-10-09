#include "renderps4/textures/PS4Texture2D.h"
#include "renderps4/system/PS4RenderUtl.h"
#include "renderps4/system/PS4Device.h"
#include "render/platform/orbis/video/orbis_back_buffer.h"

// Reconstructed from eboot.elf at 0x8D62C0.
PS4Texture2D::PS4Texture2D(const Description& desc)
    : RndTexture2D(desc),
      mGpuTextures{nullptr, nullptr},
      mPlaneTexture(nullptr),
      mDepthSize{},
      mStencilSize{},
      mHtileSize{},
      mActiveStorage(0),
      mStorageRegions(nullptr),
      mStorageControl(nullptr),
      mRenderTargets{nullptr, nullptr},
      mDepthTarget(nullptr),
      mPendingPresentations(nullptr) {}

// Reconstructed from eboot.elf at 0x8D6310. The deleting destructor at
// 0x8D6440 releases the texture through MemFree.
PS4Texture2D::~PS4Texture2D() {
    for (auto*& target : mRenderTargets) {
        operator delete(target);
        target = nullptr;
    }
    for (auto*& texture : mGpuTextures) {
        operator delete(texture);
        texture = nullptr;
    }
    operator delete(mPlaneTexture);
    mPlaneTexture = nullptr;
    operator delete(mDepthTarget);
    mDepthTarget = nullptr;
    delete[] mPendingPresentations;
    mPendingPresentations = nullptr;
    ReleaseStorage(mStorageControl);
    mStorageControl = nullptr;
    mStorageRegions = nullptr;
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

// Reconstructed from eboot.elf at 0x8D6D10.
void PS4Texture2D::_SyncDynamicImpl(RndContext&) {
    FlipStorage();
    UploadMips();
}

const rb4::OrbisGpuRenderTarget* PS4Texture2D::GetRenderTarget() const {
    const auto frame = reinterpret_cast<const rb4::OrbisBackBuffer*>(
        gPS4Device->mMainWindow)->active_buffer;
    auto* target = mRenderTargets[frame];
    return target != nullptr ? target : mRenderTargets[0];
}
