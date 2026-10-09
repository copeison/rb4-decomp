#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndCameraContext;
class RndContext;

// Splits the tile light lists culled for both eyes into one eye's lists.
// Not in the reference map. The vtable is at 0x19391A8.
class RndCShaderTiledLightsStereoToMono : public RndShaderCompute {
public:
    // Field names are not in the reference map.
    struct Params {
        // The camera whose frustum the combined lists were culled with.
        const RndCameraContext* mBothEyesCamera;
        const RndCameraContext* mCamera;
        RndBufferCollection* mBothEyesBuffers;
        RndBufferCollection* mBuffers;
        bool mUseSceneMask;
    };

    RndCShaderTiledLightsStereoToMono();             // 0x6DA550
    ~RndCShaderTiledLightsStereoToMono() override;   // 0x6DA5A0, 0x6DA5B0

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x6DAEE0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x6DAC30
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x6DAC40

    // Binds both eyes' lists and this eye's outputs, fills the eye and
    // frustum constants, and dispatches over the tiles. Name not in the
    // reference map.
    void Dispatch(RndContext& context, const Params& params);  // 0x6DA5D0

    // Field names are not in the reference map.
    unsigned long mBothEyesLightIds;       // Resource indices.
    unsigned long mBothEyesLightIdRanges;
    unsigned long mSceneMask;
    unsigned long mLightIdsCount;
    unsigned long mCurEyeLightIds;
    unsigned long mCurEyeScratchLightIds;
    unsigned long mCurEyeLightIdRanges;
    unsigned long mWhichEye;               // Constant offsets.
    unsigned long mBothEyesDimensions;
    unsigned long mOneEyeDimensions;
    unsigned long mFrustumWidths;
    unsigned long mCBufferSize;
};

static_assert(
    offsetof(RndCShaderTiledLightsStereoToMono::Params, mUseSceneMask) == 32);
static_assert(
    offsetof(RndCShaderTiledLightsStereoToMono, mBothEyesLightIds) == 288);
static_assert(offsetof(RndCShaderTiledLightsStereoToMono, mWhichEye) == 344);
static_assert(offsetof(RndCShaderTiledLightsStereoToMono, mCBufferSize) == 376);
static_assert(sizeof(RndCShaderTiledLightsStereoToMono) == 384);
