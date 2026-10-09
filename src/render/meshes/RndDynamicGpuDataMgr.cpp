#include "render/meshes/RndDynamicGpuData.h"

#include "render/context/RndContext.h"
#include "utl/text/Symbol.h"

// Reconstructed from eboot.elf at 0x6C9D40.
RndDynamicGpuDataMgr::RndDynamicGpuDataMgr() {}

// Reconstructed from eboot.elf at 0x6C9DE0.
RndDynamicGpuDataMgr::~RndDynamicGpuDataMgr() {
    if (!mQueue.empty()) {
        ScopedCritSec lock(mCritSec);
        for (RndDynamicGpuData* data : mQueue) {
            data->mMgr = nullptr;
        }
    }
}

// Reconstructed from eboot.elf at 0x6C9EB0.
// The GPU-timer label is a function-local symbol built on first use.
void RndDynamicGpuDataMgr::ProcessQueue(RndContext& context) {
    static Symbol sName;
    if (sName == Symbol()) {
        sName = Symbol("Sync Dynamic GPU Data");
    }
    RndScopedGpuStatBlock statBlock(context, sName.Str());
    ScopedCritSec lock(mCritSec);
    for (RndDynamicGpuData* data : mQueue) {
        data->_SyncDynamicGpuDataImpl(context);
        data->mMgr = nullptr;
    }
    mQueue.clear();
}
