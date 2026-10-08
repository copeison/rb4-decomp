#pragma once

#include "game/startup/system_init_options.h"

namespace rb4 {

struct RenderSystem;

void render_system_begin_initialization(
    RenderSystem& system,
    const GameSystemInitOptions& options);
void render_system_resource_manager_initialize(RenderSystem& system);
void render_system_platform_initialize(
    RenderSystem& system,
    const GameSystemInitOptions& options);
void render_system_resource_manager_finalize(RenderSystem& system);
void render_system_backend_resources_initialize(RenderSystem& system);

void render_builtin_zero_pair_buffer(RenderSystem& system);
void render_builtin_invalid_vector_buffer(RenderSystem& system);
void render_builtin_sentinel_buffer(RenderSystem& system);
void render_builtin_default_buffer(RenderSystem& system);

void render_system_platform_finish_initialization(RenderSystem& system);
void render_system_begin_runtime_epoch(RenderSystem& system);

void render_system_begin_shutdown(RenderSystem& system);
void render_system_flush_deferred_releases(RenderSystem& system);
void render_system_release_default_resources(RenderSystem& system);
void render_system_backend_resources_shutdown(RenderSystem& system);
void render_system_release_builtin_buffers(RenderSystem& system);
void render_system_platform_shutdown(RenderSystem& system);

}  // namespace rb4
