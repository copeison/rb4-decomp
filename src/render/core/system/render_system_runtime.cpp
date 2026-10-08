#include "render/core/system/render_system_runtime.h"

#include <cstddef>

#include "render/core/system/render_system_runtime_adapters.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x3DDAE0.
void render_system_initialize(
    RenderSystem& system,
    const GameSystemInitOptions& options) {
    render_system_begin_initialization(system, options);
    render_system_resource_manager_initialize(system);
    render_system_platform_initialize(system, options);
    render_system_resource_manager_finalize(system);

    render_system_backend_resources_initialize(system);
    render_system_initialize_builtin_buffers(system);

    render_frame_owner_initialize(render_system_primary_frame_owner(system));
    const auto owner_count = render_system_frame_owner_count(system);
    for (std::size_t index = 0; index < owner_count; ++index) {
        render_frame_owner_initialize(
            render_system_frame_owner_at(system, index));
    }

    render_system_platform_finish_initialization(system);
    render_system_begin_runtime_epoch(system);
}

// Reconstructed from eboot.elf at 0x3DDC20.
void render_system_initialize_builtin_buffers(RenderSystem& system) {
    render_builtin_zero_pair_buffer(system);
    render_builtin_invalid_vector_buffer(system);
    render_builtin_sentinel_buffer(system);
    render_builtin_default_buffer(system);
}

// Reconstructed from eboot.elf at 0x3DDE60.
void render_system_shutdown(RenderSystem& system) {
    render_system_begin_shutdown(system);
    render_system_flush_deferred_releases(system);
    render_system_release_default_resources(system);
    render_system_backend_resources_shutdown(system);
    render_system_release_builtin_buffers(system);

    render_frame_owner_shutdown(render_system_primary_frame_owner(system));
    const auto owner_count = render_system_frame_owner_count(system);
    for (std::size_t index = 0; index < owner_count; ++index) {
        render_frame_owner_shutdown(
            render_system_frame_owner_at(system, index));
    }

    render_system_platform_shutdown(system);
}

}  // namespace rb4
