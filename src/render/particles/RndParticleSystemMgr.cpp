// render/RndParticleSystemMgr.o (0x628E60 to 0x62949A).
#include "render/particles/RndParticleSystemMgr.h"

#include "os/system/System.h"
#include "utl/data/DataArray.h"
#include "utl/text/Symbol.h"

namespace {

// The -1, 8, 4 triple of a shared header that every object's static
// initializer sets (here at 0x6293F0; see RndMeshCom.cpp). Unused here.
// Names not in the reference map.
[[maybe_unused]] unsigned int gNullObjectId = 0xFFFFFFFFU;  // 0x1AAA450
[[maybe_unused]] int gMeshGroupSize2D = 8;                // 0x1AAA454
[[maybe_unused]] int gMeshGroupSize3D = 4;                // 0x1AAA458

// The chunk sets the limit needs: one per 16 particles. Inlined into the
// constructor once per pool. Name not in the reference map.
unsigned long NumChunkSets(unsigned long chunkedLimit) {
    return (chunkedLimit + 15) >> 4;
}

// The "rnd particlesys global_limit" config value. Inlined. Name not in
// the reference map.
int GlobalLimitConfig() {
    return SystemConfig(Symbol("rnd"), Symbol("particlesys"), Symbol("global_limit"))
        ->Int(1);
}

}  // namespace

// The object's statics, built by its static initializer (0x6293F0).
CritSec sParticleSystemCritSec;
RndParticleSystemMgr* theParticleSystemMgr;

// Reconstructed from eboot.elf at 0x628ED0. Each pool recomputes the chunk
// sets from mChunkedLimit.
RndParticleSystemMgr::RndParticleSystemMgr()
    : mGlobalLimit(GlobalLimitConfig()),
      mChunkedLimit((mGlobalLimit + 15) & ~15UL),
      mNumParticles(0),
      mPeakParticles(0),
      mNumChunks(0),
      mPeakChunks(0),
      mCritSec(),
      mIntPool(NumChunkSets(mChunkedLimit), 64),
      mFloatPool(NumChunkSets(mChunkedLimit) * 18, 64),
      mULongPool(NumChunkSets(mChunkedLimit) * 2, 128),
      mUnusedPool(0, 64),
      mColorPool(NumChunkSets(mChunkedLimit), 256),
      mEvalDataPool(NumChunkSets(mChunkedLimit) * 11, 128),
      mSystems() {}

// Reconstructed from eboot.elf at 0x6292E0. The set's subtree release is
// at 0x6293A0; the pools free their storage and the lock is destroyed.
RndParticleSystemMgr::~RndParticleSystemMgr() {}

// Reconstructed from eboot.elf at 0x628E60.
void RndParticleSystemMgr::Init() {
    theParticleSystemMgr = new RndParticleSystemMgr;
}

// Reconstructed from eboot.elf at 0x628E90.
void RndParticleSystemMgr::Terminate() {
    delete theParticleSystemMgr;
    theParticleSystemMgr = nullptr;
}
