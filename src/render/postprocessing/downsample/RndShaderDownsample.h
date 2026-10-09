#pragma once

#include <cstddef>
#include <cstdint>

#include "render/shaders/RndShader.h"

class RndTextureBase;

// 2x and 4x color and bloom downsampling. The vtable is at 0x192EC70.
class RndShaderDownsample : public RndShader {
public:
    // 16-byte parameter block for a downsample draw. The type selects one of
    // the HX_DOWNSAMPLE_COLOR_2X/4X and HX_DOWNSAMPLE_BLOOM_2X/4X programs.
    // Field names are not in the reference map.
    struct Params {
        int mDownsampleType;
        bool mValueBasedBloom;
        std::uint8_t mReserved5[3];
        RndTextureBase* mSource;
    };

    RndShaderDownsample();             // 0x635FE0
    ~RndShaderDownsample() override;   // 0x636050, 0x636060

    const char* _GetClassNameImpl() const override;   // 0x6364C0
    const char* _GetShaderFilePath() const override;  // 0x6362A0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x6362B0
    bool _SupportsRTSlicing() const override;          // 0x6364B0

    // Uploads the texel offset, binds the source, and selects the programs.
    void Select(RndContext& context, const Params& params);  // 0x636080

    // Field names are not in the reference map.
    RndShaderDefInfo mBT709ToBT2020;
    RndShaderDefInfo mDownsampleType;
    RndShaderDefInfo mBloomValueBased;
    unsigned long mTexelOffset;          // Constant offset and the buffer size.
    unsigned long mCBufferSize;
    unsigned long mTexture;              // Resource index.
};

static_assert(offsetof(RndShaderDownsample::Params, mSource) == 8);
static_assert(sizeof(RndShaderDownsample::Params) == 16);

static_assert(offsetof(RndShaderDownsample, mBT709ToBT2020) == 288);
static_assert(offsetof(RndShaderDownsample, mTexelOffset) == 352);
static_assert(offsetof(RndShaderDownsample, mTexture) == 368);
static_assert(sizeof(RndShaderDownsample) == 376);
