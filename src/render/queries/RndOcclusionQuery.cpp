#include "render/queries/RndOcclusionQuery.h"

#include "render/system/RndFactory.h"

// Reconstructed from eboot.elf at 0x5F7D30.
RndOcclusionQuery* RndOcclusionQuery::New(const char* name) {
    return TheRndFactory()->CreateOcclusionQuery(name);
}

// Reconstructed from eboot.elf at 0x5F7D50.
RndOcclusionQuery::RndOcclusionQuery(const char* name)
    : mName(name),
      mState{},
      mResults{-1, -1, -1, -1},
      mFrame(-1),
      mResult(0) {
    mLink.mNext = &mLink;
    mLink.mPrev = &mLink;
}

// Reconstructed from eboot.elf at 0x5F7DA0. The deleting destructor at
// 0x5F7DD0 releases the query through MemFree.
RndOcclusionQuery::~RndOcclusionQuery() {
    mLink.mNext->mPrev = mLink.mPrev;
    mLink.mPrev->mNext = mLink.mNext;
}
