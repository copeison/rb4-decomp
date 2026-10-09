#include "render/meshes/RndDynamicGpuData.h"

#include <algorithm>

void RndDynamicGpuDataMgr::Dequeue(RndDynamicGpuData& data) {
    ScopedCritSec lock(mCritSec);
    auto* position = std::find(mQueue.begin(), mQueue.end(), &data);
    if (position != mQueue.end()) {
        mQueue.erase(position);
    }
    data.mMgr = nullptr;
}

// The vector grows to twice its size, or to one element.
void RndDynamicGpuDataMgr::Enqueue(RndDynamicGpuData& data) {
    ScopedCritSec lock(mCritSec);
    mQueue.push_back(&data);
    data.mMgr = this;
}

// The base destructor is inlined into the mesh destructors in this build.
RndDynamicGpuData::~RndDynamicGpuData() {
    if (mMgr != nullptr) {
        mMgr->Dequeue(*this);
    }
}
