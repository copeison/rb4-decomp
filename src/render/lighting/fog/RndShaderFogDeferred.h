#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Deferred fog over the sky and linear depth. Not in the reference map; the
// device creates it at 0x451C90 and deletes it at 0x451CC0.
class RndShaderFogDeferred : public RndShader {
public:
    // Registers the shader with the shader manager.
    RndShaderFogDeferred();             // 0x452CA0
    ~RndShaderFogDeferred() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    RndShaderDefInfo mBT709ToBT2020;
    unsigned long mFalloffParams;  // Constant offset.
    unsigned long mSkyTex;         // Resource indices.
    unsigned long mLinearDepthMap;
    unsigned long mFunctionTable;
};

static_assert(offsetof(RndShaderFogDeferred, mFalloffParams) == 312);
static_assert(sizeof(RndShaderFogDeferred) == 344);
