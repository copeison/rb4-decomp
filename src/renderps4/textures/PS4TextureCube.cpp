#include "renderps4/textures/PS4TextureCube.h"
#include "renderps4/system/PS4RenderUtl.h"
#include "renderps4/system/PS4Device.h"

// Reconstructed from eboot.elf at 0x8E6BA0.
PS4TextureCube::PS4TextureCube(const Description& desc)
    : RndTextureCube(desc),
      mGpuTexture(nullptr),
      mStorage(nullptr),
      mStorage2(nullptr),
      mRenderTarget(nullptr),
      mDepthTarget(nullptr) {}

// Reconstructed from eboot.elf at 0x8E6BE0. The deleting destructor at
// 0x8E6CC0 releases the texture through MemFree.
PS4TextureCube::~PS4TextureCube() {
    if (mRenderTarget != nullptr) {
        gPS4Device->DeferredDelete(TargetMetadata(*mRenderTarget));
        gPS4Device->DeferredDelete(TargetSurface(*mRenderTarget));
        operator delete(mRenderTarget);
        mRenderTarget = nullptr;
    }
    gPS4Device->DeferredDelete(mStorage);
    gPS4Device->DeferredDelete(mStorage2);
    operator delete(mGpuTexture);
    mGpuTexture = nullptr;
    operator delete(mDepthTarget);
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
