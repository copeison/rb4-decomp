#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndContext;
struct RndSceneDrawTarget;

// Removes the non-dominant edges found by the edge detection (render/
// RndCShaderCMAAEdgePrune.o, 0x4508B0-0x450BEF). The vtable is at
// 0x1902A90.
class RndCShaderCMAAEdgePrune : public RndShaderCompute {
public:
    RndCShaderCMAAEdgePrune();  // 0x4508B0
    ~RndCShaderCMAAEdgePrune() override;  // 0x450900, 0x450910

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x450BB0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x450B00
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x450B10
    int _GetShaderStages() const override;           // slot 6 at 0x450BC0

    // Prunes the compressed edges into the edge buffer the parity selects.
    // The map's signature orders the threshold before the parity.
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
