#include "render/shaders/RndShaderBasic.h"

#include <cstring>

#include "render/context/RndContext.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderDrawUtl.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x6398D0.
RndShaderBasic::RndShaderBasic()
    : mShadingMode{},
      mTextureMode{},
      mAlphaCut{},
      mUseTexRedAsAlpha{},
      mColor(-1),
      mCBufferSize(0),
      mTexture2D(-1),
      mTexture2DRTSliced(-1) {}

// Reconstructed from eboot.elf at 0x639950.
RndShaderBasic::~RndShaderBasic() {}

// Reconstructed from eboot.elf at 0x639F00.
const char* RndShaderBasic::_GetClassNameImpl() const {
    return "RndShaderBasic";
}

// Reconstructed from eboot.elf at 0x639BD0.
const char* RndShaderBasic::_GetShaderFilePath() const {
    return "../../system/data/shaders/Basic.hlsl";
}

// Reconstructed from eboot.elf at 0x639BE0.
void RndShaderBasic::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    auto& pixel = defines.GetDefines(kShaderProgramPixel);
    mShadingMode = pixel.Add(Symbol("HX_SHADING_MODE"), 0, 19);
    mTextureMode = pixel.Add(Symbol("HX_TEXTURE_MODE"), 0, 3);
    mAlphaCut = pixel.AddBool(Symbol("HX_ALPHA_CUT"));
    mUseTexRedAsAlpha = pixel.AddBool(Symbol("HX_USE_TEX_RED_AS_ALPHA"));

    fixedDefines.Add(Symbol("HX_TEXTURE_MODE_NONE"), 0);
    fixedDefines.Add(Symbol("HX_TEXTURE_MODE_2D"), 1);
    fixedDefines.Add(Symbol("HX_TEXTURE_MODE_2D_RTSLICED"), 2);

    mColor = cbuffer.AddConstant(kShaderNumericFloat4, "gColor");
    mCBufferSize = cbuffer.mSize;
    mTexture2D = resources.AddTexture(
        "gTexture2D",
        "gTex2DSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTexture2DRTSliced = resources.AddTexture2DRTSliced(
        "gTexture2DRTSliced",
        "gTex2DRTSlicedSampler",
        RndTextureBase::kTexture2D,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x639E40. Pixel programs exist only for
// shading modes 0, 16, and 17; mode 17 has no alpha cut, and red-as-alpha
// needs a texture.
bool RndShaderBasic::_UsesShaderKeyImpl(
    RndShaderProgramType type,
    RndShaderKey key) const {
    if (type != kShaderProgramPixel) {
        return true;
    }
    const auto shadingMode = mShadingMode.GetValue(key);
    if (shadingMode == 17) {
        if (mAlphaCut.GetValue(key) != 0) {
            return false;
        }
    } else if (shadingMode != 0 && shadingMode != 16) {
        return false;
    }
    if (mTextureMode.GetValue(key) != 0) {
        return true;
    }
    return mUseTexRedAsAlpha.GetValue(key) == 0;
}

// Reconstructed from eboot.elf at 0x639EF0.
bool RndShaderBasic::_SupportsRTSlicing() const {
    return true;
}

// Reconstructed from eboot.elf at 0x639980.
void RndShaderBasic::Select(RndContext& context, const Params& params) {
    constexpr unsigned long kPixelKey = 3;
    auto& cbuffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    std::memcpy(
        RndShaderDrawUtl::GetCBufferMember(cbuffer, mColor),
        &params.mColor,
        sizeof(params.mColor));
    RndShaderDrawUtl::CommitCBuffer(cbuffer, context, mCBufferSize);

    if (params.mTexture != nullptr) {
        params.mTexture->Select(context, kShaderProgramPixel, mTexture2D, 0, 0);
    } else if (params.mRTSlicedTexture != nullptr) {
        params.mRTSlicedTexture->Select(
            context,
            kShaderProgramPixel,
            mTexture2DRTSliced,
            RndShaderResource::kSelectRTSliced,
            0);
    }

    const auto shadingMode = context.mShadingMode;
    RndShaderKeyGroup keys{};
    auto key = mShadingMode.SetValue(0, static_cast<unsigned int>(shadingMode));
    key = mAlphaCut.SetValue(
        key, params.mAlphaCut && shadingMode != kShadingModeWireframe ? 1U : 0U);
    if (params.mTexture != nullptr || params.mRTSlicedTexture != nullptr) {
        key = mTextureMode.SetValue(key, params.mTexture != nullptr ? 1U : 2U);
        key = mUseTexRedAsAlpha.SetValue(key, params.mUseTexRedAsAlpha ? 1U : 0U);
    } else {
        key = mTextureMode.SetValue(key, 0);
    }
    keys.mKeys[kPixelKey] = key;
    _SelectShaderCollection(context, keys);
}
