#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndContext;
struct RndSceneDrawTarget;

// Fits the CMAA shapes to the pruned edges (render/
// RndCShaderCMAAShapeFit.o, 0x451030-0x45144F). The vtable is at
// 0x1902B60.
class RndCShaderCMAAShapeFit : public RndShaderCompute {
public:
    RndCShaderCMAAShapeFit();  // 0x451030
    ~RndCShaderCMAAShapeFit() override;  // 0x451070, 0x451080

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x451410
    const char* _GetShaderFilePath() const override; // slot 3 at 0x451340
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x451350
    int _GetShaderStages() const override;           // slot 6 at 0x451420

    // Fits the shapes to the edge buffer the parity selects, writing the
    // target's source light accumulation and the other parity's edges. The
    // map's signature lacks the target.
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
