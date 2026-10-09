#include "render/postprocessing/blur/RndShaderBlur.h"

#include <cmath>
#include <cstring>

#include "math/vector/Vector4.h"
#include "render/shaders/RndShaderDrawUtl.h"
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

// Reconstructed from eboot.elf at 0x634BB0. Builds a normalized Gaussian
// kernel of up to 31 taps (63 when bilinear filtering lets the Gaussian
// blur fold each pair of taps into one sample), uploads the sample offsets
// along the blur direction with their weights, and selects the permutation.
void RndShaderBlur::Select(RndContext& context, const Params& params) {
    constexpr unsigned long kPixelKey = 3;
    constexpr int kGaussianBlur = 0;
    constexpr int kHorizontal = 0;
    constexpr int kVertical = 1;
    // Point filtering cannot merge two taps into one bilinear sample. The
    // filter-mode value is inferred from this use.
    constexpr unsigned int kFilterPoint = 1;
    constexpr int kMaxSamples = 32;
    constexpr int kMaxTaps = kMaxSamples - 1;
    constexpr int kMaxPairedTaps = kMaxSamples * 2 - 1;
    constexpr float kDefaultRadius = 7.0F;

    auto& source = *params.mSource;
    auto radius = kDefaultRadius;
    if (params.mRadius != 0.0F) {
        const auto extent = params.mDirection == kHorizontal
            ? source.mBaseDesc.mWidth
            : source.mBaseDesc.mHeight;
        radius = static_cast<float>(static_cast<int>(extent)) * params.mRadius;
    }

    // Tap i lies at i - center texels from the destination texel.
    float taps[kMaxPairedTaps] = {};
    const bool pairTaps = params.mBlurType == kGaussianBlur &&
        source.FilterMode() != kFilterPoint;
    const auto tapLimit = pairTaps ? kMaxPairedTaps : kMaxTaps;
    const auto requestedTaps =
        static_cast<int>(std::ceil(radius) * 2.0F + 1.0F);
    const auto numTaps =
        requestedTaps < tapLimit ? requestedTaps : tapLimit;
    const auto center = numTaps / 2;

    // sigma = radius / 2; the center tap's unnormalized weight is one.
    auto weightSum = 1.0F;
    if (numTaps + 1U >= 3) {
        const auto sigma = radius * 0.5F;
        const auto exponentScale = -0.5F / (sigma * sigma);
        for (int i = 0; i < center; ++i) {
            const auto distance = i - center;
            taps[i] = std::exp(
                static_cast<float>(distance * distance) * exponentScale);
            weightSum += taps[i] * 2.0F;
        }
        const auto normalize = 1.0F / weightSum;
        for (int i = 0; i < center; ++i) {
            taps[i] *= normalize;
            taps[numTaps - 1 - i] = taps[i];
        }
    }
    taps[center] = 1.0F / weightSum;

    float offsets[kMaxSamples] = {};
    float weights[kMaxSamples] = {};
    int numSamples;
    if (pairTaps) {
        // Each pair of neighbouring taps becomes one bilinear sample placed
        // between them at the ratio of their weights; the last tap stays
        // single.
        if (numTaps + 1U >= 3) {
            for (int i = 0; i < center; ++i) {
                const auto first = taps[i * 2];
                const auto second = taps[i * 2 + 1];
                const auto weight = first + second;
                const auto blend = second / weight;
                weights[i] = weight;
                offsets[i] =
                    (1.0F - blend) * static_cast<float>(i * 2 - center) +
                    blend * static_cast<float>(i * 2 + 1 - center);
            }
        }
        offsets[center] = static_cast<float>(center);
        weights[center] = taps[numTaps - 1];
        numSamples = center + 1;
    } else {
        if (numTaps != 0) {
            std::memcpy(weights, taps, numTaps * sizeof(float));
            for (int i = 0; i < numTaps; ++i) {
                offsets[i] = static_cast<float>(i - center);
            }
        }
        numSamples = numTaps;
    }

    auto& buffer = RndShaderDrawUtl::GetCBuffer(context, mCBufferSize);
    const auto width =
        static_cast<float>(static_cast<int>(source.mBaseDesc.mWidth));
    const auto height =
        static_cast<float>(static_cast<int>(source.mBaseDesc.mHeight));
    const auto stepX =
        (params.mDirection == kHorizontal ? 1.0F : 0.0F) / width;
    const auto stepY =
        (params.mDirection == kVertical ? 1.0F : 0.0F) / height;

    *static_cast<float*>(
        RndShaderDrawUtl::GetCBufferMember(buffer, mArraySlice)) =
        static_cast<float>(static_cast<int>(params.mArraySlice));
    for (int i = 0; i < kMaxSamples; ++i) {
        auto* sample = static_cast<float*>(RndShaderDrawUtl::GetCBufferMember(
            buffer, mBlurSampleOffsetsWeights + i));
        sample[0] = offsets[i] * stepX;
        sample[1] = offsets[i] * stepY;
        sample[2] = weights[i];
    }

    // The kernel's half extent in UV units along the direction and in taps.
    auto maxOffset = Vector4::sZero;
    const auto extent = static_cast<float>(numTaps) * 0.5F;
    const auto halfTaps = (static_cast<float>(numTaps) + 1.0F) * 0.5F;
    if (params.mDirection != kHorizontal) {
        maxOffset.y = extent / height;
        maxOffset.w = halfTaps;
    } else {
        maxOffset.x = extent / width;
        maxOffset.z = halfTaps;
    }
    *static_cast<Vector4*>(
        RndShaderDrawUtl::GetCBufferMember(buffer, mMaxOffset)) = maxOffset;

    if (params.mClassificationBuffer != nullptr) {
        const auto tileSize = params.mTileSize;
        const auto sourceWidth = static_cast<int>(source.mBaseDesc.mWidth);
        const auto sourceHeight = static_cast<int>(source.mBaseDesc.mHeight);
        auto* tileInfo = static_cast<float*>(
            RndShaderDrawUtl::GetCBufferMember(buffer, mTileInfo));
        tileInfo[0] = static_cast<float>(
            (sourceWidth + tileSize - 1) / tileSize);
        tileInfo[1] = static_cast<float>(
            (sourceHeight + tileSize - 1) / tileSize);
        tileInfo[2] = static_cast<float>(tileSize);
    }
    RndShaderDrawUtl::CommitCBuffer(buffer, context, mCBufferSize);

    const bool isArray = params.mArraySlice != -1;
    source.Select(
        context, kShaderProgramPixel, isArray ? mTexArray : mTexture, 0, 0);
    RndShaderDrawUtl::SelectPixelTexture(
        context, params.mSceneMask, mSceneMask);
    if (params.mClassificationBuffer != nullptr) {
        params.mClassificationBuffer->Select(
            context, kShaderProgramPixel, mTiledClassificationBuffer, 0, 0);
    }

    // The sample count rounds up to a power of two, at least two.
    unsigned int sampleDefine = 2;
    auto roundedSamples = static_cast<unsigned long>(numSamples) - 1;
    roundedSamples |= roundedSamples >> 1;
    roundedSamples |= roundedSamples >> 2;
    roundedSamples |= roundedSamples >> 4;
    roundedSamples |= roundedSamples >> 8;
    roundedSamples |= roundedSamples >> 16;
    if (static_cast<int>(roundedSamples + 1) > 1) {
        sampleDefine = static_cast<unsigned int>(roundedSamples + 1);
    }

    RndShaderKeyGroup keys{};
    auto key = mIsTexArray.SetValue(0, isArray ? 1U : 0U);
    key = mNumBlurSamples.SetValue(key, sampleDefine);
    key = mBlurType.SetValue(key, params.mBlurType);
    key = mBlurDirection.SetValue(key, params.mDirection);
    key = mUseSceneMask.SetValue(key, params.mSceneMask != nullptr ? 1U : 0U);
    keys.mKeys[kPixelKey] = mUseClassificationBuffer.SetValue(
        key, params.mClassificationBuffer != nullptr ? 1U : 0U);
    _SelectShaderCollection(context, keys);
}
