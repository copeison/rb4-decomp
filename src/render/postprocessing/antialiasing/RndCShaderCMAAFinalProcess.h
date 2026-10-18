#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndContext;
struct RndSceneDrawTarget;

// Blends the color along the fitted CMAA shapes (render/
// RndCShaderCMAAFinalProcess.o, 0x450BF0-0x45102F). The vtable is at
// 0x1902AF8.
class RndCShaderCMAAFinalProcess : public RndShaderCompute {
public:
    RndCShaderCMAAFinalProcess();  // 0x450BF0
    ~RndCShaderCMAAFinalProcess() override;  // 0x450C30, 0x450C40

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x450FF0
    const char* _GetShaderFilePath() const override; // slot 3 at 0x450F10
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x450F20
    int _GetShaderStages() const override;           // slot 6 at 0x451000

    // Blends the CMAA color (or the destination light accumulation) along
    // the edges of the other parity into the target's source light
    // accumulation. The map's signature lacks the target.
    void Dispatch(
        RndContext& context,
        RndBufferCollection& buffers,
        const RndSceneDrawTarget& target,
        unsigned long parity);  // 0x450C60

    // Field names are not in the reference map.
    unsigned long mInputColor;     // Resource indices.
    unsigned long mInputEdges;
    unsigned long mOutputBuffer;
    unsigned long mTargetSizeInv;  // Constant offset, buffer size.
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderCMAAFinalProcess, mInputColor) == 288);
static_assert(offsetof(RndCShaderCMAAFinalProcess, mCBufferSize) == 320);
static_assert(sizeof(RndCShaderCMAAFinalProcess) == 328);
