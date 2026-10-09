#pragma once

#include "render/shaders/RndShaderDefines.h"
#include "render/shaders/RndShaderEnums.h"
#include "utl/containers/Vector.h"

class BinStream;
class RndContext;
class RndShaderProgram;

// The compiled programs of one shader, per program type, sorted by key.
class RndShaderCollection {
public:
    ~RndShaderCollection();

    void Free();  // Name from the map; inlined at 0x6388C0.
    // Reads the programs from a compiled-shader cache. The map's signature
    // is Load(BinStream&); this build also passes the shader's file path.
    bool Load(BinStream& stream, const char* path);  // 0x63B2B0
    // Selects the program of each stage the shader uses, by its key. Fails
    // when a program is missing.
    bool Select(
        RndContext& context,
        unsigned int stages,
        const RndShaderKeyGroup& keys);  // 0x63B500

    static RndShaderProgram* _GetShaderProgram(
        eastl::vector<RndShaderProgram*>& programs,
        RndShaderKey key);

    eastl::vector<RndShaderProgram*> mPrograms[kNumShaderProgramTypes];  // Name not in the reference map.
};

static_assert(sizeof(RndShaderCollection) == 192);
