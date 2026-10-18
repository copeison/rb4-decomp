#pragma once

#include <cstddef>

#include "math/vector/Vector2i.h"
#include "render/shaders/RndShader.h"

class RndCameraContext;
class RndContext;
class RndTexture3D;
class RndTextureBase;

// Accumulates the in-scattered light of the froxel volume along each view
// ray (render/RndCShaderVScatAccumScattering.o, 0x6D1E70-0x6D26CF). The
// vtable is at 0x1938978.
class RndCShaderVScatAccumScattering : public RndShaderCompute {
public:
    // What RndVolumetricScatteringCom::BeginAsyncUpdate passes. Name and
    // field names not in the reference map.
    struct Params {
        // The buffer collection's size.
        Vector2i mScreenSize;
        RndTexture3D* mDensityInscattering;
        RndTextureBase* mTiledDepthRange;
        // Null unless the draw uses the scene mask.
        RndTextureBase* mSceneMask;
        RndTexture3D* mOutput;
        float mStartDist;
        float mEndDist;
        // The eye of a stereo pair, or -1.
        int mStereoEye;
        const RndCameraContext* mStereoCamera;
        const RndCameraContext* mCamera;
    };

    RndCShaderVScatAccumScattering();             // 0x6D1E70
    ~RndCShaderVScatAccumScattering() override;  // 0x6D1EF0, 0x6D1F00

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x6D26A0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x6D23F0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x6D2400

    // Accumulates the scattering of the volume along each view ray. The
    // map's signature is Dispatch(RndContext&, RndTextureBase&,
    // RndTextureBase&).
    void Dispatch(RndContext& context, Params& params);  // 0x6D1F20

    // Field names are not in the reference map.
    RndShaderDefInfo mUseSceneMask;
    RndShaderDefInfo mIsStereo;
    RndShaderDefInfo mStereoEye;
    unsigned long mDensityInscatteringTex;  // Resource indices.
    unsigned long mTiledDepthRangeBuffer;
    unsigned long mSceneMask;
    unsigned long mOutputTex;
    unsigned long mSrcDimensions;           // Constant offsets and the buffer
    unsigned long mDstDimensions;           // size.
    unsigned long mScreenDimensions;
    unsigned long mDepthFracOffsetParams;
    unsigned long mFrustumParams;
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderVScatAccumScattering, mUseSceneMask) == 288);
static_assert(
    offsetof(RndCShaderVScatAccumScattering, mDensityInscatteringTex) == 352);
static_assert(offsetof(RndCShaderVScatAccumScattering, mSrcDimensions) == 384);
static_assert(offsetof(RndCShaderVScatAccumScattering, mCBufferSize) == 424);
static_assert(
    offsetof(RndCShaderVScatAccumScattering::Params, mStartDist) == 0x28);
static_assert(sizeof(RndCShaderVScatAccumScattering::Params) == 0x48);
static_assert(sizeof(RndCShaderVScatAccumScattering) == 432);
