#include "render/lighting/fog/RndShaderFogDeferred.h"

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/targets/RndBufferCollection.h"
#include "render/textures/RndTextureBase.h"
#include "render/textures/RndTextureArray1D.h"

namespace {

constexpr unsigned long kPixelKey = 3;
constexpr unsigned int kHdr10Output = 1;

}  // namespace

// Reconstructed from eboot.elf at 0x452CA0. The owner registers the shader.
RndShaderFogDeferred::RndShaderFogDeferred()
    : mBT709ToBT2020{},
      mFalloffParams(-1),
      mSkyTex(-1),
      mLinearDepthMap(-1),
      mFunctionTable(-1) {}

RndShaderFogDeferred::~RndShaderFogDeferred() {}

// Reconstructed from eboot.elf at 0x452D20.
void RndShaderFogDeferred::Select(RndContext& context, Params& params) {
    RndDevice* const device = TheRndDevice();
    params.mSkyTexture->Select(context, kShaderProgramPixel, mSkyTex, 0, 0);
    const RndBufferCollection& buffers = *params.mBuffers;
    buffers.mFrameIntervals.mData[buffers.mActiveFrameInterval]
        .mLinearDepth->Select(
            context, kShaderProgramPixel, mLinearDepthMap, 0, 0);
    device->mShaderMgr.mFunctionTable->Select(
        context, kShaderProgramPixel, mFunctionTable, 0, 0);
    RndShaderKeyGroup keys{};
    keys.mKeys[kPixelKey] = mBT709ToBT2020.SetValue(
        0, device->mHdrOutputMode == kHdr10Output ? 1U : 0U);
    _SelectShaderCollection(context, keys);
}

const char* RndShaderFogDeferred::_GetClassNameImpl() const {
    return "RndShaderFogDeferred";
}

const char* RndShaderFogDeferred::_GetShaderFilePath() const {
    return "../../system/data/shaders/FogDeferred.hlsl";
}

void RndShaderFogDeferred::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mBT709ToBT2020 = defines.GetDefines(kShaderProgramPixel)
                         .AddBool(Symbol("HX_BT709_TO_BT2020"));
    mFalloffParams = cbuffer.AddConstant(kShaderNumericFloat3, "gFalloffParams");
    mSkyTex = resources.AddTexture(
        "gSkyTex",
        "gSkyTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mLinearDepthMap = resources.AddTexture(
        "gLinearDepthMap",
        "gLinearDepthMapSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mFunctionTable = resources.AddTexture(
        "gFunctionTable",
        "gFunctionTableSampler",
        RndTextureBase::kTextureArray1D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}
