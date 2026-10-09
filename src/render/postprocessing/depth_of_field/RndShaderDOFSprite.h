#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndComputeBuffer;
class RndTexture2D;

// Draws the bokeh sprites the disc blur emits, expanding each sprite through
// the geometry program.
class RndShaderDOFSprite : public RndShader {
public:
    RndShaderDOFSprite();  // 0x6F3330
    ~RndShaderDOFSprite() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;
    bool _UsesCustomGeometryShader() const override;

    void Select(
        RndContext& context,
        RndTexture2D& bokeh,
        RndComputeBuffer& sprites);  // 0x6F3390

    // Field names are not in the reference map.
    unsigned long mBokehTex;  // Resource indices.
    unsigned long mSprites;
};

static_assert(offsetof(RndShaderDOFSprite, mBokehTex) == 288);
static_assert(sizeof(RndShaderDOFSprite) == 304);
