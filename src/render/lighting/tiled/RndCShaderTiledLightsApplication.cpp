#include "render/lighting/tiled/RndCShaderTiledLightsApplication.h"

#include "render/buffers/RndComputeBuffer.h"
#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndContext.h"
#include "render/lighting/tiled/RndTiledLightsComputeBuffer.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTextureArray1D.h"
#include "render/textures/RndTextureBase.h"
#include "utl/text/MakeString.h"
#include "utl/text/Symbol.h"

namespace {

// Names not in the reference map.
constexpr unsigned long kComputeKey = 4;
constexpr unsigned int kHdr10Output = 1;
constexpr unsigned int kNoSampler = RndShaderResource::kSelectNoSampler;

void SelectCompute(
    RndContext& context,
    RndShaderResource& resource,
    unsigned long slot,
    unsigned int flags = 0) {
    resource.Select(context, kShaderProgramCompute, slot, flags, 0);
}

RndTextureBase* DefaultTexture(
    RndTextureBase::Type type,
    RndDefaultTextureType defaultType) {
    return TheRndDevice()->mDefaults.GetTexture(type, defaultType);
}

RndTextureBase* OrDefault(
    RndTextureBase* texture,
    RndTextureBase::Type type,
    RndDefaultTextureType defaultType) {
    return texture != nullptr ? texture : DefaultTexture(type, defaultType);
}

}  // namespace

// Reconstructed from eboot.elf at 0x6D7750.
RndCShaderTiledLightsApplication::RndCShaderTiledLightsApplication()
    : mBT709ToBT2020{},
      mInterpolateTiledLighting{},
      mShadingMode{},
      mCookies2D(-1),
      mCookies2DRendered(-1),
      mCookiesCube(-1),
      mProbeDiffuseTextures(-1),
      mProbeSpecularTextures(-1),
      mAmbientOcclusion(-1),
      mFunctionTable(-1),
      mPointLights(-1),
      mSpotLights(-1),
      mDirectionalLights(-1),
      mLightProbes(-1),
      mLightIds(-1),
      mLightIdRanges(-1),
      mSrcLightAccumBuffer(-1),
      mGBuffers{static_cast<unsigned long>(-1),
                static_cast<unsigned long>(-1),
                static_cast<unsigned long>(-1)},
      mLinearDepthBuffer(-1),
      mStencilBuffer(-1),
      mShadowMaps(-1),
      mDstLightAccumBuffer(-1),
      mLightInterpBuffer(-1),
      mCBufferSize(0),
      mDimensions(-1),
      mTileCounts(-1),
      mNumDirectionalLights(-1),
      mProbeIntensityMult(-1) {}

RndCShaderTiledLightsApplication::~RndCShaderTiledLightsApplication() {}

