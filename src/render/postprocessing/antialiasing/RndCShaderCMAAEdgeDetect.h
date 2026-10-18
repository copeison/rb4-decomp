#pragma once

#include <cstddef>

#include "render/shaders/RndShader.h"

class RndBufferCollection;
class RndContext;
struct RndSceneDrawTarget;

// Detects the color edges that CMAA blends along (render/
// RndCShaderCMAAEdgeDetect.o, 0x450490-0x4508AF). The vtable is at
// 0x1902A28.
class RndCShaderCMAAEdgeDetect : public RndShaderCompute {
public:
    RndCShaderCMAAEdgeDetect();  // 0x450490
    ~RndCShaderCMAAEdgeDetect() override;  // 0x4504D0, 0x4504E0

    const char* _GetClassNameImpl() const override;  // slot 2 at 0x450840
    const char* _GetShaderFilePath() const override; // slot 3 at 0x450770
    void _InitConfigImpl(
        RndShaderFixedDefines& fixedDefines,
        RndShaderDefinesGroup& defines,
        RndShaderCBufferConfig& cbuffer,
        RndShaderResourceConfig& resources) override;  // slot 4 at 0x450780
    int _GetShaderStages() const override;           // slot 6 at 0x450860

    // Finds the edges of the target's source light accumulation with the
    // threshold, writing them compressed and seeding the color output. The
    // map's signature is Dispatch(RndContext&, RndBufferCollection&, float).
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
