#pragma once

#include <cstddef>
#include <_pthread.h>

#include "os/memory/MemMgr.h"

class RndContext;

class RndDynamicGpuData;

// Queue of objects whose GPU data must be refreshed. Field names are not in
// the reference map.
class RndDynamicGpuDataMgr {
public:
    // Unlinks an object under the queue's mutex. Inlined into
    // ~RndDynamicGpuData in this build.
    void Dequeue(RndDynamicGpuData& data);

    int mLockDepth;
    ScePthreadMutex mMutex;
    RndDynamicGpuData** mBegin;
    RndDynamicGpuData** mEnd;
    RndDynamicGpuData** mCapacity;
};

static_assert(offsetof(RndDynamicGpuDataMgr, mMutex) == 8);
static_assert(offsetof(RndDynamicGpuDataMgr, mBegin) == 16);

// Base of objects with per-frame GPU data, such as meshes.
class RndDynamicGpuData {
public:
    RndDynamicGpuData() : mMgr(nullptr) {}
    virtual ~RndDynamicGpuData();  // Removes the object from its queue.

    virtual void _SyncDynamicGpuDataImpl(RndContext& context) = 0;

    DELETE_OVERLOAD

    RndDynamicGpuDataMgr* mMgr;  // Name not in the reference map.
};

static_assert(sizeof(RndDynamicGpuData) == 16);
