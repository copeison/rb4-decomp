#include "render/postprocessing/depth_of_field/RndShaderDOFSprite.h"

#include "render/buffers/RndComputeBuffer.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTexture2D.h"

// Reconstructed from eboot.elf at 0x6F3330.
RndShaderDOFSprite::RndShaderDOFSprite() : mBokehTex(-1), mSprites(-1) {}

RndShaderDOFSprite::~RndShaderDOFSprite() {}

const char* RndShaderDOFSprite::_GetClassNameImpl() const {
    return "RndShaderDOFSprite";
}

const char* RndShaderDOFSprite::_GetShaderFilePath() const {
    return "../../system/data/shaders/DOFSprite.hlsl";
}

void RndShaderDOFSprite::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig&,
    RndShaderResourceConfig& resources) {
    mBokehTex = resources.AddTexture(
        "gBokehTex",
        "gBokehTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    // Buffer usage 0.
    mSprites = resources.AddComputeBufferCustomTyped(
        "gSprites", "CSBokehSprite", 0, kShaderProgramVertex);
}

bool RndShaderDOFSprite::_UsesCustomGeometryShader() const {
    return true;
}

// Reconstructed from eboot.elf at 0x6F3390. Binds the bokeh texture to the
// pixel stage and the sprite buffer to the vertex stage, which expands each
// sprite through the geometry program.
void RndShaderDOFSprite::Select(
    RndContext& context,
    RndTexture2D& bokeh,
    RndComputeBuffer& sprites) {
    bokeh.Select(context, kShaderProgramPixel, mBokehTex, 0, 0);
    sprites.Select(context, kShaderProgramVertex, mSprites, 0, 0);
    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);
}
