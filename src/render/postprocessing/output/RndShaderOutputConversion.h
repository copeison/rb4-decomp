#pragma once

#include <cstddef>
#include <cstdint>

#include "render/shaders/RndShader.h"

class RndTextureBase;

// Final conversion of the frame to the output color space and transfer
// function. The vtable is at 0x192ED40.
class RndShaderOutputConversion : public RndShader {
public:
    // 24-byte parameter block for the final output-conversion draw. Field
    // names are not in the reference map.
    struct Params {
        RndTextureBase* mSource;
        RndTextureBase* mHmdMask;
        float mMinimumIntensity;
        bool mBT709ToBT2020;
        bool mPerceptualQuantizer;
        std::uint8_t mReserved22[2];
    };

    RndShaderOutputConversion();             // 0x6367A0
    ~RndShaderOutputConversion() override;   // 0x636810, 0x636820

    const char* _GetClassNameImpl() const override;   // 0x636C40
    const char* _GetShaderFilePath() const override;  // 0x636A50
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x636A60
    bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const override;  // 0x636BF0

    // Uploads the minimum intensity, binds the source and HMD mask, and
    // selects the programs.
    // The map's signature is Select(RndContext&, RndTextureBase&).
    void Select(RndContext& context, const Params& params);  // 0x636840

    // Field names are not in the reference map.
    RndShaderDefInfo mUseHmdMask;
    RndShaderDefInfo mUseBT709ToBT2020;
    RndShaderDefInfo mUsePerceptualQuantizer;
    unsigned long mMinIntensity;         // Constant offset and the buffer size.
    unsigned long mCBufferSize;
    unsigned long mSrcTex;               // Resource indices.
    unsigned long mHmdMaskTex;
};

static_assert(offsetof(RndShaderOutputConversion::Params, mMinimumIntensity) == 0x10);
static_assert(offsetof(RndShaderOutputConversion::Params, mBT709ToBT2020) == 0x14);
static_assert(sizeof(RndShaderOutputConversion::Params) == 0x18);

static_assert(offsetof(RndShaderOutputConversion, mUseHmdMask) == 288);
static_assert(offsetof(RndShaderOutputConversion, mMinIntensity) == 352);
static_assert(offsetof(RndShaderOutputConversion, mSrcTex) == 368);
static_assert(sizeof(RndShaderOutputConversion) == 384);
