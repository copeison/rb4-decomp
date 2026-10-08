#include "render/core/system/render_system_runtime.h"

#include <cstddef>

#include "render/core/context/render_context.h"
#include "render/core/synchronization/render_deferred_release.h"
#include "render/core/system/render_epoch.h"
#include "render/core/system/render_system.h"
#include "render/core/system/render_system_globals.h"
#include "render/core/system/render_system_runtime_adapters.h"
#include "render/core/system/render_system_state.h"
#include "render/resources/system/default_render_resources.h"

namespace rb4 {

// Reconstructed from eboot.elf at 0x3DDAE0.
void render_system_initialize(
    RenderSystem& system,
    const GameSystemInitOptions& options) {
    auto& runtime = render_system_core_state(system);
    runtime.initialized = true;
    runtime.init_options = options;
    render_system_resource_manager_initialize(system);
    render_system_platform_initialize(system, options);
    render_system_resource_manager_finalize(system);

    render_system_backend_resources_initialize(system);
    render_system_initialize_builtin_buffers(system);

    render_context_initialize(render_system_primary_render_context(system));
    const auto context_count = render_system_render_context_count(system);
    for (std::size_t index = 0; index < context_count; ++index) {
        render_context_initialize(
            render_system_render_context_at(system, index));
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
    render_system_core_state(system).shutting_down = true;
    render_system_flush_deferred_releases(system);
    render_release_default_resources(
        render_system_default_resources(system));
    render_system_backend_resources_shutdown(system);
    render_system_release_builtin_buffers(system);

    render_context_shutdown(render_system_primary_render_context(system));
    const auto context_count = render_system_render_context_count(system);
    for (std::size_t index = 0; index < context_count; ++index) {
        render_context_shutdown(
            render_system_render_context_at(system, index));
    }

    render_system_platform_shutdown(system);
}

}  // namespace rb4
