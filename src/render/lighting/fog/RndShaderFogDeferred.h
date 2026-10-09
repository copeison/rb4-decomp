#pragma once

#include <cstddef>

#include "render/drawing/RndSceneDrawParams.h"
#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndContext;
class RndTextureBase;

// Deferred fog over the sky and linear depth. Not in the reference map;
// RndAtmosphereGlobals creates and registers it (0x451C90) and deletes it
// (0x451CC0).
class RndShaderFogDeferred : public RndShader {
public:
    // What RndFogCom::ApplyDeferred passes. Name and field names not in the
    // reference map.
    struct Params {
        RndBufferCollection* mBuffers;
        // The fogged target; only its first 41 bytes are copied.
        RndSceneDrawTarget mTarget;
        // The sky's atmosphere texture, or the default black texture.
        RndTextureBase* mSkyTexture;
    };

    RndShaderFogDeferred();             // 0x452CA0
    ~RndShaderFogDeferred() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Binds the sky texture, the frame's linear depth and the function
    // table, and selects the permutation for the HDR output. Name not in
    // the reference map.
    void Select(RndContext& context, Params& params);  // 0x452D20

    // Field names are not in the reference map.
    RndShaderDefInfo mBT709ToBT2020;
    unsigned long mFalloffParams;  // Constant offset.
    unsigned long mSkyTex;         // Resource indices.
    unsigned long mLinearDepthMap;
    unsigned long mFunctionTable;
};

static_assert(offsetof(RndShaderFogDeferred, mFalloffParams) == 312);
static_assert(sizeof(RndShaderFogDeferred::Params) == 64);
static_assert(sizeof(RndShaderFogDeferred) == 344);
