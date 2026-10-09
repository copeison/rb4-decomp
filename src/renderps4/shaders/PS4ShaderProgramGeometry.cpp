#include "renderps4/shaders/PS4ShaderProgramGeometry.h"

// Reconstructed from eboot.elf at 0x8E40A0.
PS4ShaderProgramGeometry::PS4ShaderProgramGeometry() {
    _InitBackend();
}

// Reconstructed from eboot.elf at 0x8E40D0. The deleting destructor at 0x8E4100
// releases the program through MemFree.
PS4ShaderProgramGeometry::~PS4ShaderProgramGeometry() {
    Free();
}

// Reconstructed from eboot.elf at 0x8E43A0.
RndShaderProgramType PS4ShaderProgramGeometry::_GetTypeImpl() const {
    return kShaderProgramGeometry;
}
