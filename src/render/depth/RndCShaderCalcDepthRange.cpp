#include "render/depth/RndCShaderCalcDepthRange.h"

#include "render/core/settings/render_settings.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x636DE0.
RndCShaderCalcDepthRange::RndCShaderCalcDepthRange()
    : mLinearDepthBuffer(-1),
      mTiledDepthRangeBuffer(-1),
      mTileCounts(-1),
      mCBufferSize(0) {}

RndCShaderCalcDepthRange::~RndCShaderCalcDepthRange() {}

const char* RndCShaderCalcDepthRange::_GetClassNameImpl() const {
    return "RndCShaderCalcDepthRange";
}

const char* RndCShaderCalcDepthRange::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/CalcDepthRange.hlsl";
}

// Reads the device's settings without a null check.
void RndCShaderCalcDepthRange::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    const auto& settings = *TheRndDevice()->mSettings;
    fixedDefines.Add(
        Symbol("HX_TILE_SIZE"), static_cast<int>(settings.light_tile_size));

    mLinearDepthBuffer = resources.AddTexture(
        "gLinearDepthBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mTiledDepthRangeBuffer = resources.AddTextureWritable(
        "gTiledDepthRangeBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mTileCounts = cbuffer.AddConstant(kShaderNumericFloat2, "gTileCounts");
    mCBufferSize = cbuffer.mSize;
}
