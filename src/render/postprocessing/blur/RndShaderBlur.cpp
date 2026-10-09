#include "render/postprocessing/blur/RndShaderBlur.h"

#include "render/system/RndConfig.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x634AE0. The constructor leaves the
// scene-mask index unset; _InitConfigImpl assigns it.
RndShaderBlur::RndShaderBlur()
    : mIsTexArray{},
      mNumBlurSamples{},
      mBlurType{},
      mBlurDirection{},
      mUseSceneMask{},
      mUseClassificationBuffer{},
      mArraySlice(-1),
      mBlurSampleOffsetsWeights(-1),
      mMaxOffset(-1),
      mTileInfo(-1),
      mCBufferSize(0),
      mTexture(-1),
      mTexArray(-1),
      mTiledClassificationBuffer(-1) {}

// Reconstructed from eboot.elf at 0x634B80.
RndShaderBlur::~RndShaderBlur() {}

// Reconstructed from eboot.elf at 0x635FB0.
const char* RndShaderBlur::_GetClassNameImpl() const {
    return "RndShaderBlur";
}

// Reconstructed from eboot.elf at 0x635AF0.
const char* RndShaderBlur::_GetShaderFilePath() const {
    return "../../system/data/shaders/Blur.hlsl";
}

// Reconstructed from eboot.elf at 0x635B00.
void RndShaderBlur::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    auto& pixel = defines.GetDefines(kShaderProgramPixel);
    mIsTexArray = pixel.AddBool(Symbol("HX_IS_TEX_ARRAY"));
    mNumBlurSamples = pixel.Add(Symbol("HX_NUM_BLUR_SAMPLES"), 0, 33);
    mBlurType = pixel.Add(Symbol("HX_BLUR_TYPE"), 0, 2);
    mBlurDirection = pixel.Add(Symbol("HX_BLUR_DIRECTION"), 0, 2);
    mUseSceneMask = pixel.AddBool(Symbol("HX_USE_SCENE_MASK"));
    mUseClassificationBuffer =
        pixel.AddBool(Symbol("HX_USE_CLASSIFICATION_BUFFER"));

    fixedDefines.Add(Symbol("HX_BLUR_TYPE_GAUSSIAN"), 0);
    fixedDefines.Add(Symbol("HX_BLUR_TYPE_DEPTH_AWARE"), 1);
    fixedDefines.Add(Symbol("HX_BLUR_DIRECTION_HORIZONTAL"), 0);
    fixedDefines.Add(Symbol("HX_BLUR_DIRECTION_VERTICAL"), 1);
    auto* device = TheRndDevice();
    const auto* settings = device != nullptr ? device->mSettings : nullptr;
    fixedDefines.Add(
        Symbol("HX_TILE_SIZE"),
        static_cast<int>(
            settings != nullptr ? settings->mLightTileSize
                                : RndConfig::kDefaultLightTileSize));

    mArraySlice = cbuffer.AddConstant(kShaderNumericFloat, "gArraySlice");
    mBlurSampleOffsetsWeights = cbuffer.AddConstantArray(
        kShaderNumericFloat3, 32, "gBlurSampleOffsetsWeights");
    mMaxOffset = cbuffer.AddConstant(kShaderNumericFloat4, "gMaxOffset");
    mTileInfo = cbuffer.AddConstant(kShaderNumericFloat3, "gTileInfo");
    mCBufferSize = cbuffer.mSize;

    mTexture = resources.AddTexture(
        "gTexture",
        "gTexSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTexArray = resources.AddTexture(
        "gTexArray",
        "gTexArraySampler",
        RndTextureBase::kTextureArray2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mSceneMask = resources.AddTexture(
        "gSceneMask",
        "gSceneMaskSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
    mTiledClassificationBuffer = resources.AddTexture(
        "gTiledClassificationBuffer",
        "gTiledClassificationSampler",
        RndTextureBase::kTexture2D,
        kShaderProgramPixel,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x635F00. Pixel programs need a
// power-of-two sample count of at least two, and only the depth-aware blur
// reads the classification buffer.
bool RndShaderBlur::_UsesShaderKeyImpl(
    RndShaderProgramType type,
    RndShaderKey key) const {
    constexpr unsigned int kDepthAwareBlur = 1;
    if (type != kShaderProgramPixel) {
        return true;
    }
    const auto samples = mNumBlurSamples.GetValue(key);
    if (samples < 2 || (samples & (samples - 1)) != 0) {
        return false;
    }
    return mBlurType.GetValue(key) == kDepthAwareBlur ||
        mUseClassificationBuffer.GetValue(key) == 0;
}
