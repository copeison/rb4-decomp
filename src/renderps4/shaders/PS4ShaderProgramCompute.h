#pragma once

#include <cstddef>
#include <gnmx/shaderbinary.h>

#include "render/shaders/RndShaderProgram.h"

// PS4 compute shader program. The vtable is at 0x195F860.
class PS4ShaderProgramCompute : public RndShaderProgram {
public:
    PS4ShaderProgramCompute();            // 0x8E3D20
    ~PS4ShaderProgramCompute() override;  // 0x8E3D50, 0x8E3D80

    bool _CreateImpl(BinStream& stream) override;        // 0x8E3DC0
    void _SelectImpl(RndContext& context) override;      // 0x8E3F40
    void _FreeImpl() override;                           // 0x8E4030
    RndShaderProgramType _GetTypeImpl() const override;  // 0x8E4070

    // Field names are not in the reference map. The constructor leaves them
    // uninitialized.
    const sce::Gnmx::CsShader* mCsShader;
    void* mShaderData;
    void* mShaderCode;
};

static_assert(offsetof(PS4ShaderProgramCompute, mCsShader) == 40);
static_assert(offsetof(PS4ShaderProgramCompute, mShaderCode) == 56);
static_assert(sizeof(PS4ShaderProgramCompute) == 64);
