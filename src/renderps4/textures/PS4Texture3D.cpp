#include "renderps4/textures/PS4Texture3D.h"
#include "renderps4/system/PS4RenderUtl.h"
#include "renderps4/system/PS4Device.h"

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

// Reconstructed from eboot.elf at 0x8E5750.
void PS4Texture3D::_SyncFromGpuImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8E5760.
void PS4Texture3D::_Slot20Impl() {}
