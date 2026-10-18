#include "render/lighting/ambient_occlusion/RndCShaderSSAOGen.h"

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTexture2D.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Thread-group widths from the shared compute header, set by the static
// initializer at 0x6D7730; only the 2D width is used. Names not in the
// reference map.
[[maybe_unused]] int gSSAOGenGroupSizeUnused = -1;  // 0x1AB1890
int gSSAOGenGroupSize2D = 8;                        // 0x1AB1894
[[maybe_unused]] int gSSAOGenGroupSize3D = 4;       // 0x1AB1898

int CeilDiv(int count, int divisor) {
    const int quotient = count / divisor;
    return quotient * divisor < count ? quotient + 1 : quotient;
}

}  // namespace

// Reconstructed from eboot.elf at 0x6D7130. The constructor leaves the
// buffer size unset; _InitConfigImpl assigns it.
RndCShaderSSAOGen::RndCShaderSSAOGen()
    : mLinearDepthBuffer(-1),
      mGBufferNormal(-1),
      mNoiseTex(-1),
      mSceneMask(-1),
      mOutputBuffer(-1),
      mDimensions(-1),
      mParams(-1) {}

// Reconstructed from eboot.elf at 0x6D7180.
RndCShaderSSAOGen::~RndCShaderSSAOGen() {}

// Reconstructed from eboot.elf at 0x6D71B0.
RndCShaderSSAOGen::Params::Params()
    : mBuffers(nullptr),
      mNoiseTexture(nullptr),
      mCamera(nullptr),
      mAngleBias(0.2F),
      mRadius(50.0F),
      mIntensity(1.0F),
      mUseSceneMask(false) {}

// Reconstructed from eboot.elf at 0x6D71E0. The camera constants come from
// the parameters' camera for the duration of the dispatch.
void RndCShaderSSAOGen::Dispatch(RndContext& context, Params& params) {
    RndShaderKeyGroup keys{};
    _SelectShaderCollection(context, keys);

    const RndCameraContext* const camera = context.mCameraCBufferOverride;
    context.SetCameraCBufferOverrideContext(params.mCamera);

    RndBufferCollection& buffers = *params.mBuffers;
    auto& frame = buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval];
    frame.mLinearDepth->Select(
        context, kShaderProgramCompute, mLinearDepthBuffer, 0, 0);
    frame.mGBufferPixelNormals->Select(
        context, kShaderProgramCompute, mGBufferNormal, 0, 0);
    params.mNoiseTexture->Select(
        context, kShaderProgramCompute, mNoiseTex, 0, 0);
    frame.mAO->Select(
        context,
        kShaderProgramCompute,
        mOutputBuffer,
        RndShaderResource::kSelectReadWrite,
        0);
    RndTextureBase* const sceneMask = params.mUseSceneMask
        ? buffers.mTiledSceneMask[1]
        : TheRndDevice()->mDefaults.mTextures2D[kDefaultTextureZero];
    sceneMask->Select(context, kShaderProgramCompute, mSceneMask, 0, 0);

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    auto* dimensions = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mDimensions));
    dimensions[0] = static_cast<float>(buffers.mSize.x);
    dimensions[1] = static_cast<float>(buffers.mSize.y);
    dimensions[2] = 1.0F / static_cast<float>(buffers.mSize.x);
    dimensions[3] = 1.0F / static_cast<float>(buffers.mSize.y);
    auto* constants = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mParams));
    constants[0] = params.mRadius;
    constants[1] = 1.0F / params.mRadius;
    constants[2] = params.mAngleBias;
    constants[3] = params.mIntensity;
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    const auto& size = frame.mAO->mBaseDesc;
    context._DispatchComputeImpl(
        static_cast<unsigned int>(
            CeilDiv(static_cast<int>(size.mWidth), gSSAOGenGroupSize2D)),
        static_cast<unsigned int>(
            CeilDiv(static_cast<int>(size.mHeight), gSSAOGenGroupSize2D)),
        1);

    context.SetCameraCBufferOverrideContext(camera);
}

// Reconstructed from eboot.elf at 0x6D7560.
const char* RndCShaderSSAOGen::_GetShaderFilePath() const {
    return "../../system/data/shaders/compute/SSAOGen.hlsl";
}

// Reconstructed from eboot.elf at 0x6D7570. Reads the device's settings
// without a null check.
void RndCShaderSSAOGen::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup&,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mLinearDepthBuffer = resources.AddTexture(
        "gLinearDepthBuffer",
        "gLinearDepthBufferSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mGBufferNormal = resources.AddTexture(
        "gGBufferNormal",
        "gGBufferNormalSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mNoiseTex = resources.AddTexture(
        "gNoiseTex",
        "gNoiseTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mSceneMask = resources.AddTexture(
        "gSceneMask",
        "gSceneMaskSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mOutputBuffer = resources.AddTextureWritable(
        "gOutputBuffer",
        RndTextureBase::kTexture2D,
        kShaderProgramCompute,
        kShaderNumericFloat4);
    mDimensions = cbuffer.AddConstant(kShaderNumericFloat4, "gDimensions");
    mParams = cbuffer.AddConstant(kShaderNumericFloat4, "gParams");
    mCBufferSize = cbuffer.mSize;

    const auto& settings = *TheRndDevice()->mSettings;
    fixedDefines.Add(
        Symbol("HX_TILE_SIZE"), static_cast<int>(settings.mLightTileSize));
}

// Reconstructed from eboot.elf at 0x6D7710.
const char* RndCShaderSSAOGen::_GetClassNameImpl() const {
    return "RndCShaderSSAOGen";
}

// Reconstructed from eboot.elf at 0x6D7720. The compute base's body,
// emitted again for this class.
int RndCShaderSSAOGen::_GetShaderStages() const {
    return kShaderStagesCompute;
}
