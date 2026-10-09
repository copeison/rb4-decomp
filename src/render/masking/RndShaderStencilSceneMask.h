#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Writes the scene mask into the stencil buffer per light tile. It has only a
// vertex program. Not in the reference map; the name is the binary's
// class-name string.
class RndShaderStencilSceneMask : public RndShader {
public:
    RndShaderStencilSceneMask();             // 0x644FC0
    ~RndShaderStencilSceneMask() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;
    int _GetShaderStages() const override;

    // Field names are not in the reference map.
    unsigned long mUnrefinedMask;  // Resource index.
    unsigned long mTileCounts;     // Constant offset and the buffer size.
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndShaderStencilSceneMask, mUnrefinedMask) == 288);
static_assert(offsetof(RndShaderStencilSceneMask, mCBufferSize) == 304);
static_assert(sizeof(RndShaderStencilSceneMask) == 312);
