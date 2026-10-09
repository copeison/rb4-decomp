#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "utl/containers/VectorAdapter.h"

class RndContext;
struct RndInstanceData;

// Particle vertex buffer. The base vtable is at 0x1939A20.
class RndParticleBuffer {
public:
    // Reconstructed from eboot.elf at 0x6EAFD0.
    static RndParticleBuffer* New(unsigned long numParticles, const char* name);

    RndParticleBuffer(unsigned long numParticles, const char* name);  // 0x6EB000
    virtual ~RndParticleBuffer() {}  // slots 0-1: 0x6ECB80, 0x6ECB90

    // Slot 2.
    virtual void _DrawBatchImpl(
        RndContext& context,
        const VectorAdapter<RndInstanceData>& instances) = 0;

    // Generates the frame's particle vertices at 0x6EBD70. The map has
    // _FillVertexBuffer(RndContext const&, RndCameraContext const&, void*);
    // this build passes no camera context.
    void _FillVertexBuffer(RndContext& context, void* vertices);
    // Records the draw statistics; empty in this build.
    void _UpdateStats(RndContext& context);  // 0x6ECB60

    DELETE_OVERLOAD

    // Field names are not in the reference map.
    unsigned long mCapacity;
    unsigned long mNumActive;
    void* mParticles;
    unsigned long mUnknown32;
    int mAlignment;
    int mSortMode;
    bool mVelocityAligned;
    bool mWorldSpace;
    bool mHasRotation;
    const char* mName;
};

static_assert(offsetof(RndParticleBuffer, mCapacity) == 8);
static_assert(offsetof(RndParticleBuffer, mAlignment) == 40);
static_assert(offsetof(RndParticleBuffer, mVelocityAligned) == 48);
static_assert(offsetof(RndParticleBuffer, mName) == 56);
static_assert(sizeof(RndParticleBuffer) == 64);
