#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndComputeBuffer;
struct RndSceneDrawTarget;

// Depth-of-field disc blur: blurs the scene by the normalized depth and emits
// bokeh sprites for the overbright pixels. The vtable is at 0x1939A48.
class RndCShaderDOFDiscBlur : public RndShaderCompute {
public:
    RndCShaderDOFDiscBlur();  // 0x6F2B40
    ~RndCShaderDOFDiscBlur() override;  // 0x6F2B90, 0x6F2BA0

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x6F32F0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x6F30B0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x6F30C0
    int _GetShaderStages() const override;           // slot 6 at 0x6F3300

    // 28-byte parameter block the depth-of-field stage fills (0x632BDF).
    // Name and field names are not in the reference map.
    struct Params {
        // The near and far blur distances, normalized to the camera's
        // clip range and clamped to [0, 1].
        float mNearBlur;
        float mFarBlur;
        float mBlurRadius;
        float mBokehOverbrightScale;
        // The bokeh luminance threshold: one over the light manager's
        // tonemapping exposure.
        float mOverbrightLuminance;
        int mBlurFalloffFunction;
        // Restricts the blur to the scene mask (the draw parameters'
        // mDrawSceneMask); otherwise a default texture stands in.
        bool mUseSceneMask;
    };

    // Blurs the target's source buffer into its destination buffer, one
    // thread group per light tile, and appends the bokeh sprites. Name not
    // in the reference map.
    void Dispatch(
        RndContext& context,
        RndBufferCollection& buffers,
        const RndSceneDrawTarget& target,
        RndComputeBuffer& sprites,
        const Params& params);  // 0x6F2BC0

    // Field names are not in the reference map.
    unsigned long mSceneTex;             // Resource indices.
    unsigned long mNormalizedDepthTex;
    unsigned long mSceneMask;
    unsigned long mFunctionTable;
    unsigned long mOutputSceneTex;
    unsigned long mSprites;
    unsigned long mTextureSize;          // Constant offsets and the buffer size.
    unsigned long mFalloffParams;
    unsigned long mBlurParams;
    unsigned long mOverbrightLuminance;
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderDOFDiscBlur::Params, mBlurFalloffFunction) == 20);
static_assert(offsetof(RndCShaderDOFDiscBlur::Params, mUseSceneMask) == 24);
static_assert(sizeof(RndCShaderDOFDiscBlur::Params) == 28);

static_assert(offsetof(RndCShaderDOFDiscBlur, mSceneTex) == 288);
static_assert(offsetof(RndCShaderDOFDiscBlur, mTextureSize) == 336);
static_assert(offsetof(RndCShaderDOFDiscBlur, mCBufferSize) == 368);
static_assert(sizeof(RndCShaderDOFDiscBlur) == 376);
