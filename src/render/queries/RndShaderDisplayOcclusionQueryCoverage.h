#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndComputeBuffer;

// Displays the coverage the occlusion queries wrote. The vtable is at
// 0x192B028.
class RndShaderDisplayOcclusionQueryCoverage : public RndShader {
public:
    RndShaderDisplayOcclusionQueryCoverage();             // 0x5F8D90
    ~RndShaderDisplayOcclusionQueryCoverage() override;   // 0x5F8DC0, 0x5F8DD0

    const char* _GetClassNameImpl() const override;   // 0x5F8EF0
    const char* _GetShaderFilePath() const override;  // 0x5F8EA0
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x5F8EB0

    // Not reconstructed yet.
    void Select(RndContext& context, RndComputeBuffer& coverage);  // 0x5F8DF0

    unsigned long mCoverage;  // Resource index. Name not in the reference map.
};

static_assert(offsetof(RndShaderDisplayOcclusionQueryCoverage, mCoverage) == 288);
static_assert(sizeof(RndShaderDisplayOcclusionQueryCoverage) == 296);
