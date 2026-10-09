#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/shaders/RndShaderResource.h"

// Structured GPU buffer with a CPU staging copy. The base vtable is at
// 0x192ED98.
class RndComputeBuffer : public RndShaderResource {
public:
    // Field names are not in the reference map.
    struct Description {
        unsigned long mElementSize;
        unsigned long mNumElements;
        const void* mInitialData;
        void* mGpuData;
        // Zeroed with the rest of the description by its initializer
        // (0x636DA0) and never set or read otherwise: no creator in the
        // binary fills it and neither RndComputeBuffer nor PS4ComputeBuffer
        // reads it. Name not in the reference map.
        unsigned int mReserved;
        unsigned int mFlags;
        const char* mName;
    };

    // Reconstructed from eboot.elf at 0x636C70. Creates the platform buffer
    // through the factory, allocates its staging copy, and syncs it.
    static RndComputeBuffer* New(const Description& desc);

    explicit RndComputeBuffer(const Description& desc);  // 0x636CC0
    ~RndComputeBuffer() override;                        // 0x636D10, 0x636D50

    int _GetTypeImpl() const override;  // 0x636DB0

    // Slot 10.
    virtual bool _SyncStaticImpl() = 0;
    // Slot 11.
    virtual void _SyncDynamicImpl(RndContext& context) = 0;
    // Slot 12.
    virtual void _SyncFromGpuImpl(RndContext& context) = 0;
    // Slot 13.
    virtual void _FreeImpl() = 0;

    DELETE_OVERLOAD

    Description mDesc;
    void* mStagingData;  // Name not in the reference map.
    // Never set by the recovered code. Name not in the reference map.
    unsigned long mStagingSize;
};

static_assert(sizeof(RndComputeBuffer::Description) == 48);
static_assert(offsetof(RndComputeBuffer, mDesc) == 16);
static_assert(offsetof(RndComputeBuffer, mStagingData) == 64);
static_assert(sizeof(RndComputeBuffer) == 80);
