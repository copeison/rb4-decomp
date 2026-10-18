#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndContext;
struct RndSceneDrawTarget;

// Shows the scene with an overdraw limit for the debug shading modes. Not in
// the reference map. The vtable is at 0x192F160.
class RndShaderDisplayShadingMode : public RndShader {
public:
    RndShaderDisplayShadingMode();             // 0x63DC90
    ~RndShaderDisplayShadingMode() override;  // 0x63DCE0, 0x63DCF0

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x63DFA0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x63DF10
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x63DF20

    // Selects the shader with the configured geometry, lighting and light-
    // probe overdraw limits and the target's source light buffer as the
    // scene texture. Name not in the reference map.
    void Select(
        RndContext& context,
        RndBufferCollection& buffers,
        const RndSceneDrawTarget& target);  // 0x63DD10

    // Field names are not in the reference map.
    unsigned long mMaxOverdraw;  // Constant offset and the buffer size.
    unsigned long mCBufferSize;
    unsigned long mSceneTex;     // Resource index.
};

static_assert(offsetof(RndShaderDisplayShadingMode, mMaxOverdraw) == 288);
static_assert(offsetof(RndShaderDisplayShadingMode, mSceneTex) == 304);
static_assert(sizeof(RndShaderDisplayShadingMode) == 312);
