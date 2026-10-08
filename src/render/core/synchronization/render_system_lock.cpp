#include "render/core/synchronization/render_system_lock.h"

#include <_pthread.h>

#include "render/core/system/render_system_state.h"

namespace rb4 {

void render_system_lock(RenderSystem& system) {
    auto& runtime = render_system_core_state(system);
    scePthreadMutexLock(&runtime.frame_mutex);
}

void render_system_unlock(RenderSystem& system) {
    auto& runtime = render_system_core_state(system);
    scePthreadMutexUnlock(&runtime.frame_mutex);
}

void render_system_enter_locked_call(RenderSystem& system) {
    ++render_system_core_state(system).lock_depth;
}

void render_system_leave_locked_call(RenderSystem& system) {
    --render_system_core_state(system).lock_depth;
}

}  // namespace rb4
