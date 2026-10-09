#include "renderps4/textures/PS4Texture1D.h"
#include "renderps4/system/PS4RenderUtl.h"
#include "renderps4/system/PS4Device.h"

// Reconstructed from eboot.elf at 0x8E4F60.
PS4Texture1D::PS4Texture1D(const Description& desc)
    : RndTexture1D(desc),
      mGpuTexture(nullptr), mStorage(nullptr) {}

// Reconstructed from eboot.elf at 0x8E4F90. The deleting destructor at
// 0x8E4FF0 releases the texture through MemFree.
PS4Texture1D::~PS4Texture1D() {
    gPS4Device->DeferredDelete(mStorage);
    operator delete(mGpuTexture);
    mGpuTexture = nullptr;
}

// Reconstructed from eboot.elf at 0x8E52D0.
void PS4Texture1D::_SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int) {
    PS4RenderUtl::SelectTextureForVS(
        context, slot, mGpuTexture, WrapMode(), FilterMode());
}

// Reconstructed from eboot.elf at 0x8E52F0.
void PS4Texture1D::_SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForHS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E5310.
void PS4Texture1D::_SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForDS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E5330.
void PS4Texture1D::_SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForGS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E5350.
void PS4Texture1D::_SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForPS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E5370.
void PS4Texture1D::_SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long) {
    PS4RenderUtl::SelectTextureForCS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E52B0.
void PS4Texture1D::_SyncFromGpuImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8E52C0.
void PS4Texture1D::_Slot20Impl() {}
