#include "render/postprocessing/downsample/RndShaderDownsample.h"

#include <immintrin.h>

#include "render/buffers/RndShaderCBuffer.h"
#include "render/resources/shaders/shader_draw_state.h"
#include "render/shaders/RndShaderCBufferConfig.h"
#include "render/shaders/RndShaderResourceConfig.h"
#include "render/system/RndDevice.h"
#include "render/textures/RndTextureBase.h"

// Reconstructed from eboot.elf at 0x635FE0.
RndShaderDownsample::RndShaderDownsample()
    : mBT709ToBT2020{},
      mDownsampleType{},
      mBloomValueBased{},
      mTexelOffset(-1),
      mCBufferSize(0),
      mTexture(-1) {}

// Reconstructed from eboot.elf at 0x636050.
RndShaderDownsample::~RndShaderDownsample() {}

// Reconstructed from eboot.elf at 0x6364C0.
const char* RndShaderDownsample::_GetClassNameImpl() const {
    return "RndShaderDownsample";
}

// Reconstructed from eboot.elf at 0x6362A0.
const char* RndShaderDownsample::_GetShaderFilePath() const {
    return "../../system/data/shaders/Downsample.hlsl";
}

// Reconstructed from eboot.elf at 0x6362B0.
void RndShaderDownsample::_InitConfigImpl(
    RndShaderFixedDefines& fixedDefines,
    RndShaderDefinesGroup& defines,
    RndShaderCBufferConfig& cbuffer,
    RndShaderResourceConfig& resources) {
    fixedDefines.Add(Symbol("HX_DOWNSAMPLE_COLOR_2X"), 0);
    fixedDefines.Add(Symbol("HX_DOWNSAMPLE_COLOR_4X"), 1);
    fixedDefines.Add(Symbol("HX_DOWNSAMPLE_BLOOM_2X"), 2);
    fixedDefines.Add(Symbol("HX_DOWNSAMPLE_BLOOM_4X"), 3);

    auto& pixel = defines.GetDefines(kShaderProgramPixel);
    mBT709ToBT2020 = pixel.AddBool(Symbol("HX_BT709_TO_BT2020"));
    mDownsampleType = pixel.Add(Symbol("HX_DOWNSAMPLE_TYPE"), 0, 4);
    mBloomValueBased = pixel.AddBool(Symbol("HX_BLOOM_VALUE_BASED"));

    mTexelOffset = cbuffer.AddConstant(kShaderNumericFloat4, "gTexelOffset");
    mCBufferSize = cbuffer.mSize;
    mTexture = resources.AddTexture2DRTSliced(
        "gTexture",
        "gTexSampler",
        RndTextureBase::kTexture2D,
        kShaderNumericFloat4);
}

// Reconstructed from eboot.elf at 0x6364B0.
bool RndShaderDownsample::_SupportsRTSlicing() const {
    return true;
}

// Reconstructed from eboot.elf at 0x636080. The texel offset is half a texel
// of the source, (+x, +y, -x, -y), from a refined hardware reciprocal of its
// dimensions; it is written and the source bound only when a source exists.
// The constant buffer is uploaded only while its upload flag is set. HDR10
// output (mode 1) selects the BT.709-to-BT.2020 permutation.
void RndShaderDownsample::Select(RndContext& context, const Params& params) {
    constexpr unsigned long kPixelKey = 3;
    constexpr unsigned int kTextureFlags = 2;
    constexpr unsigned int kHdr10Output = 1;

    auto& buffer =
        rb4::render_shader_select_constant_buffer(context, mCBufferSize);
    if (auto* source = params.mSource) {
        const auto width = static_cast<int>(source->mBaseDesc.mWidth);
        const auto height = static_cast<int>(source->mBaseDesc.mHeight);
        const auto size = _mm_cvtepi32_ps(
            _mm_setr_epi32(width, height, width, height));
        const auto estimate = _mm_rcp_ps(size);
        const auto refined = _mm_add_ps(
            estimate,
            _mm_mul_ps(
                estimate,
                _mm_sub_ps(_mm_set1_ps(1.0F), _mm_mul_ps(size, estimate))));
        const auto offset =
            _mm_mul_ps(refined, _mm_setr_ps(0.5F, 0.5F, -0.5F, -0.5F));
        _mm_storeu_ps(
            static_cast<float*>(
                rb4::render_shader_constant_member(buffer, mTexelOffset)),
            offset);
        buffer.mSyncPending = true;
        rb4::render_shader_bind_pixel_texture(
            context, source, mTexture, kTextureFlags);
    }
    if (buffer.mSyncPending) {
        buffer._SyncImpl(context, 0, mCBufferSize);
        buffer.mSyncPending = false;
    }
    buffer._SelectImpl(context);

    const auto hdrMode = TheRndDevice()->mHdrOutputMode;
    RndShaderKeyGroup keys{};
    keys.mKeys[kPixelKey] =
        mBT709ToBT2020.SetValue(0, hdrMode == kHdr10Output ? 1U : 0U);
    keys.mKeys[kPixelKey] = mDownsampleType.SetValue(
        keys.mKeys[kPixelKey],
        static_cast<unsigned int>(params.mDownsampleType));
    keys.mKeys[kPixelKey] = mBloomValueBased.SetValue(
        keys.mKeys[kPixelKey], params.mValueBasedBloom ? 1U : 0U);
    _SelectShaderCollection(context, keys);
}