// Reconstructed from eboot.elf at 0x6D7840. The function table is bound
// twice, as the binary does.
void RndCShaderTiledLightsApplication::Dispatch(
    RndContext& context,
    Params& params) {
    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("Tiled Lights Application");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    auto* device = TheRndDevice();
    const auto* settings = device->mSettings;
    RndShaderKeyGroup keys{};
    auto key = mBT709ToBT2020.SetValue(
        0, device->mHdrOutputMode == kHdr10Output ? 1U : 0U);
    key = mInterpolateTiledLighting.SetValue(
        key, settings->mTiledLightInterpolationEnabled ? 1U : 0U);
    keys.mKeys[kComputeKey] = mShadingMode.SetValue(
        key, static_cast<unsigned int>(context.mShadingMode));
    _SelectShaderCollection(context, keys);

    const auto* previousCamera =
        reinterpret_cast<const RndCameraContext*>(context.mUnknown18776);
    context.SetCameraCBufferOverrideContext(params.mCamera);

    auto& buffers = *params.mBuffers;
    const int tileSize = static_cast<int>(settings->mLightTileSize);
    const int width = buffers.mSize.x;
    const int height = buffers.mSize.y;
    const int tilesX = width / tileSize + (width / tileSize * tileSize < width ? 1 : 0);
    const int tilesY = height / tileSize + (height / tileSize * tileSize < height ? 1 : 0);

    SelectCompute(
        context,
        *OrDefault(
            params.mCookies2D,
            RndTextureBase::kTextureArray2D,
            kDefaultTextureError),
        mCookies2D);
    SelectCompute(
        context,
        *OrDefault(
            params.mCookies2DRendered,
            RndTextureBase::kTextureArray2D,
            kDefaultTextureError),
        mCookies2DRendered);
    SelectCompute(
        context,
        *OrDefault(
            params.mCookiesCube,
            RndTextureBase::kTextureArrayCube,
            kDefaultTextureError),
        mCookiesCube);
    SelectCompute(
        context,
        *OrDefault(
            params.mProbeDiffuseTextures,
            RndTextureBase::kTextureArrayCube,
            kDefaultTextureError),
        mProbeDiffuseTextures);
    SelectCompute(
        context,
        *OrDefault(
            params.mProbeSpecularTextures,
            RndTextureBase::kTextureArrayCube,
            kDefaultTextureError),
        mProbeSpecularTextures);
    SelectCompute(context, *params.mAmbientOcclusion, mAmbientOcclusion);
    SelectCompute(context, *device->mShaderMgr.mFunctionTable, mFunctionTable);
    auto* lights = params.mLightBuffers;
    SelectCompute(context, *lights[kTiledLightsPoint].mBuffer, mPointLights);
    SelectCompute(context, *lights[kTiledLightsSpot].mBuffer, mSpotLights);
    SelectCompute(
        context, *lights[kTiledLightsDirectional].mBuffer, mDirectionalLights);
    SelectCompute(context, *params.mLightProbes, mLightProbes);

    auto& frame = buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval];
    SelectCompute(context, *frame.mTiledLightIds[0], mLightIds);
    SelectCompute(context, *frame.mTiledLightIdRanges, mLightIdRanges);
    SelectCompute(
        context, *params.mSrcLightAccumBuffer, mSrcLightAccumBuffer, kNoSampler);
    RndTextureBase* const gbuffers[3] = {
        frame.mGBufferColor,
        frame.mGBufferPixelNormals,
        frame.mGBufferVertexNormals,
    };
    for (unsigned long i = 0; i < 3; ++i) {
        if (gbuffers[i] != nullptr) {
            SelectCompute(context, *gbuffers[i], mGBuffers[i], kNoSampler);
        }
    }
    SelectCompute(
        context,
        *frame.mLinearDepth,
        mLinearDepthBuffer,
        kNoSampler);
    SelectCompute(
        context,
        *OrDefault(
            buffers.mShadowContribArray,
            RndTextureBase::kTextureArray2D,
            kDefaultTextureWhite),
        mShadowMaps);
    SelectCompute(
        context,
        *frame.mDepthStencil,
        mStencilBuffer,
        RndShaderResource::kSelectStencil | kNoSampler);
    SelectCompute(context, *device->mShaderMgr.mFunctionTable, mFunctionTable);

    auto* dstLightAccum = buffers.mActiveSceneContext != 0
        ? buffers.mLightAccum[params.mSceneContext]
        : frame.mPartialLightAccum;
    SelectCompute(
        context,
        *dstLightAccum,
        mDstLightAccumBuffer,
        RndShaderResource::kSelectReadWrite);
    if (settings->mTiledLightInterpolationEnabled) {
        SelectCompute(
            context,
            *buffers.mFrameIntervals.mData[0].mTiledLightInterp,
            mLightInterpBuffer,
            RndShaderResource::kSelectReadWrite);
    }

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    auto* dimensions = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mDimensions));
    dimensions[0] = static_cast<float>(width);
    dimensions[1] = static_cast<float>(height);
    dimensions[2] = 1.0F / static_cast<float>(width);
    dimensions[3] = 1.0F / static_cast<float>(height);
    auto* tileCounts = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mTileCounts));
    tileCounts[0] = static_cast<float>(tilesX);
    tileCounts[1] = static_cast<float>(tilesY);
    *static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mNumDirectionalLights)) =
        static_cast<float>(lights[kTiledLightsDirectional].mNumPosLights);
    *static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mProbeIntensityMult)) =
        params.mProbeIntensityMult;
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    context._DispatchComputeImpl(
        static_cast<unsigned int>(tilesX), static_cast<unsigned int>(tilesY), 1);
    context.SetCameraCBufferOverrideContext(previousCamera);
}

const char* RndCShaderTiledLightsApplication::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/TiledLightsApplication.hlsl";
}

