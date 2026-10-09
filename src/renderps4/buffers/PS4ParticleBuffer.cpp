#include "renderps4/buffers/PS4ParticleBuffer.h"

namespace {

constexpr unsigned long kVertexBytesPerParticle = 208;
constexpr unsigned long kIndexBytesPerParticle = 12;
constexpr unsigned int kIndicesPerParticle = 6;
constexpr const char* kAllocationName = "ParticleBuffer";

}  // namespace

// Reconstructed from eboot.elf at 0x8E2AA0. Each particle is a quad drawn
// as two triangles.
PS4ParticleBuffer::PS4ParticleBuffer(unsigned long numParticles, const char* name)
    : RndParticleBuffer(numParticles, name),
      mVertexBuffers{},
      mVertexStorage{nullptr, nullptr},
      mActiveBank(0),
      mBufferMask(0),
      mUnknown332(0),
      mIndices(nullptr) {
    const auto vertexBytes = kVertexBytesPerParticle * numParticles;
    _AllocateVertexStream(0, vertexBytes, kAllocationName);
    _AllocateVertexStream(1, vertexBytes, kAllocationName);

    auto* indices =
        _AllocateIndexStream(kIndexBytesPerParticle * numParticles, kAllocationName);
    for (unsigned int particle = 0; particle < numParticles; ++particle) {
        const auto base = static_cast<unsigned short>(particle * 4);
        *indices++ = base;
        *indices++ = static_cast<unsigned short>(base + 1);
        *indices++ = static_cast<unsigned short>(base + 2);
        *indices++ = base;
        *indices++ = static_cast<unsigned short>(base + 2);
        *indices++ = static_cast<unsigned short>(base + 3);
    }
}

// Reconstructed from eboot.elf at 0x8E2D30. The deleting destructor at
// 0x8E2D80 releases the buffer through MemFree.
PS4ParticleBuffer::~PS4ParticleBuffer() {
    PS4DeferredDelete(mVertexStorage[0]);
    PS4DeferredDelete(mVertexStorage[1]);
    PS4DeferredDelete(mIndices);
}

void PS4ParticleBuffer::_UpdateBuffer(RndContext& context) {
    mActiveBank = (mActiveBank & 1U) == 0 ? 1 : 0;
    _FillVertexBuffer(context, mVertexStorage[mActiveBank]);
}

// Reconstructed from eboot.elf at 0x8E2E10.
void PS4ParticleBuffer::_DrawBatchImpl(RndContext& context, const void* instances) {
    _UpdateBuffer(context);

    const auto numParticles = mNumActive;
    if (numParticles == 0) {
        return;
    }

    _SelectVertexStreams(context);
    _SelectInstanceStreams(context, instances);
    _DrawIndexed(context, static_cast<unsigned int>(kIndicesPerParticle * numParticles));
}
