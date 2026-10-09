#pragma once

#include <cstddef>

#include "math/vector/Vector2i.h"
#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndCameraContext;
class RndComputeBuffer;
class RndContext;
struct RndTiledLightsComputeBuffer;

// Builds the per-tile light and probe lists for tiled lighting by culling
// every light against each screen tile's depth slices. The vtable is at
// 0x19390D8.
class RndCShaderTiledLightsCull : public RndShaderCompute {
public:
    // Field names are not in the reference map.
    struct Params {
        const RndCameraContext* mCamera;
        RndBufferCollection* mBuffers;
        // The right eye's buffers, read for stereo cameras.
        RndBufferCollection* mRightEyeBuffers;
        // Point and spot lights.
        RndTiledLightsComputeBuffer* mLightBuffers;
        RndComputeBuffer* mLightProbes;
        RndComputeBuffer* mSliceZeroLightIds;
        // Positive and negative point and spot lights of depth slice zero.
        unsigned long mNumSliceZeroLights[2][2];
        unsigned long mNumSliceZeroProbes;
        bool mUseSceneMask;
        bool mOnlySliceZero;
    };

    RndCShaderTiledLightsCull();             // 0x6D8A20
    ~RndCShaderTiledLightsCull() override;   // 0x6D8AC0, 0x6D8AD0

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x6D9E90
    const char* _GetShaderFilePath() const override; // slot 3 at 0x6D9900
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x6D9910
    bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const override;  // slot 7 at 0x6D9E00

    // Binds the lights, the depth ranges and the output lists, fills the
    // tile and camera constants, and dispatches over the tiles. Not
    // reconstructed yet: it reads camera-context fields whose layout is not
    // recovered.
    void Dispatch(RndContext& context, Params& params);  // 0x6D8AF0

    // Field names are not in the reference map.
    Vector2i mTilesPerThreadGroup;
    RndShaderDefInfo mIsStereo;
    RndShaderDefInfo mOnlySliceZero;
    unsigned long mPointLights;                // Resource indices.
    unsigned long mSpotLights;
    unsigned long mLightProbes;
    unsigned long mSliceZeroLightIds;
    unsigned long mTiledDepthRangeBuffer;
    unsigned long mRightEyeTiledDepthRangeBuffer;
    unsigned long mSceneMask;
    unsigned long mLightIdsCount;
    unsigned long mLightIds;
    unsigned long mScratchLightIds;
    unsigned long mLightIdRanges;
    unsigned long mDimensions;                 // Constant offsets.
    unsigned long mMonoDimensions;
    unsigned long mTileCounts;
    unsigned long mNumPosLights;
    unsigned long mNumNegLights;
    unsigned long mNumProbes;
    unsigned long mNumSliceZeroPosLights;
    unsigned long mNumSliceZeroNegLights;
    unsigned long mNumSliceZeroProbes;
    unsigned long mCamDir;
    unsigned long mCamNearFarParams;
    unsigned long mCamDepthRangeParams;
    unsigned long mFrustumCorners;
    unsigned long mFrustumWidths;
    unsigned long mCBufferSize;
};

// Tiles each thread group covers for the depth-slice count: 64 slots,
// widened to two tiles across and then doubled alternately in height and
// width until the group holds every slice's tiles. Name not in the
// reference map.
Vector2i CalcTilesPerThreadGroup(unsigned long depthSlices);  // 0x4AB6D0

static_assert(offsetof(RndCShaderTiledLightsCull::Params, mLightProbes) == 32);
static_assert(
    offsetof(RndCShaderTiledLightsCull::Params, mNumSliceZeroLights) == 48);
static_assert(offsetof(RndCShaderTiledLightsCull::Params, mUseSceneMask) == 88);
static_assert(offsetof(RndCShaderTiledLightsCull::Params, mOnlySliceZero) == 89);
static_assert(offsetof(RndCShaderTiledLightsCull, mIsStereo) == 296);
static_assert(offsetof(RndCShaderTiledLightsCull, mPointLights) == 336);
static_assert(offsetof(RndCShaderTiledLightsCull, mDimensions) == 424);
static_assert(offsetof(RndCShaderTiledLightsCull, mCBufferSize) == 536);
static_assert(sizeof(RndCShaderTiledLightsCull) == 544);
