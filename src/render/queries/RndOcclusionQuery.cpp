// render/RndOcclusionQuery.o (0x5F7D30 to 0x5F7EBF). Its static initializer
// (0x5F7EA0) sets three ints of a shared header (0x1AA7C04), which are not
// modelled.
#include "render/queries/RndOcclusionQuery.h"

#include "render/buffers/RndShaderCBuffer.h"
#include "render/context/RndContext.h"
#include "render/shaders/RndShaderMgr.h"
#include "render/system/RndDevice.h"
#include "render/system/RndFactory.h"

namespace {

// The occlusion parameters in the occlusion constant buffer: the query
// index and the expected coverage. Name not in the reference map.
float* CoverageParams(RndShaderCBuffer& buffer) {
    return static_cast<float*>(buffer.mData) +
        gRndDevice->mShaderMgr.mOcclusionQueryCoverageParams * 4;
}

}  // namespace

// Reconstructed from eboot.elf at 0x5F7D30.
RndOcclusionQuery* RndOcclusionQuery::New(const char* name) {
    return TheRndFactory()->CreateOcclusionQuery(name);
}

// Reconstructed from eboot.elf at 0x5F7D50.
RndOcclusionQuery::RndOcclusionQuery(const char* name)
    : mName(name),
      mEnabled(false),
      mCulled{false, false},
      mSphere(Sphere::sZero),
      mIndex(~0U),
      mPixelCount(0) {
    mLink.mNext = &mLink;
    mLink.mPrev = &mLink;
}

// Reconstructed from eboot.elf at 0x5F7DA0. The deleting destructor at
// 0x5F7DD0 releases the query through MemFree.
RndOcclusionQuery::~RndOcclusionQuery() {
    mLink.mNext->mPrev = mLink.mPrev;
    mLink.mPrev->mNext = mLink.mNext;
}

// Reconstructed from eboot.elf at 0x5F7E00.
void RndOcclusionQuery::SetShaderConstants(RndContext& context) {
    RndShaderCBuffer* buffer = context.mCBuffers[4];
    float* params = CoverageParams(*buffer);
    params[0] = static_cast<float>(mIndex);
    params[1] = static_cast<float>(mPixelCount);
    buffer->mSyncPending = true;
    buffer->_SyncImpl(context, 0, buffer->mNumElements);
    buffer->mSyncPending = false;
    buffer->_SelectImpl(context);
}

// Reconstructed from eboot.elf at 0x5F7E70.
void RndOcclusionQuery::SetNoOcclusionQueryShaderConstants(
    RndShaderCBuffer& buffer) {
    float* params = CoverageParams(buffer);
    params[0] = 0.0F;
    params[1] = 1.0F;
    buffer.mSyncPending = true;
}
