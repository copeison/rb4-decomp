#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndContext;
struct RndSceneDrawTarget;

// Shows the scene with an overdraw limit for the debug shading modes. Not in
// the reference map.
class RndShaderDisplayShadingMode : public RndShader {
public:
    RndShaderDisplayShadingMode();             // 0x63DC90
    ~RndShaderDisplayShadingMode() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Selects the shader with the overdraw limit of the debug view and the
    // target's source light buffer as the scene texture. Name not in the
    // reference map.
    void Select(RndContext& context, RndBufferCollection& buffers, const RndSceneDrawTarget& target);  // 0x63DD10

    // Field names are not in the reference map.
    unsigned long mMaxOverdraw;  // Constant offset and the buffer size.
    unsigned long mCBufferSize;
    unsigned long mSceneTex;     // Resource index.
};

static_assert(offsetof(RndShaderDisplayShadingMode, mMaxOverdraw) == 288);
static_assert(offsetof(RndShaderDisplayShadingMode, mSceneTex) == 304);
static_assert(sizeof(RndShaderDisplayShadingMode) == 312);
