#include "os/threading/CallOnMainThread.h"

#include "os/threading/CritSec.h"
#include "utl/containers/List.h"
#include "utl/containers/Map.h"
#include "utl/threading/Thread.h"

namespace {

// The queued calls and their lock, at 0x1A01D90 and 0x1A01DB0. Names not in
// the reference map.
eastl::list<eastl::pair<Hmx::MainThreadCallback*, void*>> gMainThreadCalls;
CritSec gMainThreadCallsCritSec;

}  // namespace

namespace Hmx {

// Reconstructed from eboot.elf at 0x3902B0.
void CallOnMainThread(MainThreadCallback* callback, void* context) {
    const ScePthread mainThread = Thread::s_MainThreadID;
    if (scePthreadSelf() == mainThread || mainThread == nullptr) {
        callback->OnMainThread(context);
        return;
    }
    ScopedCritSec lock(gMainThreadCallsCritSec);
    gMainThreadCalls.push_back({callback, context});
}

// Reconstructed from eboot.elf at 0x390370.
void CallOnMainThreadPoll() {
    ScopedCritSec lock(gMainThreadCallsCritSec);
    for (auto it = gMainThreadCalls.begin(); it != gMainThreadCalls.end(); ++it) {
        it->first->OnMainThread(it->second);
    }
    gMainThreadCalls.clear();
}

}  // namespace Hmx
