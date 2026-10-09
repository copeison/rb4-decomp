#include "renderps4/shaders/PS4ShaderProgramCompute.h"

// Reconstructed from eboot.elf at 0x8E3D20.
PS4ShaderProgramCompute::PS4ShaderProgramCompute() {
    _InitBackend();
}

// Reconstructed from eboot.elf at 0x8E3D50. The deleting destructor at 0x8E3D80
// releases the program through MemFree.
PS4ShaderProgramCompute::~PS4ShaderProgramCompute() {
    Free();
}

// Reconstructed from eboot.elf at 0x8E4070.
RndShaderProgramType PS4ShaderProgramCompute::_GetTypeImpl() const {
    return kShaderProgramCompute;
}
