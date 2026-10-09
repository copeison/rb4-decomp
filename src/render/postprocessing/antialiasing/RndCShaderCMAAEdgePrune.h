#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndContext;
struct RndSceneDrawTarget;

// Removes the non-dominant edges found by the edge detection.
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

    // Prunes the edges into the edge buffer the parity selects. Not
    // reconstructed. Name not in the reference map.
    void Dispatch(
        RndContext& context,
        RndBufferCollection& buffers,
        unsigned long parity,
        float nonDominantEdgeThreshold);  // 0x450930

    // Field names are not in the reference map.
    unsigned long mSourceEdges;               // Resource indices.
    unsigned long mOutputEdges;
    unsigned long mNonDominantEdgeThreshold;  // Constant offset, buffer size.
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderCMAAEdgePrune, mSourceEdges) == 288);
static_assert(offsetof(RndCShaderCMAAEdgePrune, mCBufferSize) == 312);
static_assert(sizeof(RndCShaderCMAAEdgePrune) == 320);
