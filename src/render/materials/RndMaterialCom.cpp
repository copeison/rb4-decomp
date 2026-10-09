#include "render/materials/RndMaterialCom.h"

#include "os/files/File.h"

// Reconstructed from eboot.elf at 0x4F3790.
void RndMaterialCom::SetShaderGraphFile(const char* file) {
    mShaderGraphFile = file;
}

// Reconstructed from eboot.elf at 0x4F3A00.
void RndMaterialCom::SetBlendMode(RndBlendMode mode) {
    if (mBlendMode != mode) {
        mBlendMode = mode;
        mRenderStateDirty = true;
    }
}
