#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndTextureBase;

// Refines the unrefined scene mask. Not in the reference map; the name is the
// binary's class-name string.
class RndShaderRefineSceneMask : public RndShader {
public:
    RndShaderRefineSceneMask();             // 0x642360
    ~RndShaderRefineSceneMask() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Binds the unrefined mask and selects the default programs. Name not in
    // the reference map.
    void Select(RndContext& context, RndTextureBase& unrefinedMask);  // 0x6423C0

    // Field name is not in the reference map.
    unsigned long mUnrefinedMask;  // Resource index.
};

static_assert(offsetof(RndShaderRefineSceneMask, mUnrefinedMask) == 288);
static_assert(sizeof(RndShaderRefineSceneMask) == 296);
