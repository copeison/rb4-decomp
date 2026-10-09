#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

// Fits the CMAA shapes to the pruned edges.
// The Dispatch function is not reconstructed yet.
class RndCShaderCMAAShapeFit : public RndShaderCompute {
public:
    RndCShaderCMAAShapeFit();  // 0x451030
    ~RndCShaderCMAAShapeFit() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Field names are not in the reference map.
    unsigned long mInputEdges;     // Resource indices.
    unsigned long mOutputColor;
    unsigned long mOutputEdges;
    unsigned long mTargetSizeInv;  // Constant offset, buffer size.
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderCMAAShapeFit, mInputEdges) == 288);
static_assert(offsetof(RndCShaderCMAAShapeFit, mCBufferSize) == 320);
static_assert(sizeof(RndCShaderCMAAShapeFit) == 328);
