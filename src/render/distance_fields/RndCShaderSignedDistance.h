#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndContext;
class RndTextureBase;

// Signed-distance pass over a source buffer, guided by the classification
// buffer. Not in the reference map; the name is the binary's class-name
// string. The object spans 0x62E660-0x62EAEF; the vtable is at 0x192E1C0.
class RndCShaderSignedDistance : public RndShaderCompute {
public:
    // What the scene drawer's scene-mask distance pass passes. Name and
    // field names not in the reference map.
    struct Params {
        RndTextureBase* mSrc;
        RndTextureBase* mClassification;
        RndTextureBase* mDst;
        // The distance the field saturates at, in pixels; the caller clamps
        // it to the tile size.
        int mMaxDistance;
        // The settings' mask tile size, in pixels.
        int mTileSize;
    };

    RndCShaderSignedDistance();             // 0x62E660
    ~RndCShaderSignedDistance() override;  // 0x62E6A0, 0x62E6B0

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x62EAC0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x62E9B0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x62E9C0

    // Writes the signed distance of the source's mask, guided by the tile
    // classification. Name not in the reference map.
    void Dispatch(RndContext& context, const Params& params);  // 0x62E6D0

    // Field names are not in the reference map.
    unsigned long mSrcBuffer;              // Resource indices.
    unsigned long mClassificationBuffer;
    unsigned long mDstBuffer;
    unsigned long mCBufferSize;
    unsigned long mDimensions;             // Constant offsets.
    unsigned long mTileParams;
    unsigned long mDistanceParams;
};

static_assert(offsetof(RndCShaderSignedDistance::Params, mTileSize) == 28);
static_assert(offsetof(RndCShaderSignedDistance, mSrcBuffer) == 288);
static_assert(offsetof(RndCShaderSignedDistance, mCBufferSize) == 312);
static_assert(sizeof(RndCShaderSignedDistance) == 344);
