#pragma once

#include <cstddef>
#include <cstdint>

#include "render/shaders/RndShader.h"

class RndTextureBase;

// Composites the half- and quarter-size bloom over the source. The vtable is
// at 0x192EBA0.
class RndShaderBloom : public RndShader {
public:
    // 48-byte parameter block assembled by the bloom pass. Field names are
    // not in the reference map.
    struct Params {
        RndTextureBase* mSource;
        RndTextureBase* mHalfSizeBloom;
        RndTextureBase* mQuarterSizeBloom;
        float mBloom[3];
        bool mHuePreservation;
        std::uint8_t mReserved37[3];
        float mOverbright[2];
    };

    RndShaderBloom();             // 0x634640
    ~RndShaderBloom() override;   // 0x6346B0, 0x6346C0

    const char* _GetClassNameImpl() const override;   // 0x634AB0
    const char* _GetShaderFilePath() const override;  // 0x634910
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x634920

    // Binds the source and bloom textures, uploads the bloom constants, and
    // selects the programs.
    void Select(RndContext& context, const Params& params);  // 0x6346E0

    // Field names are not in the reference map.
    RndShaderDefInfo mSampleHalfSize;
    RndShaderDefInfo mHuePreservation;
    unsigned long mBloomParams;          // Constant offsets and the buffer size.
    unsigned long mOverbrightParams;
    unsigned long mCBufferSize;
    unsigned long mSrcTex;               // Resource indices.
    unsigned long mHalfSizeBloomTex;
    unsigned long mQtrSizeBloomTex;
};

static_assert(offsetof(RndShaderBloom::Params, mBloom) == 0x18);
static_assert(offsetof(RndShaderBloom::Params, mHuePreservation) == 0x24);
static_assert(offsetof(RndShaderBloom::Params, mOverbright) == 0x28);
static_assert(sizeof(RndShaderBloom::Params) == 0x30);

static_assert(offsetof(RndShaderBloom, mSampleHalfSize) == 288);
static_assert(offsetof(RndShaderBloom, mBloomParams) == 328);
static_assert(offsetof(RndShaderBloom, mSrcTex) == 352);
static_assert(sizeof(RndShaderBloom) == 376);
