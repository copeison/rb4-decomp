#include "render/shaders/RndShaderCompiler.h"

#include "utl/streams/BinStream.h"

// Reconstructed from eboot.elf at 0x11B2EE0.
RndShaderCompilerBlob::RndShaderCompilerBlob() : mData(nullptr), mSize(0) {}

// Reconstructed from eboot.elf at 0x11B2EF0. The bytes are not released; the
// owner calls Free.
RndShaderCompilerBlob::~RndShaderCompilerBlob() {}

// Reconstructed from eboot.elf at 0x11B2F00.
void RndShaderCompilerBlob::Free() {
    delete[] mData;
    mData = nullptr;
    mSize = 0;
}

// Reconstructed from eboot.elf at 0x11B2F30.
void RndShaderCompilerBlob::Load(BinStream& stream) {
    unsigned int size;
    stream.ReadEndian(&size, sizeof(size));
    mSize = size;
    if (size != 0) {
        mData = new unsigned char[size];
        stream.Read(mData, size);
    }
}