// Reconstructed from eboot.elf at 0x6D82F0. Reads the device's settings
// without a null check.
void RndCShaderTiledLightsApplication::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    const auto& settings = *TheRndDevice()->mSettings;
    fixedDefines.Add(
        Symbol("HX_TILE_SIZE"), static_cast<int>(settings.mLightTileSize));
    fixedDefines.Add(
        Symbol("HX_TILE_DEPTH_SLICES"),
        static_cast<int>(settings.mLightTileDepthSlices));
    fixedDefines.Add(
        Symbol("HX_MAX_LIGHTS_PER_TILE"),
        static_cast<int>(settings.mMaxLightsPerTile));
    fixedDefines.Add(Symbol("HX_SHADOW_CAST_CONTEXT_GEOMETRY"), 2);

    auto& compute = defines.GetDefines(kShaderProgramCompute);
    mBT709ToBT2020 = compute.AddBool(Symbol("HX_BT709_TO_BT2020"));
    mInterpolateTiledLighting =
        compute.AddBool(Symbol("HX_INTERPOLATE_TILED_LIGHTING"));
    mShadingMode = compute.Add(Symbol("HX_SHADING_MODE"), 0, 19);

    mCookies2D = resources.AddTexture(
        "gCookies2D",
        "gCookies2DSampler",
        RndTextureBase::kTextureArray2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mCookies2DRendered = resources.AddTexture(
        "gCookies2DRendered",
        "gCookies2DRenderedSampler",
        RndTextureBase::kTextureArray2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mCookiesCube = resources.AddTexture(
        "gCookiesCube",
        "gCookiesCubeSampler",
        RndTextureBase::kTextureArrayCube,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mProbeDiffuseTextures = resources.AddTexture(
        "gProbeDiffuseTextures",
        "gProbeDiffuseSampler",
        RndTextureBase::kTextureArrayCube,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mProbeSpecularTextures = resources.AddTexture(
        "gProbeSpecularTextures",
        "gProbeSpecularSampler",
        RndTextureBase::kTextureArrayCube,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mAmbientOcclusion = resources.AddTexture(
        "gAmbientOcclusion",
        "gAmbientOcclusionSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mFunctionTable = resources.AddTexture(
        "gFunctionTable",
        "gFunctionTableSampler",
        RndTextureBase::kTextureArray1D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mShadowMaps = resources.AddTexture(
        "gShadowMaps",
        "gShadowMapsSampler",
        RndTextureBase::kTextureArray2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mPointLights = resources.AddComputeBufferCustomTyped(
        "gPointLights", "CSLightPoint", 0, kShaderProgramCompute);
    mSpotLights = resources.AddComputeBufferCustomTyped(
        "gSpotLights", "CSLightSpot", 0, kShaderProgramCompute);
    mDirectionalLights = resources.AddComputeBufferCustomTyped(
        "gDirectionalLights", "CSLightDirectional", 0, kShaderProgramCompute);
    mLightProbes = resources.AddComputeBufferCustomTyped(
        "gLightProbes", "CSLightProbe", 0, kShaderProgramCompute);
    mLightIds = resources.AddComputeBufferCustomTyped(
        "gLightIds", "Uint2As32", 0, kShaderProgramCompute);
    mLightIdRanges = resources.AddComputeBufferCustomTyped(
        "gLightIdRanges", "CSLightIdRange", 0, kShaderProgramCompute);
    mSrcLightAccumBuffer = resources.AddTexture(
        "gSrcLightAccumBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    for (int i = 0; i < 3; ++i) {
        mGBuffers[i] = resources.AddTexture(
            MakeString("gGBuffer%d", i),
            "",
            RndTextureBase::kTexture2D,
            kShaderProgramCompute,
            kShaderNumericFloat4);
    }
    mLinearDepthBuffer = resources.AddTexture(
        "gLinearDepthBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mStencilBuffer = resources.AddTexture(
        "gStencilBuffer",
        "",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericUInt4);
    mDstLightAccumBuffer = resources.AddTextureWritable(
        "gDstLightAccumBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mLightInterpBuffer = resources.AddTextureWritable(
        "gLightInterpBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);

    mDimensions = cbuffer.AddConstant(kShaderNumericFloat4, "gDimensions");
    mTileCounts = cbuffer.AddConstant(kShaderNumericFloat2, "gTileCounts");
    mNumDirectionalLights =
        cbuffer.AddConstant(kShaderNumericFloat, "gNumDirectionalLights");
    mProbeIntensityMult =
        cbuffer.AddConstant(kShaderNumericFloat, "gProbeIntensityMult");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x6D8990. Compute programs exist only for
// the standard and debug shading modes.
bool RndCShaderTiledLightsApplication::_UsesShaderKeyImpl(
    RndShaderProgramType type,
    RndShaderKey key) const {
    if (type == kShaderProgramCompute) {
        const auto shadingMode = mShadingMode.GetValue(key);
        if (shadingMode != static_cast<unsigned int>(kShadingModeDebugMisc) &&
            shadingMode != static_cast<unsigned int>(kShadingModeStandard)) {
            return false;
        }
    }
    return true;
}

const char* RndCShaderTiledLightsApplication::_GetClassNameImpl() const {
    return "RndCShaderTiledLightsApplication";
}
