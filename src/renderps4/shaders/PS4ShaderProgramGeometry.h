#pragma once

#include <cstddef>
#include <gnmx/shaderbinary.h>

#include "render/shaders/RndShaderProgram.h"

// PS4 geometry shader program: a GS-stage shader with its embedded VS-stage
// copy shader. The vtable is at 0x195F8A0.
class PS4ShaderProgramGeometry : public RndShaderProgram {
public:
    PS4ShaderProgramGeometry();            // 0x8E40A0
    ~PS4ShaderProgramGeometry() override;  // 0x8E40D0, 0x8E4100

    bool _CreateImpl(BinStream& stream) override;        // 0x8E4140
    void _SelectImpl(RndContext& context) override;      // 0x8E4330
    void _FreeImpl() override;                           // 0x8E4350
    RndShaderProgramType _GetTypeImpl() const override;  // 0x8E43A0

    // Field names are not in the reference map.
    const sce::Gnmx::GsShader* mGsShader;
    void* mShaderData;
    void* mGsShaderCode;
    void* mCopyShaderCode;
};

static_assert(offsetof(PS4ShaderProgramGeometry, mGsShader) == 40);
static_assert(offsetof(PS4ShaderProgramGeometry, mCopyShaderCode) == 64);
static_assert(sizeof(PS4ShaderProgramGeometry) == 72);
