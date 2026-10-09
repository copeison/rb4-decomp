#include "renderps4/shaders/PS4ShaderProgramPixel.h"

// Reconstructed from eboot.elf at 0x8E43D0.
PS4ShaderProgramPixel::PS4ShaderProgramPixel() {
    _InitBackend();
}

// Reconstructed from eboot.elf at 0x8E4410. The deleting destructor at 0x8E4440
// releases the program through MemFree.
PS4ShaderProgramPixel::~PS4ShaderProgramPixel() {
    Free();
}

// Reconstructed from eboot.elf at 0x8E46B0.
RndShaderProgramType PS4ShaderProgramPixel::_GetTypeImpl() const {
    return kShaderProgramPixel;
}
