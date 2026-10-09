#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Fills the target with a two-color tile pattern. Not in the reference map.
// The vtable is at 0x192F548.
class RndShaderTestPattern : public RndShader {
public:
    // Field names are not in the reference map.
    struct Params {
        float mColor0[4];
        float mColor1[4];
        float mNumTiles[2];
    };

    RndShaderTestPattern();             // 0x6452E0
    ~RndShaderTestPattern() override;   // 0x6453F0, 0x645400

    const char* _GetClassNameImpl() const override;   // 0x6455B0
    const char* _GetShaderFilePath() const override;  // 0x645530
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x645540

    void Select(RndContext& context, const Params& params);  // 0x645420

    // Field names are not in the reference map.
    unsigned long mColor0;       // Constant offsets and the buffer size.
    unsigned long mColor1;
    unsigned long mNumTiles;
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndShaderTestPattern::Params, mNumTiles) == 0x20);
static_assert(sizeof(RndShaderTestPattern::Params) == 0x28);

static_assert(offsetof(RndShaderTestPattern, mColor0) == 288);
static_assert(offsetof(RndShaderTestPattern, mCBufferSize) == 312);
static_assert(sizeof(RndShaderTestPattern) == 320);
