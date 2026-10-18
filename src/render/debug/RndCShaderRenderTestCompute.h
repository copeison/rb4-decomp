#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndComputeBuffer;
class RndContext;
class RndTextureBase;

// Compute render test that writes an animated pattern to a texture or a
// buffer. Not in the reference map. The object spans 0x6F3E00-0x6F426F; the
// vtable is at 0x1939B80.
class RndCShaderRenderTestCompute : public RndShaderCompute {
public:
    // Name and field names not in the reference map.
    struct Params {
        float mTime;  // The pattern's animation parameter.
        RndTextureBase* mTexture;
        // Written instead of the texture when set.
        RndComputeBuffer* mBuffer;
    };

    RndCShaderRenderTestCompute();             // 0x6F3E00
    ~RndCShaderRenderTestCompute() override;  // 0x6F3E50, 0x6F3E60

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x6F4240
    const char* _GetShaderFilePath() const override; // slot 3 at 0x6F4120
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x6F4130

    // Writes the pattern into the buffer or the texture. Name not in the
    // reference map.
    void Dispatch(RndContext& context, const Params& params);  // 0x6F3E80

    // Field names are not in the reference map.
    RndShaderDefInfo mWriteToBuffer;
    unsigned long mDstTexture;    // Resource indices.
    unsigned long mDstBuffer;
    unsigned long mDimensions;    // Constant offsets and the buffer size.
    unsigned long mAnimParams;
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderRenderTestCompute::Params, mBuffer) == 16);
static_assert(offsetof(RndCShaderRenderTestCompute, mWriteToBuffer) == 288);
static_assert(offsetof(RndCShaderRenderTestCompute, mDstTexture) == 312);
static_assert(offsetof(RndCShaderRenderTestCompute, mCBufferSize) == 344);
static_assert(sizeof(RndCShaderRenderTestCompute) == 352);
