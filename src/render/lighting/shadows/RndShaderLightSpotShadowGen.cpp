#include "render/lighting/shadows/RndShaderLightSpotShadowGen.h"

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Name not in the reference map.
constexpr unsigned long kPixelKey = 3;

}  // namespace

// Reconstructed from eboot.elf at 0x4ABED0. Only the shadow-map index is
// initialized; the other indices and offsets stay unset until
// _InitConfigImpl.
RndShaderLightSpotShadowGen::RndShaderLightSpotShadowGen()
    : mSoften{}, mDownsampleDepth2x{}, mShadowMap(-1) {}

RndShaderLightSpotShadowGen::~RndShaderLightSpotShadowGen() {}

// Reconstructed from eboot.elf at 0x4ABF50.
void RndShaderLightSpotShadowGen::Select(RndContext& context, Params& params) {
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

const char* RndShaderLightSpotShadowGen::_GetShaderFilePath() const {
    return "../../system/data/shaders/LightSpotShadowGen.hlsl";
}

// Reconstructed from eboot.elf at 0x4AC0B0.
void RndShaderLightSpotShadowGen::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    RndLightDeferredShader::_InitConfigImpl(
        fixedDefines, defines, cbuffer, resources);
    auto& pixel = defines.GetDefines(kShaderProgramPixel);
    mSoften = pixel.AddBool(Symbol("HX_SOFTEN"));
    mDownsampleDepth2x = pixel.AddBool(Symbol("HX_DOWNSAMPLE_DEPTH_2X"));

    mLightMeshParams0 =
        cbuffer.AddConstant(kShaderNumericFloat4, "gLightMeshParams0");
    mLightMeshParams1 =
        cbuffer.AddConstant(kShaderNumericFloat2, "gLightMeshParams1");
    mShadowParams = cbuffer.AddConstant(kShaderNumericFloat4, "gShadowParams");
    mShadowNearFarParams =
        cbuffer.AddConstant(kShaderNumericFloat4, "gShadowNearFarParams");
    mShadowOffset = cbuffer.AddConstant(kShaderNumericFloat, "gShadowOffset");
    mSoftShadowParams =
        cbuffer.AddConstant(kShaderNumericFloat4, "gSoftShadowParams");
    mLightPosCamSpace =
        cbuffer.AddConstant(kShaderNumericFloat3, "gLightPosCamSpace");
    mLightClipPlane =
        cbuffer.AddConstant(kShaderNumericFloat4, "gLightClipPlane");
    mCamToLightXfm =
        cbuffer.AddConstant(kShaderNumericFloat3x4, "gCamToLightXfm");
    mShadowMapIndex =
        cbuffer.AddConstant(kShaderNumericFloat, "gShadowMapIndex");

    mShadowMap = resources.AddTexture(
        "gShadowMap",
        "gShadowMapSampler",
        RndTextureBase::kTextureArray2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTiledClassificationBuffer = resources.AddTextureWritable(
        "gTiledClassificationBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat);
}

const char* RndShaderLightSpotShadowGen::_GetClassNameImpl() const {
    return "RndShaderLightSpotShadowGen";
}
