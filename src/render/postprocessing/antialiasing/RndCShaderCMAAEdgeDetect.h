#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndContext;
struct RndSceneDrawTarget;

// Detects the color edges that CMAA blends along.
class RndCShaderCMAAEdgeDetect : public RndShaderCompute {
public:
    RndCShaderCMAAEdgeDetect();  // 0x450490
    ~RndCShaderCMAAEdgeDetect() override;

    const char* _GetClassNameImpl() const override;
    const char* _GetShaderFilePath() const override;
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;

    // Finds the edges of the target's source light accumulation with the
    // threshold. Not reconstructed. Name not in the reference map.
    void Dispatch(
        RndContext& context,
        RndBufferCollection& buffers,
        const RndSceneDrawTarget& target,
        float edgeThreshold);  // 0x450500

    // Field names are not in the reference map.
    unsigned long mSourceBuffer;       // Resource indices.
    unsigned long mOutputEdgesBuffer;
    unsigned long mOutputColorBuffer;
    unsigned long mEdgeThreshold;      // Constant offset, buffer size.
    unsigned long mCBufferSize;
};

static_assert(offsetof(RndCShaderCMAAEdgeDetect, mSourceBuffer) == 288);
static_assert(offsetof(RndCShaderCMAAEdgeDetect, mCBufferSize) == 320);
static_assert(sizeof(RndCShaderCMAAEdgeDetect) == 328);
