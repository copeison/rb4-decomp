#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndContext;
class RndTextureBase;

// Applies the accumulated volumetric scattering to the light accumulation
// buffer.
class RndCShaderVScatDeferred : public RndShaderCompute {
public:
    RndCShaderVScatDeferred();             // 0x6D3490
    ~RndCShaderVScatDeferred() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Fogs and scatters the source light accumulation into the destination
    // over the fog range. Not reconstructed. Name not in the reference map.
    void Dispatch(
        RndContext& context,
        RndTextureBase* accumScattering,
        RndTextureBase* srcLightAccum,
        RndTextureBase* dstLightAccum,
        RndTextureBase* linearDepth,
        float startDist,
        float endDist,
        float fogDensity,
        RndTextureBase* sceneMask);  // 0x6D3530

    // Field names are not in the reference map.
    RndShaderDefInfo mUseSceneMask;
    unsigned long mAccumScatteringTex;     // Resource indices.
    unsigned long mSrcLightAccumBuffer;
    unsigned long mLinearDepthTex;
    unsigned long mSceneMask;
    unsigned long mDstLightAccumBuffer;
    unsigned long mDimensions;             // Constant offsets and the buffer
    unsigned long mProj;                   // size.
    unsigned long mDepthFracOffsetParams;
    unsigned long mFogEndDistance;
    unsigned long mFogDensity;
    unsigned long mTexelSize;
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderVScatDeferred, mUseSceneMask) == 288);
static_assert(offsetof(RndCShaderVScatDeferred, mAccumScatteringTex) == 312);
static_assert(offsetof(RndCShaderVScatDeferred, mSceneMask) == 336);
static_assert(offsetof(RndCShaderVScatDeferred, mCBufferSize) == 400);
static_assert(sizeof(RndCShaderVScatDeferred) == 408);
