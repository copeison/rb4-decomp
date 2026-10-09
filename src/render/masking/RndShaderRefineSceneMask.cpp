#include "render/masking/RndShaderRefineSceneMask.h"

#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x642360.
RndShaderRefineSceneMask::RndShaderRefineSceneMask() : mUnrefinedMask(-1) {}

RndShaderRefineSceneMask::~RndShaderRefineSceneMask() {}

const char* RndShaderRefineSceneMask::_GetClassNameImpl() const {
    return "RndShaderRefineSceneMask";
}

const char* RndShaderRefineSceneMask::_GetShaderFilePath() const {
    return "../../system/data/shaders/RefineSceneMask.hlsl";
}

void RndShaderRefineSceneMask::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig&,
    RndShaderResourceConfig& resources) {
    mUnrefinedMask = resources.AddTexture(
        "gUnrefinedMask",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x6423C0.
void RndShaderRefineSceneMask::Select(
    RndContext& context,
    RndTextureBase& unrefinedMask) {
    RndShaderDrawUtl::SelectWithPixelTexture(
        *this, context, unrefinedMask, mUnrefinedMask);
}
