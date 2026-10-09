#pragma once

#include <cstddef>

#include "render/buffers/RndShaderCBuffer.h"

// PS4 constant buffer. PS4Factory allocates the object with its element data
// inline after it. The vtable is at 0x195F828.
class PS4ShaderCBuffer : public RndShaderCBuffer {
public:
    PS4ShaderCBuffer(
        const RndShaderCBufferConfig& config,
        unsigned int flags,
        unsigned long numElements,
        void* data);              // 0x8E3800
    ~PS4ShaderCBuffer() override;  // 0x8E3830, 0x8E38C0

    void _CreateImpl() override;  // 0x8E3900
    void _SyncImpl(RndContext& context, unsigned long first, unsigned long end) override;  // 0x8E3980
    void _SelectImpl(RndContext& context) override;  // 0x8E39C0

    // Field names are not in the reference map.
    unsigned char mUnknown64[16];
    void* mFrameData;
    void* mGpuData;
    unsigned long mGpuSize;
    unsigned long mFrame;

private:
    // Reconstructed from eboot.elf at 0x8E3880. Name not in the reference
    // map.
    void _FreeGpuData();
    // Stand-ins for code inlined into _SelectImpl; not yet reconstructed.
    // Names not in the reference map.
    void _PrepareFrameData(RndContext& context);
    void _SelectStages(RndContext& context) const;
};

static_assert(offsetof(PS4ShaderCBuffer, mFrameData) == 80);
static_assert(offsetof(PS4ShaderCBuffer, mGpuData) == 88);
static_assert(sizeof(PS4ShaderCBuffer) == 112);
