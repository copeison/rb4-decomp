#include "render/postprocessing/depth_of_field/RndCShaderDOFDiscBlur.h"

#include "render/buffers/RndComputeBuffer.h"
#include "render/context/RndContext.h"
#include "render/drawing/RndSceneDrawParams.h"
#include "render/system/RndConfig.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTextureArray1D.h"
#include "render/textures/RndTextureBase.h"
#include "utl/text/Symbol.h"

namespace {

// Name not in the reference map.
int CeilDiv(int value, int divisor) {
    const int quotient = value / divisor;
    return quotient * divisor < value ? quotient + 1 : quotient;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6F2B40.
RndCShaderDOFDiscBlur::RndCShaderDOFDiscBlur()
    : mSceneTex(-1),
      mNormalizedDepthTex(-1),
      mSceneMask(-1),
      mFunctionTable(-1),
      mOutputSceneTex(-1),
      mSprites(-1),
      mTextureSize(-1),
      mFalloffParams(-1),
      mBlurParams(-1),
      mOverbrightLuminance(-1),
      mCBufferSize(0) {}

// Reconstructed from eboot.elf at 0x6F2B90.
RndCShaderDOFDiscBlur::~RndCShaderDOFDiscBlur() {}

// Reconstructed from eboot.elf at 0x6F32F0.
const char* RndCShaderDOFDiscBlur::_GetClassNameImpl() const {
    return "RndCShaderDOFDiscBlur";
}

// Reconstructed from eboot.elf at 0x6F30B0.
const char* RndCShaderDOFDiscBlur::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/DOFDiscBlur.hlsl";
}

// Reconstructed from eboot.elf at 0x6F3300. The compute base's body,
// emitted again for this class.
int RndCShaderDOFDiscBlur::_GetShaderStages() const {
    return kShaderStagesCompute;
}

// Reconstructed from eboot.elf at 0x6F30C0.
void RndCShaderDOFDiscBlur::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mSceneTex = resources.AddTexture(
        "gSceneTex",
        "gSceneTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mNormalizedDepthTex = resources.AddTexture(
        "gNormalizedDepthTex",
        "gNormalizedDepthTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSceneMask = resources.AddTexture(
        "gSceneMask",
        "gSceneMaskSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mFunctionTable = resources.AddTexture(
        "gFunctionTable",
        "gFunctionTableSampler",
        RndTextureBase::kTextureArray1D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mOutputSceneTex = resources.AddTextureWritable(
        "gOutputSceneTex",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    // Buffer usage 1.
    mSprites = resources.AddComputeBufferCustomTypedWritable(
        "gSprites", "CSBokehSprite", 1, kShaderProgramCompute);

    mTextureSize = cbuffer.AddConstant(kShaderNumericFloat4, "gTextureSize");
    mFalloffParams =
        cbuffer.AddConstant(kShaderNumericFloat3, "gFalloffParams");
    mBlurParams = cbuffer.AddConstant(kShaderNumericFloat2, "gBlurParams");
    mOverbrightLuminance =
        cbuffer.AddConstant(kShaderNumericFloat, "gOverbrightLuminance");
    mCBufferSize = cbuffer.mSize;

    // Without device settings the tile size defaults to 32.
    auto* device = TheRndDevice();
    const auto* settings = device != nullptr ? device->mSettings : nullptr;
    const int tileSize = settings != nullptr
        ? static_cast<int>(settings->mLightTileSize)
        : 32;
    fixedDefines.Add(Symbol("HX_MAX_RADIUS"), tileSize / 2);
    fixedDefines.Add(Symbol("HX_TILE_SIZE"), tileSize);
}

// Reconstructed from eboot.elf at 0x6F2BC0. The blur radius is authored for
// a 1080-line target and scales with the collection's height.
void RndCShaderDOFDiscBlur::Dispatch(
    RndContext& context,
    RndBufferCollection& buffers,
    const RndSceneDrawTarget& target,
    RndComputeBuffer& sprites,
    const Params& params) {
    constexpr float kReferenceHeight = 1080.0F;
    // The falloff scale when the near and far distances coincide.
    constexpr float kDegenerateFalloffScale = 10000.0F;

    static Symbol sStatName;
    if (sStatName == Symbol()) {
        sStatName = Symbol("DOF Disc Blur");
    }
    RndScopedGpuStatBlock statBlock(context, sStatName.Str());

    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);

    auto* device = TheRndDevice();
    auto& frame = buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval];
    const auto lightAccum = [&buffers, &frame](unsigned long index) {
        return buffers.mActiveSceneContext != 0 ? buffers.mLightAccum[index]
                                                : frame.mPartialLightAccum;
    };
    lightAccum(target.mSrcLightAccum)
        ->Select(context, kShaderProgramCompute, mSceneTex, 0, 0);
    frame.mLinearDepth->Select(
        context, kShaderProgramCompute, mNormalizedDepthTex, 0, 0);
    RndTextureBase* sceneMask = params.mUseSceneMask
        ? buffers.mTiledSceneMask[1]
        : device->mDefaults.mTextures2D[kDefaultTextureZero];
    sceneMask->Select(context, kShaderProgramCompute, mSceneMask, 0, 0);
    device->mShaderMgr.mFunctionTable->Select(
        context, kShaderProgramCompute, mFunctionTable, 0, 0);
    lightAccum(target.mDstLightAccum)
        ->Select(
            context,
            kShaderProgramCompute,
            mOutputSceneTex,
            RndShaderResource::kSelectReadWrite,
            0);
    sprites.Select(
        context,
        kShaderProgramCompute,
        mSprites,
        RndShaderResource::kSelectReadWrite,
        0);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    const auto width = static_cast<float>(buffers.mSize.x);
    const auto height = static_cast<float>(buffers.mSize.y);
    auto* textureSize = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mTextureSize));
    textureSize[0] = width;
    textureSize[1] = height;
    textureSize[2] = 1.0F / width;
    textureSize[3] = 1.0F / height;

    // Maps the normalized depth onto [0, 1] between the near and far blur
    // distances.
    const auto range = params.mFarBlur - params.mNearBlur;
    const auto falloffScale =
        range != 0.0F ? 1.0F / range : kDegenerateFalloffScale;
    auto* falloff = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mFalloffParams));
    falloff[0] = falloffScale;
    falloff[1] = -(params.mNearBlur * falloffScale);
    falloff[2] = static_cast<float>(params.mBlurFalloffFunction);

    auto* blur = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mBlurParams));
    blur[0] = height * (1.0F / kReferenceHeight) * params.mBlurRadius;
    blur[1] = params.mBokehOverbrightScale;
    *static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mOverbrightLuminance)) =
        params.mOverbrightLuminance;
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    const auto tileSize = static_cast<int>(device->mSettings->mLightTileSize);
    context._DispatchComputeImpl(
        static_cast<unsigned int>(CeilDiv(buffers.mSize.x, tileSize)),
        static_cast<unsigned int>(CeilDiv(buffers.mSize.y, tileSize)),
        1);
}
