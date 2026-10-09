#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndContext;
struct RndSceneDrawTarget;

// Fits the CMAA shapes to the pruned edges.
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

    // Fits the shapes to the edge buffer the parity selects. Not
    // reconstructed. Name not in the reference map.
    void Dispatch(
        RndContext& context,
        RndBufferCollection& buffers,
        const RndSceneDrawTarget& target,
        unsigned long parity);  // 0x4510A0

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
