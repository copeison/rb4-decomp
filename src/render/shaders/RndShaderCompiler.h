#pragma once

#include <cstddef>

class BinStream;

// A compiled shader binary as stored in a shader collection: a 32-bit byte
// count followed by the bytes (render/RndShaderCompiler.o). The PS4 shader
// programs load one into temporary memory and parse it. Only the members
// this build uses are declared.
class RndShaderCompilerBlob {
public:
    RndShaderCompilerBlob();   // 0x11B2EE0
    ~RndShaderCompilerBlob();  // 0x11B2EF0

    // Releases the bytes.
    void Free();  // 0x11B2F00
    // Reads the byte count and, when nonzero, allocates and reads the bytes.
    void Load(BinStream& stream);  // 0x11B2F30

    // Field names are not in the reference map.
    unsigned char* mData;
    unsigned long mSize;
};

static_assert(offsetof(RndShaderCompilerBlob, mSize) == 8);
static_assert(sizeof(RndShaderCompilerBlob) == 16);
