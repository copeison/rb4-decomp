#include "os/system/Core.h"

#include "utl/threading/ThreadCall.h"
#include "utl/time/TimeMgr.h"

// Core services polled with ThreadCall whose objects have not been
// reconstructed. Names not in the reference map.

// Destroys the resource preload requests (vtable 0x195E538, created at
// 0x1167F10) that were released and whose load has finished. The name is
// inferred from those requests; the evidence is weak.
void resource_preload_poll();  // 0x1168140
// Empty in this build; core_terminate (0x219BE0) calls its neighbour at
// 0x112DC60. The name is a guess.
void platform_poll();  // 0x112DC50

// Reconstructed from eboot.elf at 0x219B80.
void core_poll_and_update_time() {
    core_poll();
    core_update_time();
}

// Reconstructed from eboot.elf at 0x219BB0.
void core_poll() {
    ThreadCallPoll();
    resource_preload_poll();
    platform_poll();
}

// Reconstructed from eboot.elf at 0x219BD0.
void core_update_time() {
    TheTimeMgr->Poll();
}
