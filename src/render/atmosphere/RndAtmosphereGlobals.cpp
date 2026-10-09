// render/RndAtmosphereGlobals.o (0x451C70 to 0x451CEF). The initializer
// after it (0x451CF0) sets the shared header's ints for the next object and
// is not modelled.
#include "render/atmosphere/RndAtmosphereGlobals.h"

#include "render/lighting/fog/RndShaderFogDeferred.h"

// Reconstructed from eboot.elf at 0x451C70.
RndAtmosphereGlobals::RndAtmosphereGlobals() : mFogDeferred(nullptr) {}

// Reconstructed from eboot.elf at 0x451C80.
RndAtmosphereGlobals::~RndAtmosphereGlobals() {}

// Reconstructed from eboot.elf at 0x451C90.
void RndAtmosphereGlobals::Init() {
    RndShaderFogDeferred* shader = new RndShaderFogDeferred;
    shader->_Register();
    mFogDeferred = shader;
}

// Reconstructed from eboot.elf at 0x451CC0.
void RndAtmosphereGlobals::Terminate() {
    delete mFogDeferred;
    mFogDeferred = nullptr;
}
