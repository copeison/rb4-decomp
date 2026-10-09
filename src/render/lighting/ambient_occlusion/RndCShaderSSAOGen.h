#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndCameraContext;
class RndContext;
class RndTextureBase;

// Generates screen-space ambient occlusion from linear depth and the
// G-buffer normals.
class RndCShaderSSAOGen : public RndShaderCompute {
public:
    // What RndSSAOCom::GenerateAO passes. Name and field names not in the
    // reference map.
    struct Params {
        Params();  // 0x6D71B0

        RndBufferCollection* mBuffers;
        RndTextureBase* mNoiseTexture;
        RndCameraContext* mCamera;
        float mAngleBias;
        // In pixels of the collection's height.
        float mRadius;
        float mIntensity;
        // Limits the occlusion to the scene mask.
        bool mUseSceneMask;
    };

    RndCShaderSSAOGen();             // 0x6D7130
    ~RndCShaderSSAOGen() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Writes the occlusion of the frame's linear depth and normals into
    // its AO buffer. Not reconstructed. Name not in the reference map.
    void Dispatch(RndContext& context, Params& params);  // 0x6D71E0

    // Field names are not in the reference map.
    unsigned long mLinearDepthBuffer;  // Resource indices.
    unsigned long mGBufferNormal;
    unsigned long mNoiseTex;
    unsigned long mSceneMask;
    unsigned long mOutputBuffer;
    unsigned long mCBufferSize;
    unsigned long mDimensions;         // Constant offsets.
    unsigned long mParams;
};

static_assert(offsetof(RndCShaderSSAOGen, mLinearDepthBuffer) == 288);
static_assert(offsetof(RndCShaderSSAOGen, mCBufferSize) == 328);
static_assert(offsetof(RndCShaderSSAOGen::Params, mAngleBias) == 24);
static_assert(sizeof(RndCShaderSSAOGen::Params) == 40);
static_assert(sizeof(RndCShaderSSAOGen) == 352);
