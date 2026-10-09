#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndContext;
class RndTextureBase;

// Shared configuration and selection of the pixel and compute tonemap
// shaders. The derived shader's stage mask picks the program type that owns
// the defines and the source texture; the compute variant also writes an
// output texture. Name not in the reference map, whose RndCShaderTonemap
// still configured itself; the binary places it with RndTonemapShader. The
// vtable is at 0x1908440.
class RndTonemapShaderBase : public RndShader {
public:
    // Field names are not in the reference map.
    struct Params {
        RndTextureBase* mSource;
        RndTextureBase* mOutput;  // Read by the compute shader only.
        int mTonemapOp;           // An HX_TONEMAP_OP_* value.
        // The first two gTonemapParams components; the last two are derived
        // from them.
        float mTonemapParams[2];
    };

    RndTonemapShaderBase();             // 0x4ADC40
    ~RndTonemapShaderBase() override;   // 0x4AE200, 0x4AE210

    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x4ADF50

    // Selects the permutation, binds the textures for the program type, and
    // commits the tonemap constants. Name not in the reference map.
    void _Select(RndContext& context, const Params& params);  // 0x4ADCB0

    // Field names are not in the reference map.
    RndShaderProgramType mProgramType;  // Pixel or compute.
    RndShaderDefInfo mBT709ToBT2020;
    RndShaderDefInfo mTonemapOp;
    unsigned long mTonemapParams;       // Constant offset and buffer size.
    unsigned long mCBufferSize;
    unsigned long mSourceBuffer;        // Resource indices.
    unsigned long mOutputBuffer;
};

static_assert(offsetof(RndTonemapShaderBase::Params, mTonemapOp) == 16);
static_assert(offsetof(RndTonemapShaderBase::Params, mTonemapParams) == 20);
static_assert(sizeof(RndTonemapShaderBase::Params) == 32);
static_assert(offsetof(RndTonemapShaderBase, mProgramType) == 288);
static_assert(offsetof(RndTonemapShaderBase, mBT709ToBT2020) == 292);
static_assert(offsetof(RndTonemapShaderBase, mTonemapOp) == 312);
static_assert(offsetof(RndTonemapShaderBase, mTonemapParams) == 336);
static_assert(offsetof(RndTonemapShaderBase, mSourceBuffer) == 352);
static_assert(sizeof(RndTonemapShaderBase) == 368);
