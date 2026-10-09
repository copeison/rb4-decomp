#pragma once

#include <cstddef>
#include <gnm/buffer.h>

#include "render/buffers/RndParticleBuffer.h"
#include "render/meshes/RndVertexInterpreter.h"

// PS4 particle buffer with double-buffered vertex streams. The vtable is at
// 0x195F780.
class PS4ParticleBuffer : public RndParticleBuffer {
public:
    PS4ParticleBuffer(unsigned long numParticles, const char* name);  // 0x8E2AA0
    ~PS4ParticleBuffer() override;  // 0x8E2D30, 0x8E2D80

    void _DrawBatchImpl(
        RndContext& context,
        const VectorAdapter<RndInstanceData>& instances) override;  // 0x8E2E10

    // Reconstructed from eboot.elf at 0x8E2DE0 and inlined at 0x8E2E43.
    void _UpdateBuffer(RndContext& context);

    // Field names are not in the reference map.
    sce::Gnm::Buffer mVertexBuffers[2][RndVertexInterpreter::kNumStreams];
    void* mVertexStorage[2];
    unsigned long mActiveBank;
    unsigned int mBufferMask;
    unsigned int mUnknown332;
    unsigned short* mIndices;

private:
    // Inlined into the constructor.
    void _CreateBuffers(unsigned long numParticles);
};

static_assert(offsetof(PS4ParticleBuffer, mVertexBuffers) == 64);
static_assert(offsetof(PS4ParticleBuffer, mVertexStorage) == 320);
static_assert(offsetof(PS4ParticleBuffer, mIndices) == 352);
static_assert(sizeof(PS4ParticleBuffer) == 360);
