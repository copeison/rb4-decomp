#include "render/shaders/RndShaderProgram.h"

#include "render/system/RndFactory.h"

// Reconstructed from eboot.elf at 0x642250.
RndShaderProgram* RndShaderProgram::New(RndShaderProgramType type) {
    return TheRndFactory()->CreateShaderProgram(type);
}

// Reconstructed from eboot.elf at 0x642270.
RndShaderProgram::RndShaderProgram()
    : mKey(0), mCreated(false), mLastSelectFrame(-1), mName(nullptr) {}

// Reconstructed from eboot.elf at 0x6422C0.
bool RndShaderProgram::Create(
    unsigned long key,
    BinStream* stream,
    const char* name) {
    mKey = key;
    mName = name;
    Free();

    if (stream == nullptr) {
        return true;
    }

    mCreated = _CreateImpl(*stream);
    return mCreated;
}

// Reconstructed from eboot.elf at 0x642310.
void RndShaderProgram::Free() {
    if (!mCreated) {
        return;
    }

    _FreeImpl();
    mCreated = false;
}
