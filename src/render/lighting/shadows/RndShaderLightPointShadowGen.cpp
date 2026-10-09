#include "render/lighting/shadows/RndShaderLightPointShadowGen.h"

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Name not in the reference map.
constexpr unsigned long kPixelKey = 3;

}  // namespace

// Reconstructed from eboot.elf at 0x4ABB20. The constant offsets stay unset
// until _InitConfigImpl.
RndShaderLightPointShadowGen::RndShaderLightPointShadowGen()
    : mSoften{},
      mDownsampleDepth2x{},
      mShadowMap(-1),
      mTiledClassificationBuffer(-1) {}

RndShaderLightPointShadowGen::~RndShaderLightPointShadowGen() {}

// Reconstructed from eboot.elf at 0x4ABBA0.
void RndShaderLightPointShadowGen::Select(RndContext& context, Params& params) {
    if (params.mShadowMap != nullptr) {
        params.mShadowMap->Select(context, kShaderProgramPixel, mShadowMap, 0, 0);
    }
    if (params.mTiledClassificationBuffer != nullptr) {
        params.mTiledClassificationBuffer->Select(
            context,
            kShaderProgramPixel,
            mTiledClassificationBuffer,
            RndShaderResource::kSelectReadWrite,
            0);
    }
    RndShaderKeyGroup keys{};
    const auto key = mSoften.SetValue(0, params.mSoften ? 1U : 0U);
    keys.mKeys[kPixelKey] =
        mDownsampleDepth2x.SetValue(key, params.mDownsampleDepth2x ? 1U : 0U);
    params.mIllumType = 0;
    _SelectBase(context, params, keys);
}

const char* RndShaderLightPointShadowGen::_GetShaderFilePath() const {
    return "../../system/data/shaders/LightPointShadowGen.hlsl";
}

// Reconstructed from eboot.elf at 0x4ABD00.
void RndShaderLightPointShadowGen::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    RndLightDeferredShader::_InitConfigImpl(
        fixedDefines, defines, cbuffer, resources);
    auto& pixel = defines.GetDefines(kShaderProgramPixel);
    mSoften = pixel.AddBool(Symbol("HX_SOFTEN"));
    mDownsampleDepth2x = pixel.AddBool(Symbol("HX_DOWNSAMPLE_DEPTH_2X"));

    mShadowCamParams =
        cbuffer.AddConstant(kShaderNumericFloat4, "gShadowCamParams");
    mShadowOffset = cbuffer.AddConstant(kShaderNumericFloat, "gShadowOffset");
    mSoftShadowParams =
        cbuffer.AddConstant(kShaderNumericFloat4, "gSoftShadowParams");
    mLightPosCamSpace =
        cbuffer.AddConstant(kShaderNumericFloat3, "gLightPosCamSpace");
    mCamToLightXfm =
        cbuffer.AddConstant(kShaderNumericFloat3x4, "gCamToLightXfm");

    mShadowMap = resources.AddTexture(
        "gShadowMap",
        "gShadowMapSampler",
        RndTextureBase::kTextureCube,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTiledClassificationBuffer = resources.AddTextureWritable(
        "gTiledClassificationBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat);
}

const char* RndShaderLightPointShadowGen::_GetClassNameImpl() const {
    return "RndShaderLightPointShadowGen";
}
