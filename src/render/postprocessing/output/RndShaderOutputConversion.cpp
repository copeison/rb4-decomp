#include "render/postprocessing/output/RndShaderOutputConversion.h"

#include <cstring>

#include "math/color/Color.h"
#include "render/shaders/shader_draw_state.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x6367A0.
RndShaderOutputConversion::RndShaderOutputConversion()
    : mUseHmdMask{},
      mUseBT709ToBT2020{},
      mUsePerceptualQuantizer{},
      mMinIntensity(-1),
      mCBufferSize(0),
      mSrcTex(-1),
      mHmdMaskTex(-1) {}

// Reconstructed from eboot.elf at 0x636810.
RndShaderOutputConversion::~RndShaderOutputConversion() {}

// Reconstructed from eboot.elf at 0x636C40.
const char* RndShaderOutputConversion::_GetClassNameImpl() const {
    return "RndShaderOutputConversion";
}

// Reconstructed from eboot.elf at 0x636A50.
const char* RndShaderOutputConversion::_GetShaderFilePath() const {
    return "../../system/data/shaders/OutputConversion.hlsl";
}

// Reconstructed from eboot.elf at 0x636A60. The HMD-mask permutation is a
// global define; the color-space and transfer-function permutations are
// pixel defines.
void RndShaderOutputConversion::_InitConfigImpl(
    RndShaderFixedDefines&,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    mUseHmdMask = defines.GetGlobalDefines().AddBool(Symbol("HX_USE_HMD_MASK"));
    auto& pixel = defines.GetDefines(kShaderProgramPixel);
    mUseBT709ToBT2020 = pixel.AddBool(Symbol("HX_USE_BT709_TO_BT2020"));
    mUsePerceptualQuantizer =
        pixel.AddBool(Symbol("HX_USE_PERCEPTUAL_QUANTIZER"));

    mMinIntensity = cbuffer.AddConstant(kShaderNumericFloat, "gMinIntensity");
    mCBufferSize = cbuffer.mSize;
    mSrcTex = resources.AddTexture(
        "gSrcTex",
        "gSrcTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mHmdMaskTex = resources.AddTexture(
        "gHmdMaskTex",
        "gHmdMaskTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x636BF0. HMD-mask permutations are never
// built, on any program type.
bool RndShaderOutputConversion::_UsesShaderKeyImpl(
    RndShaderProgramType,
    RndShaderKey key) const {
    return mUseHmdMask.GetValue(key) == 0;
}

// Reconstructed from eboot.elf at 0x636840. Uploads the linearized minimum
// intensity before binding the source and optional HMD mask with binding flag
// 2. The HMD-mask permutation is global, so it is written into every program
// key; the color-space and transfer-function permutations are pixel-only.
void RndShaderOutputConversion::Select(
    RndContext& context,
    const Params& params) {
    constexpr unsigned long kPixelKey = 3;
    constexpr unsigned int kTextureFlags = 2;

    auto& buffer =
        rb4::render_shader_select_constant_buffer(context, mCBufferSize);
    const Hmx::Color intensity(
        params.mMinimumIntensity,
        params.mMinimumIntensity,
        params.mMinimumIntensity,
        1.0F);
    Hmx::Color linear(0.0F, 0.0F, 0.0F, 1.0F);
    GammaToLinear_sRGB(intensity, linear);
    std::memcpy(
        rb4::render_shader_constant_member(buffer, mMinIntensity),
        &linear.red,
        sizeof(linear.red));
    rb4::render_shader_commit_constant_buffer(buffer, context, mCBufferSize);

    rb4::render_shader_bind_pixel_texture(
        context, params.mSource, mSrcTex, kTextureFlags);
    rb4::render_shader_bind_pixel_texture(
        context, params.mHmdMask, mHmdMaskTex, kTextureFlags);

    const auto globalField = static_cast<RndShaderKey>(
        ((params.mHmdMask != nullptr ? 1U : 0U) -
         static_cast<unsigned int>(mUseHmdMask.mFirst))
        << mUseHmdMask.mShift) << 32;
    RndShaderKeyGroup keys;
    for (auto& key : keys.mKeys) {
        key = globalField;
    }
    keys.mKeys[kPixelKey] = mUseBT709ToBT2020.SetValue(
        globalField, params.mBT709ToBT2020 ? 1U : 0U);
    keys.mKeys[kPixelKey] = mUsePerceptualQuantizer.SetValue(
        keys.mKeys[kPixelKey], params.mPerceptualQuantizer ? 1U : 0U);
    _SelectShaderCollection(context, keys);
}
