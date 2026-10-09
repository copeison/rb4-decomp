#include "renderps4/textures/PS4TextureArrayCube.h"
#include "renderps4/system/PS4RenderUtl.h"
#include "renderps4/system/PS4Device.h"

// Reconstructed from eboot.elf at 0x8E6640.
PS4TextureArrayCube::PS4TextureArrayCube(const Description& desc)
    : RndTextureArrayCube(desc),
      mGpuTexture(nullptr), mStorage(nullptr) {}

// Reconstructed from eboot.elf at 0x8E6670. The deleting destructor at
// 0x8E66D0 releases the texture through MemFree.
PS4TextureArrayCube::~PS4TextureArrayCube() {
    gPS4Device->DeferredDelete(mStorage);
    operator delete(mGpuTexture);
    mGpuTexture = nullptr;
}

// Reconstructed from eboot.elf at 0x8E6AB0.
void PS4TextureArrayCube::_SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int) {
    PS4RenderUtl::SelectTextureForVS(
        context, slot, mGpuTexture, WrapMode(), FilterMode());
}

// Reconstructed from eboot.elf at 0x8E6AD0.
void PS4TextureArrayCube::_SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForHS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6AF0.
void PS4TextureArrayCube::_SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForDS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6B10.
void PS4TextureArrayCube::_SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForGS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6B30.
void PS4TextureArrayCube::_SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) {
    PS4RenderUtl::SelectTextureForPS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6B50.
void PS4TextureArrayCube::_SelectForCSImpl(RndContext& context, unsigned long slot, unsigned int flags, unsigned long) {
    PS4RenderUtl::SelectTextureForCS(
        context, slot, mGpuTexture, WrapMode(), FilterMode(), flags);
}

// Reconstructed from eboot.elf at 0x8E6A90.
void PS4TextureArrayCube::_SyncFromGpuImpl(RndContext&) {}

// Reconstructed from eboot.elf at 0x8E6AA0.
void PS4TextureArrayCube::_Slot20Impl() {}
