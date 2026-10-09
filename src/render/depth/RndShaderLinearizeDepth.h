#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndTextureBase;

// Full-screen pass that converts the depth buffer to linear depth.
class RndShaderLinearizeDepth : public RndShader {
public:
    RndShaderLinearizeDepth();             // 0x63EF70
    ~RndShaderLinearizeDepth() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Binds the depth texture and selects the default programs.
    void Select(RndContext& context, RndTextureBase& depth);  // 0x63EFD0

    // Field name is not in the reference map.
    unsigned long mTexture;  // Resource index.
};

static_assert(offsetof(RndShaderLinearizeDepth, mTexture) == 288);
static_assert(sizeof(RndShaderLinearizeDepth) == 296);
