#include "render/masking/RndShaderStencilSceneMask.h"

#include "render/core/settings/render_settings.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x644FC0.
RndShaderStencilSceneMask::RndShaderStencilSceneMask()
    : mUnrefinedMask(-1), mTileCounts(-1), mCBufferSize(0) {}

RndShaderStencilSceneMask::~RndShaderStencilSceneMask() {}

const char* RndShaderStencilSceneMask::_GetClassNameImpl() const {
    return "RndShaderStencilSceneMask";
}

const char* RndShaderStencilSceneMask::_GetShaderFilePath() const {
    return "../../system/data/shaders/StencilSceneMask.hlsl";
}

void RndShaderStencilSceneMask::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    static const rb4::RenderSettings sDefaultSettings{};
    auto* device = TheRndDevice();
    const auto* settings = device != nullptr ? device->mSettings : nullptr;
    fixedDefines.Add(
        Symbol("HX_TILE_SIZE"),
        static_cast<int>(
            (settings != nullptr ? *settings : sDefaultSettings).light_tile_size));

    mUnrefinedMask = resources.AddTexture(
        "gUnrefinedMask",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramVertex,
        kShaderNumericFloat4);
    mTileCounts = cbuffer.AddConstant(kShaderNumericFloat2, "gTileCounts");
    mCBufferSize = cbuffer.mSize;
}

// Only the vertex program.
int RndShaderStencilSceneMask::_GetShaderStages() const {
    return 1;
}
