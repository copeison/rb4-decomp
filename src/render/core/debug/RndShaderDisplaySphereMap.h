#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndTexture2D;

// Displays a sphere map texture.
class RndShaderDisplaySphereMap : public RndShader {
public:
    RndShaderDisplaySphereMap();             // 0x6F4270
    ~RndShaderDisplaySphereMap() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    void Select(RndContext& context, RndTexture2D& sphereMap);  // 0x6F42D0

    // Field names are not in the reference map.
    unsigned long mSphereMap;  // Resource index.
};

static_assert(offsetof(RndShaderDisplaySphereMap, mSphereMap) == 288);
static_assert(sizeof(RndShaderDisplaySphereMap) == 296);
