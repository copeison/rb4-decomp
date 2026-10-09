#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndContext;
struct RndSceneDrawTarget;
class RndTextureBase;

// Fills the pixels that tiled lighting skipped at half resolution by
// interpolating the lit neighbors, writing the light buffer. Not in the
// reference map. The vtable is at 0x1939140.
class RndCShaderTiledLightsInterpolation : public RndShaderCompute {
public:
    RndCShaderTiledLightsInterpolation();             // 0x6D9F10
    ~RndCShaderTiledLightsInterpolation() override;   // 0x6D9F70, 0x6D9F80

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x6DA520
    const char* _GetShaderFilePath() const override; // slot 3 at 0x6DA340
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x6DA350
    bool _UsesShaderKeyImpl(RndShaderProgramType type, RndShaderKey key) const override;  // slot 7 at 0x6DA4C0

    // Interpolates the source light buffer into the scene's light buffer,
    // one thread per 2x2 pixel block. Name not in the reference map.
    void Dispatch(
        RndContext& context,
        RndBufferCollection& buffers,
        const RndSceneDrawTarget& drawTarget,
        RndTextureBase& srcLightAccum);  // 0x6D9FA0

    // Field names are not in the reference map.
    RndShaderDefInfo mShadingMode;
    unsigned long mSrcLightAccumBuffer;  // Resource indices.
    unsigned long mGBuffer0;
    unsigned long mLightInterpBuffer;
    unsigned long mDstLightAccumBuffer;
    unsigned long mCBufferSize;
    unsigned long mDimensions;           // Constant offset.
};

static_assert(
    offsetof(RndCShaderTiledLightsInterpolation, mSrcLightAccumBuffer) == 312);
static_assert(offsetof(RndCShaderTiledLightsInterpolation, mCBufferSize) == 344);
static_assert(sizeof(RndCShaderTiledLightsInterpolation) == 360);
