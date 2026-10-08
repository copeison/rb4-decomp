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

// Reconstructed from eboot.elf at 0x3DE080.
void render_system_acquire_frame_lock(RenderSystem& system) {
    auto& runtime = render_system_core_state(system);
    scePthreadMutexLock(&runtime.frame_mutex);
    ++runtime.lock_depth;
    runtime.lock_owner = scePthreadSelf();
}

// Reconstructed from eboot.elf at 0x3DE0B0.
void render_system_release_frame_lock(RenderSystem& system) {
    auto& runtime = render_system_core_state(system);
    static_cast<void>(scePthreadSelf());
    runtime.lock_owner = nullptr;
    --runtime.lock_depth;
    scePthreadMutexUnlock(&runtime.frame_mutex);
}

}  // namespace rb4
