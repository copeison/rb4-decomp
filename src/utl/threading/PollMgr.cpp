#include "utl/threading/PollMgr.h"

#include "entity/core/Entity.h"
#include "entity/core/EntityResource.h"

// The calling thread's poll state. The map's PollMgr::sThreadPolls; this
// build keeps it in the per-thread block at the offset recorded at
// 0x19B03D0.
thread_local ThreadPollContext gEntityThreadState;

// Reconstructed from eboot.elf at 0x2514C0.
void PollChildEntity(Entity* parent, Entity* child) {
    child->mPollMgr = parent->mPollMgr;
    child->mResource->PollEntity(child);
    child->mPollMgr = nullptr;
}
