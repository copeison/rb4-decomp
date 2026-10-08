#include "render/platform/orbis/system/orbis_render_system.h"

#include <cstddef>

#include "render/platform/orbis/system/orbis_render_system_adapters.h"
#include "render/core/render_system_lifecycle.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisRenderSystemSize = 4352;
constexpr const char* kUnknownWorkerName = "Unknown Thread!";

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
    orbis_render_system_initialize_video_state(system);
    orbis_render_system_initialize_worker_state(system, kUnknownWorkerName);
    orbis_render_system_initialize_submission_state(system);
    orbis_render_system_initialize_command_list(system);
    orbis_render_system_publish_instance(system);
}

// Reconstructed from eboot.elf at 0x8D79B0.
void orbis_render_system_destruct(OrbisRenderSystem& system) {
    orbis_render_system_clear_instance();
    orbis_render_system_destroy_command_list(system);
    orbis_render_system_destroy_submission_state(system);
    orbis_render_system_destroy_profile_state(system);
    orbis_render_system_destroy_condition_state(system);
    render_system_destruct(orbis_render_system_base(system));
}

}  // namespace rb4
