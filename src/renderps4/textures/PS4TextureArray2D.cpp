#include "renderps4/textures/PS4TextureArray2D.h"
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
    PS4DeferredDelete(mStorage);
    PS4DeferredDelete(mStencilStorage);
    PS4DeferredDelete(mHtileStorage);
    if (mColorTarget != nullptr) {
        PS4DeferredDelete(ColorTargetMetadata(mColorTarget));
        operator delete(mColorTarget);
        mColorTarget = nullptr;
    }
    operator delete(mGpuTexture);
    mGpuTexture = nullptr;
    operator delete(mDepthTarget);
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
