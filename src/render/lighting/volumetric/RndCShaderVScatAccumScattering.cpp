#include "render/lighting/volumetric/RndCShaderVScatAccumScattering.h"

#include "render/core/settings/render_settings.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x6D1E70.
RndCShaderVScatAccumScattering::RndCShaderVScatAccumScattering()
    : mUseSceneMask{},
      mIsStereo{},
      mStereoEye{},
      mDensityInscatteringTex(-1),
      mTiledDepthRangeBuffer(-1),
      mSceneMask(-1),
      mOutputTex(-1),
      mSrcDimensions(-1),
      mDstDimensions(-1),
      mScreenDimensions(-1),
      mDepthFracOffsetParams(-1),
      mFrustumParams(-1),
      mCBufferSize(0) {}

RndCShaderVScatAccumScattering::~RndCShaderVScatAccumScattering() {}

const char* RndCShaderVScatAccumScattering::_GetClassNameImpl() const {
    return "RndCShaderVScatAccumScattering";
}

const char* RndCShaderVScatAccumScattering::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/VScatAccumScattering.hlsl";
}

void RndCShaderVScatAccumScattering::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    static const rb4::RenderSettings sDefaultSettings{};
    auto* device = TheRndDevice();
    const auto* settings = device != nullptr ? device->mSettings : nullptr;
    fixedDefines.Add(
        Symbol("HX_TILE_SIZE"),
        static_cast<int>(
            (settings != nullptr ? *settings : sDefaultSettings).light_tile_size));

    auto& compute = defines.GetDefines(kShaderProgramCompute);
    mUseSceneMask = compute.AddBool(Symbol("HX_USE_SCENE_MASK"));
    mIsStereo = compute.AddBool(Symbol("HX_IS_STEREO"));
    mStereoEye = compute.Add(Symbol("HX_STEREO_EYE"), 0, 2);

    mDensityInscatteringTex = resources.AddTexture(
        "gDensityInscatteringTex",
        "gDensityInscatteringTexSampler",
        RndTextureBase::kTexture3D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mTiledDepthRangeBuffer = resources.AddTexture(
        "gTiledDepthRangeBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSceneMask = resources.AddTexture(
        "gSceneMask",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mOutputTex = resources.AddTextureWritable(
        "gOutputTex",
        RndTextureBase::kTexture3D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSrcDimensions =
        cbuffer.AddConstant(kShaderNumericFloat3, "gSrcDimensions");
    mDstDimensions =
        cbuffer.AddConstant(kShaderNumericFloat3, "gDstDimensions");
    mScreenDimensions =
        cbuffer.AddConstant(kShaderNumericFloat2, "gScreenDimensions");
    mDepthFracOffsetParams =
        cbuffer.AddConstant(kShaderNumericFloat4, "gDepthFracOffsetParams");
    mFrustumParams = cbuffer.AddConstant(kShaderNumericFloat4, "gFrustumParams");
    mCBufferSize = cbuffer.mSize;
}
