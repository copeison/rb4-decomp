#include "render/depth/RndShaderLinearizeDepth.h"

#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x63EF70.
RndShaderLinearizeDepth::RndShaderLinearizeDepth() : mTexture(-1) {}

RndShaderLinearizeDepth::~RndShaderLinearizeDepth() {}

const char* RndShaderLinearizeDepth::_GetClassNameImpl() const {
    return "RndShaderLinearizeDepth";
}

const char* RndShaderLinearizeDepth::_GetShaderFilePath() const {
    return "../../system/data/shaders/LinearizeDepth.hlsl";
}

void RndShaderLinearizeDepth::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig&,
    RndShaderResourceConfig& resources) {
    mTexture = resources.AddTexture(
        "gTexture",
        "gTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x63EFD0.
void RndShaderLinearizeDepth::Select(
    RndContext& context,
    RndTextureBase& depth) {
    RndShaderDrawUtl::SelectWithPixelTexture(*this, context, depth, mTexture);
}
