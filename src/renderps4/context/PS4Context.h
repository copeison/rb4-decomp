#pragma once

#include <cstddef>

#include "render/context/RndContext.h"

// PS4 context: two graphics contexts, compute queues, transient vertex
// buffers, and submission state. Its own state is still reached through the
// orbis_render_context_* helpers by offset. The vtable is at 0x195FDB0; the
// constructor at 0x8E72B0 and the slot bodies are in the orbis context files.
class PS4Context : public RndContext {
public:
    PS4Context();            // 0x8E72B0
    ~PS4Context() override;  // 0x8E8070, 0x8E82B0

    void _SignalFenceImpl(RndFence& fence) override;            // 0x8EB730
    void _WaitFenceImpl(const RndFence& fence) override;        // 0x8EB7F0
    void _BeginFrameImpl() override;                            // 0x8E8850
    void _SetRenderTargetsImpl(int mode, const RenderTargetParams& params) override;  // 0x8E8D20
    void _SetBlendModeImpl(
        rb4::RndMaterialBlendMode mode,
        const BlendParams& params) override;                    // 0x8E92D0
    void _SetDepthModeImpl(unsigned int mode) override;         // 0x8E9F60
    void _SetStencilModeImpl(
        unsigned int mode,
        unsigned char reference,
        unsigned int readMask,
        unsigned int writeMask) override;                       // 0x8EA030
    void _SetFrontFaceImpl(bool counterClockwise) override;     // 0x8EA150
    void _SetCullModeImpl(RndCullMode mode) override;           // 0x8EA1C0
    void _SetFillModeImpl(bool solid) override;                 // 0x8EA230
    void _SetColorWriteMaskImpl(
        unsigned char targets,
        RndWriteMaskChannelSet channels) override;              // 0x8E96F0
    void _SetDepthClipEnabledImpl(bool enabled) override;       // 0x8EA2A0
    void _SetDepthBiasEnabledImpl(bool enabled) override;       // 0x8EA2B0
    void _SetThickLinesImpl(bool enabled) override;             // 0x8EA2C0
    void _DispatchComputeImpl(unsigned int x, unsigned int y, unsigned int z) override;  // 0x8EB870
    void _ResourceBarrierImpl(
        unsigned long count,
        const RndResourceBarrier* barriers) override;   // 0x8EAA10
    void _DrawPrimitivesImpl(
        RndPrimitive primitive,
        RndVertexType type,
        const void* vertices,
        unsigned long count) override;                          // 0x8EA2D0
    void _DrawIndirectImpl(RndPrimitive primitive, const RndComputeBuffer& args) override;  // 0x8EA730
    void _CopyBufferCounter(
        const RndComputeBuffer& source,
        const RndComputeBuffer& dest) override;                 // 0x8EA740
    void _DeselectAllReadWriteTexturesImpl(unsigned int stages) override;  // 0x8E9810
    void _DeselectAllSourceTexturesImpl(unsigned int stages) override;     // 0x8E9940
    void _SetSamplerImpl(
        RndShaderProgramType type,
        unsigned int slot,
        unsigned int wrap,
        unsigned int filter) override;                          // 0x8EA830
    void _DeactivateShaderProgramTypeImpl(RndShaderProgramType type) override;  // 0x8EA920
    void _PushMarkerImpl(const char* name) override;            // 0x8EB970
    void _PopMarkerImpl() override;                             // 0x8EB9D0
    void _BeginGpuStatsImpl(unsigned long key) override;        // 0x8EBA20
    void _EndGpuStatsImpl(unsigned long key) override;          // 0x8EBBB0
    RndGpuStatSample _EvalAndRetireGpuStatsImpl(unsigned long key) override;  // 0x8EBC70

    // Layout not yet modeled beyond the offsets the helpers use.
    unsigned char mPS4[0x44890 - 0x5728];
};

static_assert(sizeof(PS4Context) == 0x44890);
