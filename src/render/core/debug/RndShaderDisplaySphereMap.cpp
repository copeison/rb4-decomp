#include "render/core/debug/RndShaderDisplaySphereMap.h"

#include "render/resources/shaders/shader_draw_state.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTexture2D.h"

// Reconstructed from eboot.elf at 0x6F4270.
RndShaderDisplaySphereMap::RndShaderDisplaySphereMap() : mSphereMap(-1) {}

RndShaderDisplaySphereMap::~RndShaderDisplaySphereMap() {}

const char* RndShaderDisplaySphereMap::_GetClassNameImpl() const {
    return "RndShaderDisplaySphereMap";
}

const char* RndShaderDisplaySphereMap::_GetShaderFilePath() const {
    return "../../system/data/shaders/DisplaySphereMap.hlsl";
}

void RndShaderDisplaySphereMap::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig&,
    RndShaderResourceConfig& resources) {
    mSphereMap = resources.AddTexture(
        "gSphereMap",
        "gSphereMapSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x6F42D0.
void RndShaderDisplaySphereMap::Select(
    RndContext& context,
    RndTexture2D& sphereMap) {
    rb4::render_shader_draw_with_pixel_texture(
        *this, context, sphereMap, mSphereMap);
}
