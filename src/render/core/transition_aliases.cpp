#include "render/core/transition_aliases.h"

#include "render/platform/orbis/synchronization/orbis_gpu_sync.h"
#include "render/platform/orbis/system/orbis_render_system_globals.h"

void PS4DeferredDelete(void* allocation) {
    rb4::orbis_defer_allocation_release(*rb4::g_orbis_render_system, allocation);
}
