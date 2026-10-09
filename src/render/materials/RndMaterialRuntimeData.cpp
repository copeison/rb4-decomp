#include "render/materials/RndMaterialRuntimeData.h"

#include "render/buffers/RndShaderCBuffer.h"

// Reconstructed from eboot.elf at 0x4F7580. Deletes the constant buffer;
// the members then release their resource references.
RndMaterialRuntimeData::~RndMaterialRuntimeData() {
    RndShaderCBuffer::SafeDelete(mCBuffer);
}
