#include "render/meshes/RndDynamicGpuData.h"

#include <algorithm>
#include <cstring>

void RndDynamicGpuDataMgr::Dequeue(RndDynamicGpuData& data) {
    scePthreadMutexLock(&mMutex);
    ++mLockDepth;
    auto** position = std::find(mBegin, mEnd, &data);
    if (position != mEnd) {
        std::memmove(
            position,
            position + 1,
            static_cast<unsigned long>(mEnd - position - 1) * sizeof(*position));
        --mEnd;
    }
    data.mMgr = nullptr;
    --mLockDepth;
    scePthreadMutexUnlock(&mMutex);
}

// The base destructor is inlined into the mesh destructors in this build.
RndDynamicGpuData::~RndDynamicGpuData() {
    if (mMgr != nullptr) {
        mMgr->Dequeue(*this);
    }
}
