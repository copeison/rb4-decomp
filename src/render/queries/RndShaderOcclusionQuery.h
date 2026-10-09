#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Draws the occlusion query proxies, writing each query's coverage. Not in
// the reference map. The vtable is at 0x192B090.
class RndShaderOcclusionQuery : public RndShader {
public:
    RndShaderOcclusionQuery();             // 0x5F8F20
    ~RndShaderOcclusionQuery() override;   // 0x5F8F90, 0x5F8FA0

    const char* _GetClassNameImpl() const override;   // 0x5F90F0
    const char* _GetShaderFilePath() const override;  // 0x5F9070
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // 0x5F9080

    // Field names are not in the reference map.
    unsigned long mCoverageId;   // Constant offset.
    unsigned long mCBufferSize;
    unsigned long mCoverage;     // Resource index.
};

static_assert(offsetof(RndShaderOcclusionQuery, mCoverageId) == 288);
static_assert(offsetof(RndShaderOcclusionQuery, mCBufferSize) == 296);
static_assert(offsetof(RndShaderOcclusionQuery, mCoverage) == 304);
static_assert(sizeof(RndShaderOcclusionQuery) == 312);
