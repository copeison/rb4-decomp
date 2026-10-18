#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndTextureBase;

// Full-screen pass that converts the depth buffer to linear depth. The
// vtable is at 0x192F2F8.
class RndShaderLinearizeDepth : public RndShader {
public:
    RndShaderLinearizeDepth();             // 0x63EF70
    ~RndShaderLinearizeDepth() override;  // 0x63EFA0, 0x63EFB0

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x63F0E0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x63F080
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x63F090

    // Binds the depth texture and selects the default programs.
    void Select(RndContext& context, RndTextureBase& depth);  // 0x63EFD0

    // Field name is not in the reference map.
    unsigned long mTexture;  // Resource index.
};

static_assert(offsetof(RndShaderLinearizeDepth, mTexture) == 288);
static_assert(sizeof(RndShaderLinearizeDepth) == 296);
