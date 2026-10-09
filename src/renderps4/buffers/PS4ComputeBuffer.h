#pragma once

#include <cstddef>
#include <gnm/buffer.h>

#include "render/buffers/RndComputeBuffer.h"
#include "render/shaders/RndShaderEnums.h"

// Double-buffered PS4 compute buffer. The vtable is at 0x195F7A8.
class PS4ComputeBuffer : public RndComputeBuffer {
public:
    explicit PS4ComputeBuffer(const Description& desc);  // 0x8E3250
    ~PS4ComputeBuffer() override;                        // 0x8E3290, 0x8E32F0

    void _SelectForVSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;
    void _SelectForHSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;
    void _SelectForDSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;
    void _SelectForGSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;
    void _SelectForPSImpl(RndContext& context, unsigned long slot, unsigned int flags) override;
    void _SelectForCSImpl(
        RndContext& context,
        unsigned long slot,
        unsigned int flags,
        unsigned long extra) override;
    void _GpuCopyFromImpl(RndContext& context, RndShaderResource& source) override;
    bool _SyncStaticImpl() override;
    void _SyncDynamicImpl(RndContext& context) override;
    void _SyncFromGpuImpl(RndContext& context) override;
    void _FreeImpl() override;

    // Names not in the reference map.
    const sce::Gnm::Buffer& ActiveBuffer() const {
        return mBuffers[mActiveBank];
    }
    void* ActiveStorage() const {
        return mStorage[mActiveBank];
    }

    sce::Gnm::Buffer mBuffers[2];
    void* mStorage[2];
    unsigned long mActiveBank;
};

static_assert(offsetof(PS4ComputeBuffer, mBuffers) == 80);
static_assert(offsetof(PS4ComputeBuffer, mStorage) == 112);
static_assert(sizeof(PS4ComputeBuffer) == 136);
