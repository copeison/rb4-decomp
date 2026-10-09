#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndContext;
struct RndSceneDrawTarget;

// Blends the color along the fitted CMAA shapes.
class RndCShaderCMAAFinalProcess : public RndShaderCompute {
public:
    RndCShaderCMAAFinalProcess();  // 0x450BF0
    ~RndCShaderCMAAFinalProcess() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Blends into the target's light accumulation. Not reconstructed.
    // Name not in the reference map.
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
