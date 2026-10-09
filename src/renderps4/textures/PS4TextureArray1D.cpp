#include "renderps4/textures/PS4TextureArray1D.h"
#include "renderps4/system/PS4RenderUtl.h"

// Reconstructed from eboot.elf at 0x8E5870.
PS4TextureArray1D::PS4TextureArray1D(const Description& desc)
    : RndTextureArray1D(desc),
      mGpuTexture(nullptr), mStorage(nullptr) {}

// Reconstructed from eboot.elf at 0x8E58A0. The deleting destructor at
// 0x8E5900 releases the texture through MemFree.
PS4TextureArray1D::~PS4TextureArray1D() {
    PS4DeferredDelete(mStorage);
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

// Reconstructed from eboot.elf at 0x8E5C30.
void PS4TextureArray1D::_SyncFromGpuImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8E5C40.
void PS4TextureArray1D::_Slot20Impl() {}
