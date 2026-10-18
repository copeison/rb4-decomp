#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndContext;
class RndTextureBase;

// Compute pass that converts the depth buffer to linear depth (render/
// RndCShaderLinearizeDepth.o, 0x637C10-0x6380AF). The vtable is at
// 0x192EEE8.
class RndCShaderLinearizeDepth : public RndShaderCompute {
public:
    // What the scene drawer's depth analysis passes. Field names are not in
    // the reference map.
    struct Params {
        RndTextureBase* mDepth;
        RndTextureBase* mLinearDepth;
    };

    RndCShaderLinearizeDepth();             // 0x637C10
    ~RndCShaderLinearizeDepth() override;  // 0x637C70, 0x637C80

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x638080
    const char* _GetShaderFilePath() const override; // slot 3 at 0x637F20
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x637F30

    // Converts the depth into the linear depth, one thread per texel in
    // 2x2 tiles. Orthographic (or camera-less) views take the HX_IS_ORTHO
    // permutation.
    void Dispatch(RndContext& context, Params& params);  // 0x637CA0

    // Field names are not in the reference map.
    RndShaderDefInfo mIsOrtho;
    unsigned long mDepth;        // Resource indices.
    unsigned long mLinearDepth;
    unsigned long mDimensions;   // Constant offset and the buffer size.
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderLinearizeDepth, mIsOrtho) == 288);
static_assert(offsetof(RndCShaderLinearizeDepth, mDepth) == 312);
static_assert(offsetof(RndCShaderLinearizeDepth, mCBufferSize) == 336);
static_assert(sizeof(RndCShaderLinearizeDepth::Params) == 16);
static_assert(sizeof(RndCShaderLinearizeDepth) == 344);
