#include "render/postprocessing/depth_of_field/RndShaderDOFSprite.h"

#include "render/shaders/shader_draw_state.h"
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
    rb4::render_shader_bind_texture(
        context, bokeh, kShaderProgramPixel, mBokehTex, 0);
    rb4::render_shader_bind_buffer(
        context, sprites, kShaderProgramVertex, mSprites, 0);
    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);
}
