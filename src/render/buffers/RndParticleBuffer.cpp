#include "render/buffers/RndParticleBuffer.h"

#include "render/system/RndFactory.h"

// Reconstructed from eboot.elf at 0x6EAFD0.
RndParticleBuffer* RndParticleBuffer::New(
    unsigned long numParticles,
    const char* name) {
    return TheRndFactory()->CreateParticleBuffer(
        static_cast<unsigned int>(numParticles), name);
}

// Reconstructed from eboot.elf at 0x6EB000.
RndParticleBuffer::RndParticleBuffer(unsigned long numParticles, const char* name)
    : mCapacity(numParticles),
      mNumActive(0),
      mParticles(nullptr),
      mUnknown32(0),
      mAlignment(-1),
      mSortMode(-1),
      mVelocityAligned(false),
      mWorldSpace(true),
      mHasRotation(false),
      mName(name) {}

// Reconstructed from eboot.elf at 0x6ECB60.
void RndParticleBuffer::_UpdateStats(RndContext&) {}
