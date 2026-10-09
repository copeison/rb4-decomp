#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"

class RndContext;
class RndShaderCBufferConfig;

// Shader constant buffer created from a constant-buffer configuration. The
// base vtable is at 0x192F070; the platform buffer stores its element data
// inline after the object.
class RndShaderCBuffer {
public:
    // Requests the configuration's own element count. Name not in the
    // reference map.
    static constexpr unsigned long kConfigElementCount = ~0UL;
    // Creation flag that defers the initial upload. Name not in the
    // reference map.
    static constexpr unsigned int kDeferInitialSync = 1U << 0;

    // Reconstructed from eboot.elf at 0x639F30.
    static RndShaderCBuffer* New(
        const RndShaderCBufferConfig& config,
        unsigned int flags,
        unsigned long numElements = kConfigElementCount);

    RndShaderCBuffer(
        const RndShaderCBufferConfig& config,
        unsigned int flags,
        unsigned long numElements,
        void* data);                   // 0x639FF0
    virtual ~RndShaderCBuffer() {}     // slots 0-1: 0x63A030, 0x63A040

    // Reconstructed from eboot.elf at 0x639FC0. Runs the complete destructor,
    // releases the buffer through MemFree, and clears the pointer.
    static void SafeDelete(RndShaderCBuffer*& buffer);

    // Slot 2. Uploads the whole buffer. Name not in the reference map.
    virtual void _CreateImpl() = 0;
    // Slot 3. The map has _SyncImpl(RndContext&, unsigned long, unsigned long).
    virtual void _SyncImpl(RndContext& context, unsigned long first, unsigned long end) = 0;
    // Slot 4.
    virtual void _SelectImpl(RndContext& context) = 0;

    DELETE_OVERLOAD

    // Field names are not in the reference map.
    const char* mName;
    unsigned int mFlags;
    unsigned int mIndex;
    unsigned int mStageMask;
    unsigned int mUnknown28;
    unsigned long mConfigNumElements;
    unsigned long mNumElements;
    void* mData;
    bool mSyncPending;
};

static_assert(offsetof(RndShaderCBuffer, mName) == 8);
static_assert(offsetof(RndShaderCBuffer, mIndex) == 20);
static_assert(offsetof(RndShaderCBuffer, mConfigNumElements) == 32);
static_assert(offsetof(RndShaderCBuffer, mData) == 48);
static_assert(offsetof(RndShaderCBuffer, mSyncPending) == 56);
static_assert(sizeof(RndShaderCBuffer) == 64);
