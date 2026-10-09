#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Render test geometry colored by its vertices or by a constant. Not in the
// reference map. The vtable is at 0x192F418.
class RndShaderRenderTestSimple : public RndShader {
public:
    // Field names are not in the reference map.
    struct Params {
        bool mUseVertexColor;
        bool mUseCBufferColor;
        unsigned char mReserved2[2];
        float mColor[4];
    };

    RndShaderRenderTestSimple();             // 0x642500
    ~RndShaderRenderTestSimple() override;   // 0x642550, 0x642560

    const char* _GetClassNameImpl() const override;   // 0x6427A0
    const char* _GetShaderFilePath() const override;  // 0x6426B0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x6426C0

    void Select(RndContext& context, const Params& params);  // 0x642580

    // Field names are not in the reference map.
    RndShaderDefInfo mUseVertexColor;
    RndShaderDefInfo mUseCBufferColor;
    unsigned long mColor;        // Constant offset and the buffer size.
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndShaderRenderTestSimple::Params, mColor) == 4);
static_assert(sizeof(RndShaderRenderTestSimple::Params) == 0x14);

static_assert(offsetof(RndShaderRenderTestSimple, mUseVertexColor) == 288);
static_assert(offsetof(RndShaderRenderTestSimple, mColor) == 328);
static_assert(offsetof(RndShaderRenderTestSimple, mCBufferSize) == 336);
static_assert(sizeof(RndShaderRenderTestSimple) == 344);
