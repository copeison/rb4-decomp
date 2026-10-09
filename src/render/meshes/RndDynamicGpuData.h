#pragma once

#include <cstddef>

#include "os/memory/MemMgr.h"
#include "os/threading/CritSec.h"
#include "utl/containers/Vector.h"

class RndContext;

class RndDynamicGpuData;

// Queue of objects whose GPU data must be refreshed (render/
// RndDynamicGpuDataMgr.o). Each scene drawer owns one. Field names are not
// in the reference map.
class RndDynamicGpuDataMgr {
public:
    RndDynamicGpuDataMgr();   // 0x6C9D40
    // Detaches the objects still queued.
    ~RndDynamicGpuDataMgr();  // 0x6C9DE0
    // Refreshes the queued objects' GPU data and empties the queue.
    void ProcessQueue(RndContext& context);  // 0x6C9EB0
    // Unlinks an object under the queue's lock. Inlined into
    // ~RndDynamicGpuData in this build.
    void Dequeue(RndDynamicGpuData& data);
    // Appends an object under the queue's lock and records the queue in
    // it. Inlined into RndMaterialCom's poll (0x4F44B2) and RndMeshCom::_Poll
    // (0x5C9E00). Name not in the reference map.
    void Enqueue(RndDynamicGpuData& data);

    CritSec mCritSec;
    eastl::vector<RndDynamicGpuData*> mQueue;
};

static_assert(offsetof(RndDynamicGpuDataMgr, mQueue) == 16);
static_assert(sizeof(RndDynamicGpuDataMgr) == 48);

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
