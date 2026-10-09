#pragma once

#include <cstddef>

#include "render/buffers/RndComputeBuffer.h"
#include "render/shaders/RndShaderEnums.h"

// Double-buffered PS4 compute buffer. The vtable is at 0x195F7A8.
class PS4ComputeBuffer : public RndComputeBuffer {
public:
    // sce::Gnm::Buffer. Name not in the reference map.
    struct GnmBuffer {
        unsigned char mRegisters[16];
    };

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
    const GnmBuffer& ActiveBuffer() const {
        return mBuffers[mActiveBank];
    }
    void* ActiveStorage() const {
        return mStorage[mActiveBank];
    }

    GnmBuffer mBuffers[2];
    void* mStorage[2];
    unsigned long mActiveBank;

private:
    // Stand-ins for code inlined into _SyncStaticImpl and the stage selects;
    // not yet reconstructed. Names not in the reference map.
    void _AllocateStorage();
    void _Select(
        RndContext& context,
        RndShaderProgramType type,
        unsigned long slot,
        unsigned int flags) const;
};

static_assert(offsetof(PS4ComputeBuffer, mBuffers) == 80);
static_assert(offsetof(PS4ComputeBuffer, mStorage) == 112);
static_assert(sizeof(PS4ComputeBuffer) == 136);
