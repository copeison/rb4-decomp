#include "render/postprocessing/tonemap/RndTonemapShaderBase.h"

#include <cmath>

#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

namespace {

// Names not in the reference map.
constexpr unsigned long kPixelKey = 3;
constexpr unsigned long kComputeKey = 4;
constexpr unsigned int kHdr10Output = 1;
// Keeps the curve's denominator positive.
constexpr float kMaxTonemapCurve = 0.999F;

}  // namespace

// Reconstructed from eboot.elf at 0x4ADC40.
RndTonemapShaderBase::RndTonemapShaderBase()
    : mProgramType(static_cast<RndShaderProgramType>(-1)),
      mBT709ToBT2020{},
      mTonemapOp{},
      mTonemapParams(-1),
      mCBufferSize(0),
      mSourceBuffer(-1),
      mOutputBuffer(-1) {}

RndTonemapShaderBase::~RndTonemapShaderBase() {}

// Reconstructed from eboot.elf at 0x4ADCB0.
void RndTonemapShaderBase::_Select(RndContext& context, const Params& params) {
    RndShaderKeyGroup keys{};
    auto& key = keys.mKeys
        [mProgramType == kShaderProgramPixel ? kPixelKey : kComputeKey];
    key = mBT709ToBT2020.SetValue(
        0, TheRndDevice()->mHdrOutputMode == kHdr10Output ? 1U : 0U);
    key = mTonemapOp.SetValue(key, static_cast<unsigned int>(params.mTonemapOp));
    _SelectShaderCollection(context, keys);

    if (mProgramType == kShaderProgramPixel) {
        params.mSource->Select(context, kShaderProgramPixel, mSourceBuffer, 0, 0);
    } else {
        params.mSource->Select(
            context, kShaderProgramCompute, mSourceBuffer, 0, 0);
        params.mOutput->Select(
            context,
            kShaderProgramCompute,
            mOutputBuffer,
            RndShaderResource::kSelectReadWrite,
            0);
    }

    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    const float first = params.mTonemapParams[0];
    const float second = params.mTonemapParams[1];
    const float curve = second < kMaxTonemapCurve ? second : kMaxTonemapCurve;
    const float exponent = 1.0F / (1.0F - curve);
    const float values[4] = {
        first,
        second,
        (1.0F - std::pow(curve, exponent)) * first,
        1.0F - std::pow(curve, curve * exponent),
    };
    auto* constant = static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mTonemapParams));
    for (unsigned long i = 0; i < 4; ++i) {
        constant[i] = values[i];
    }
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);
}

// Reconstructed from eboot.elf at 0x4ADF50. The defines and the source
// texture belong to the compute program when the derived shader's stages
// include compute, and to the pixel program otherwise.
void RndTonemapShaderBase::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mProgramType = (_GetShaderStages() & kShaderStagesCompute) != 0
        ? kShaderProgramCompute
        : kShaderProgramPixel;
    fixedDefines.Add(Symbol("HX_TONEMAP_OP_KEVIN"), 0);
    fixedDefines.Add(Symbol("HX_TONEMAP_OP_LINEAR"), 1);
    fixedDefines.Add(Symbol("HX_TONEMAP_OP_POWER"), 2);
    fixedDefines.Add(Symbol("HX_TONEMAP_OP_HYBRID"), 3);

    auto& programDefines = defines.GetDefines(mProgramType);
    mBT709ToBT2020 = programDefines.AddBool(Symbol("HX_BT709_TO_BT2020"));
    mTonemapOp = programDefines.Add(Symbol("HX_TONEMAP_OP"), 0, 4);

    mSourceBuffer = resources.AddTexture(
        "gSourceBuffer",
        "gSourceBufferSampler",
        RndTextureBase::kTexture2D,
        mProgramType,
        kShaderNumericFloat4);
    if (mProgramType == kShaderProgramCompute) {
        mOutputBuffer = resources.AddTextureWritable(
            "gOutputBuffer",
            RndTextureBase::kTexture2D,
            kShaderProgramCompute,
            kShaderNumericFloat4);
    }
    mTonemapParams = cbuffer.AddConstant(kShaderNumericFloat4, "gTonemapParams");
    mCBufferSize = cbuffer.mSize;
}
