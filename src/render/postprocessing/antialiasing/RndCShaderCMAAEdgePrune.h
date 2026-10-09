#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Removes the non-dominant edges found by the edge detection.
// The Dispatch function is not reconstructed yet.
class RndCShaderCMAAEdgePrune : public RndShaderCompute {
public:
    RndCShaderCMAAEdgePrune();  // 0x4508B0
    ~RndCShaderCMAAEdgePrune() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    unsigned long mSourceEdges;               // Resource indices.
    unsigned long mOutputEdges;
    unsigned long mNonDominantEdgeThreshold;  // Constant offset, buffer size.
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderCMAAEdgePrune, mSourceEdges) == 288);
static_assert(offsetof(RndCShaderCMAAEdgePrune, mCBufferSize) == 312);
static_assert(sizeof(RndCShaderCMAAEdgePrune) == 320);
