#include "renderps4/shaders/PS4ShaderProgramVertex.h"

// Reconstructed from eboot.elf at 0x8E46E0.
PS4ShaderProgramVertex::PS4ShaderProgramVertex() {
    _InitBackend();
}

// Reconstructed from eboot.elf at 0x8E4720. The deleting destructor at 0x8E4750
// releases the program through MemFree.
PS4ShaderProgramVertex::~PS4ShaderProgramVertex() {
    Free();
}

// Reconstructed from eboot.elf at 0x8E4F30.
RndShaderProgramType PS4ShaderProgramVertex::_GetTypeImpl() const {
    return kShaderProgramVertex;
}
