#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndContext;
class RndTextureBase;

// Classifies the tiles of a source buffer for the signed-distance pass. Not in
// the reference map; the name is the binary's class-name string. The object
// spans 0x62EAF0-0x62EF0F; the vtable is at 0x192E228.
class RndCShaderSignedDistanceClassify : public RndShaderCompute {
public:
    // What the scene drawer's scene-mask distance pass passes. Name and
    // field names not in the reference map.
    struct Params {
        RndTextureBase* mSrc;
        RndTextureBase* mDst;
        RndTextureBase* mDstClassification;
        // The settings' mask tile size, in pixels.
        int mTileSize;
    };

    RndCShaderSignedDistanceClassify();             // 0x62EAF0
    ~RndCShaderSignedDistanceClassify() override;  // 0x62EB30, 0x62EB40

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x62EEE0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x62EDF0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x62EE00

    // Classifies each tile of the source mask, one thread per tile. Name
    // not in the reference map.
    void Dispatch(RndContext& context, const Params& params);  // 0x62EB60

    // Field names are not in the reference map.
    unsigned long mSrcBuffer;                 // Resource indices.
    unsigned long mDstBuffer;
    unsigned long mDstClassificationBuffer;
    unsigned long mCBufferSize;
    unsigned long mDimensions;                // Constant offsets.
    unsigned long mTileParams;
};

static_assert(
    offsetof(RndCShaderSignedDistanceClassify::Params, mTileSize) == 24);
static_assert(offsetof(RndCShaderSignedDistanceClassify, mSrcBuffer) == 288);
static_assert(offsetof(RndCShaderSignedDistanceClassify, mCBufferSize) == 312);
static_assert(sizeof(RndCShaderSignedDistanceClassify) == 336);
