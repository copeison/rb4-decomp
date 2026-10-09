#include "render/postprocessing/bloom/RndShaderBloom.h"

#include <cstring>

#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x634640.
RndShaderBloom::RndShaderBloom()
    : mSampleHalfSize{},
      mHuePreservation{},
      mBloomParams(-1),
      mOverbrightParams(-1),
      mCBufferSize(0),
      mSrcTex(-1),
      mHalfSizeBloomTex(-1),
      mQtrSizeBloomTex(-1) {}

// Reconstructed from eboot.elf at 0x6346B0.
RndShaderBloom::~RndShaderBloom() {}

// Reconstructed from eboot.elf at 0x634AB0.
const char* RndShaderBloom::_GetClassNameImpl() const {
    return "RndShaderBloom";
}

// Reconstructed from eboot.elf at 0x634910.
const char* RndShaderBloom::_GetShaderFilePath() const {
    return "../../system/data/shaders/Bloom.hlsl";
}

// Reconstructed from eboot.elf at 0x634920.
void RndShaderBloom::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    auto& pixel = defines.GetDefines(kShaderProgramPixel);
    mSampleHalfSize = pixel.AddBool(Symbol("HX_SAMPLE_HALF_SIZE"));
    mHuePreservation = pixel.AddBool(Symbol("HX_HUE_PRESERVATION"));

    mSrcTex = resources.AddTexture(
        "gSrcTex",
        "gSrcTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mHalfSizeBloomTex = resources.AddTexture(
        "gHalfSizeBloomTex",
        "gHalfSizeBloomTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mQtrSizeBloomTex = resources.AddTexture(
        "gQtrSizeBloomTex",
        "gQtrSizeBloomTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);

    mBloomParams = cbuffer.AddConstant(kShaderNumericFloat3, "gBloomParams");
    mOverbrightParams =
        cbuffer.AddConstant(kShaderNumericFloat2, "gOverbrightParams");
    mCBufferSize = cbuffer.mSize;
}

// Reconstructed from eboot.elf at 0x6346E0. Binds the source and both bloom
// textures, uploads the bloom and overbright constants, and selects the
// half-size and hue-preservation permutation on the pixel program.
void RndShaderBloom::Select(RndContext& context, const Params& params) {
    constexpr unsigned long kPixelKey = 3;

    RndShaderDrawUtl::SelectPixelTexture(context, params.mSource, mSrcTex);
    RndShaderDrawUtl::SelectPixelTexture(
        context, params.mHalfSizeBloom, mHalfSizeBloomTex);
    RndShaderDrawUtl::SelectPixelTexture(
        context, params.mQuarterSizeBloom, mQtrSizeBloomTex);

    auto& buffer =
        RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(buffer, mBloomParams),
        params.mBloom,
        sizeof(params.mBloom));
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(buffer, mOverbrightParams),
        params.mOverbright,
        sizeof(params.mOverbright));
    RndShaderDrawUtl::CommitCBuffer(buffer, context, mCBufferSize);

    RndShaderKeyGroup keys{};
    keys.mKeys[kPixelKey] = mSampleHalfSize.SetValue(
        0, params.mHalfSizeBloom != nullptr ? 1U : 0U);
    keys.mKeys[kPixelKey] = mHuePreservation.SetValue(
        keys.mKeys[kPixelKey], params.mHuePreservation ? 1U : 0U);
    _SelectShaderCollection(context, keys);
}
