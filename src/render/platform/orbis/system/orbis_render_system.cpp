#include "render/platform/orbis/system/orbis_render_system.h"

#include <cstddef>
#include <cstdint>

#include "core/memory/engine_memory.h"
#include "core/threading/engine_thread.h"
#include "render/core/system/render_system_lifecycle.h"
#include "render/platform/orbis/synchronization/orbis_frame_submit.h"
#include "render/platform/orbis/synchronization/orbis_gpu_sync.h"
#include "render/platform/orbis/system/orbis_render_system_globals.h"
#include "render/platform/orbis/video/orbis_video_output.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisRenderSystemSize = 4352;
constexpr const char* kUnknownWorkerName = "Unknown Thread!";

using ErasedRenderSystemMethod = void (*)();

struct OrbisRenderSystemVtable {
    ErasedRenderSystemMethod methods[17];
};

struct EmptyRenderSystemRange {
    const void* begin;
    const void* end;
    const void* capacity;
};

void orbis_render_system_noop_4() {}
void render_system_noop_8() {}
void render_system_noop_9() {}
void render_system_noop_10() {}
void render_system_noop_11() {}
void render_system_noop_12() {}
std::uint64_t orbis_render_system_zero_13() { return 0; }
std::uint64_t orbis_render_system_zero_14() { return 0; }
EmptyRenderSystemRange render_system_empty_range_15() { return {}; }
void render_system_noop_16() {}

template <typename Method>
ErasedRenderSystemMethod erase_method(Method method) {
    return reinterpret_cast<ErasedRenderSystemMethod>(method);
}

const OrbisRenderSystemVtable kOrbisRenderSystemVtable{{
    erase_method(orbis_render_system_destruct),
    erase_method(orbis_render_system_delete),
    erase_method(orbis_release_all_retired_allocations),
    erase_method(orbis_render_system_initialize),
    erase_method(orbis_render_system_noop_4),
    erase_method(orbis_render_system_shutdown),
    erase_method(orbis_render_system_wait_idle),
    erase_method(orbis_render_system_submit_frame),
    erase_method(render_system_noop_8),
    erase_method(render_system_noop_9),
    erase_method(render_system_noop_10),
    erase_method(render_system_noop_11),
    erase_method(render_system_noop_12),
    erase_method(orbis_render_system_zero_13),
    erase_method(orbis_render_system_zero_14),
    erase_method(render_system_empty_range_15),
    erase_method(render_system_noop_16),
}};

static_assert(sizeof(OrbisRenderSystemVtable) == 17 * sizeof(void*));

void orbis_render_system_install_vtable(OrbisRenderSystem& system) {
    auto** vtable =
        reinterpret_cast<const OrbisRenderSystemVtable**>(&system);
    *vtable = &kOrbisRenderSystemVtable;
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D5DF0.
OrbisRenderSystem* orbis_render_system_create() {
    auto* storage = render_allocate(kOrbisRenderSystemSize);
    auto* system = reinterpret_cast<OrbisRenderSystem*>(storage);
    orbis_render_system_construct(*system);
    return system;
}

// Reconstructed from eboot.elf at 0x8D77F0.
void orbis_render_system_construct(OrbisRenderSystem& system) {
    render_system_construct(orbis_render_system_base(system));
    orbis_render_system_install_vtable(system);
    orbis_render_system_initialize_video_state(system);
    engine_thread_initialize(
        orbis_submit_thread_wrapper(system), kUnknownWorkerName);
    orbis_render_system_initialize_submission_state(system);
    orbis_render_system_initialize_command_list(system);
    orbis_set_cached_flip_rate(system, -1);
    orbis_render_system_publish_instance(system);
}

// Reconstructed from eboot.elf at 0x8D79B0.
void orbis_render_system_destruct(OrbisRenderSystem& system) {
    orbis_render_system_clear_instance();
    orbis_render_system_destroy_command_list(system);
    orbis_render_system_destroy_submission_state(system);
    engine_thread_cancel(orbis_submit_thread(system));
    orbis_destroy_submit_condition(system);
    render_system_destruct(orbis_render_system_base(system));
}

}  // namespace rb4
