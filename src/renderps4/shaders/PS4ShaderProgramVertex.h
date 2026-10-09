#pragma once

#include <cstddef>
#include <gnmx/shaderbinary.h>

#include "render/shaders/RndShaderProgram.h"

// PS4 vertex shader program. One compiled vertex shader is kept as both a
// VS-stage shader and, for draws with a geometry shader, an ES-stage shader,
// each with its own fetch shader. The vtable is at 0x195F920.
class PS4ShaderProgramVertex : public RndShaderProgram {
public:
    PS4ShaderProgramVertex();            // 0x8E46E0
    ~PS4ShaderProgramVertex() override;  // 0x8E4720, 0x8E4750

    bool _CreateImpl(BinStream& stream) override;        // 0x8E4790
    void _SelectImpl(RndContext& context) override;      // 0x8E4D80
    void _FreeImpl() override;                           // 0x8E4ED0
    RndShaderProgramType _GetTypeImpl() const override;  // 0x8E4F30

    // Field names are not in the reference map.
    const sce::Gnmx::VsShader* mVsShader;
    const sce::Gnmx::EsShader* mEsShader;
    void* mVsShaderData;
    void* mEsShaderData;
    void* mShaderCode;
    unsigned int mShaderModifier;
    void* mVsFetchShader;
    void* mEsFetchShader;
};

static_assert(offsetof(PS4ShaderProgramVertex, mVsShader) == 40);
static_assert(offsetof(PS4ShaderProgramVertex, mShaderCode) == 72);
static_assert(offsetof(PS4ShaderProgramVertex, mShaderModifier) == 80);
static_assert(offsetof(PS4ShaderProgramVertex, mEsFetchShader) == 96);
static_assert(sizeof(PS4ShaderProgramVertex) == 104);
