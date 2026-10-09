#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "render/particles/RndParticleCom.h"
#include "utl/containers/Vector.h"
#include "utl/containers/VectorAdapter.h"

namespace Hmx {
class Matrix3;
}

class RndCameraContext;
class RndContext;
class RndParticleCollection;
class Transform;
class Vector3;
struct RndInstanceData;

// Particle vertex buffer. The base vtable is at 0x1939A20.
class RndParticleBuffer {
public:
    // A particle's sort key and index. Field names are not in the reference
    // map.
    struct ParticleIndexDepth {
        float mDepth;
        unsigned long mIndex;
    };

    // Orders the sort list by ascending key. Inlined into the list's sort at
    // 0x6ECBA0.
    static bool ParticleDepthSort(
        const ParticleIndexDepth& left,
        const ParticleIndexDepth& right) {
        return left.mDepth < right.mDepth;
    }

    // Reconstructed from eboot.elf at 0x6EAFD0.
    static RndParticleBuffer* New(unsigned long numParticles, const char* name);

    RndParticleBuffer(unsigned long numParticles, const char* name);  // 0x6EB000
    virtual ~RndParticleBuffer() {}  // slots 0-1: 0x6ECB80, 0x6ECB90

    // Slot 2.
    virtual void _DrawBatchImpl(
        RndContext& context,
        const VectorAdapter<RndInstanceData>& instances) = 0;

    // Writes a quad of four vertices per active particle, in sorted order
    // for the blend modes that need it. The map has
    // _FillVertexBuffer(RndContext const&, RndCameraContext const&, void*);
    // this build takes the context as non-const.
    void _FillVertexBuffer(
        RndContext& context,
        const RndCameraContext& camera,
        void* vertices);  // 0x6EBD70
    // The particle's right and up axes, unscaled.
    void _ComputeParticleBasis(
        unsigned long particle,
        Vector3& right,
        Vector3& up,
        const Hmx::Matrix3& rotation,
        RndParticleCom::ParticleAlignment alignment,
        bool velocityAligned,
        const RndParticleCollection& particles);  // 0x6EB6B0
    // Fills the sort list in the given order: 0 back to front, 1 oldest
    // first, 2 newest first. Name not in the reference map.
    void _SortParticles(
        eastl::vector<ParticleIndexDepth>& sorts,
        int sortMode,
        const RndParticleCollection& particles,
        const RndCameraContext& camera);  // 0x6EC700
    // Records the draw statistics; empty in this build.
    void _UpdateStats(RndContext& context);  // 0x6ECB60

    DELETE_OVERLOAD

    // Field names are not in the reference map.
    unsigned long mCapacity;
    unsigned long mNumActive;
    RndParticleCollection* mParticles;
    const Transform* mXfm;  // The particle system's world transform.
    int mAlignment;         // A RndParticleCom::ParticleAlignment.
    int mSortMode;
    bool mVelocityAligned;
    // Moves each quad by its pivot in whole axis lengths; otherwise the
    // pivot moves it in quad sizes.
    bool mWorldSpace;
    // Copies the particles' data attributes into the vertices.
    bool mHasRotation;
    const char* mName;
};

static_assert(sizeof(RndParticleBuffer::ParticleIndexDepth) == 16);
static_assert(offsetof(RndParticleBuffer, mCapacity) == 8);
static_assert(offsetof(RndParticleBuffer, mXfm) == 32);
static_assert(offsetof(RndParticleBuffer, mAlignment) == 40);
static_assert(offsetof(RndParticleBuffer, mVelocityAligned) == 48);
static_assert(offsetof(RndParticleBuffer, mName) == 56);
static_assert(sizeof(RndParticleBuffer) == 64);
