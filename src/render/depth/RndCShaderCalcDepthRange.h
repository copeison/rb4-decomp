#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndContext;
class RndTextureBase;

// Computes each light tile's depth range from the linear depth buffer. Not
// in the reference map; the name is the binary's class-name string. The
// object spans 0x636DE0-0x63720F; the vtable is at 0x192EE18.
class RndCShaderCalcDepthRange : public RndShaderCompute {
public:
    // What the scene drawer's depth analysis passes. Name and field names
    // not in the reference map.
    struct Params {
        RndTextureBase* mLinearDepth;
        RndTextureBase* mTiledDepthRange;
    };

    RndCShaderCalcDepthRange();             // 0x636DE0
    ~RndCShaderCalcDepthRange() override;  // 0x6371B0, 0x6371C0

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x6371E0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x6370A0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x6370B0

    // Writes each light tile's depth range. Name not in the reference map.
    void Dispatch(RndContext& context, const Params& params);  // 0x636E30

    // Field names are not in the reference map.
    unsigned long mLinearDepthBuffer;  // Resource indices.
    unsigned long mTiledDepthRangeBuffer;
    unsigned long mTileCounts;         // Constant offset and the buffer size.
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderCalcDepthRange, mLinearDepthBuffer) == 288);
static_assert(offsetof(RndCShaderCalcDepthRange, mCBufferSize) == 312);
static_assert(sizeof(RndCShaderCalcDepthRange) == 320);
