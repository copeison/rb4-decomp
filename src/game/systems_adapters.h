#pragma once

namespace rb4 {

struct RenderSystem;
struct DefaultRenderResources;
struct GameSystemInitOptions;
struct OrbisRenderSystem;

using GameCleanupCallback = void (*)(void* context);

RenderSystem& game_render_system_base(OrbisRenderSystem& system);
DefaultRenderResources& game_default_render_resources(RenderSystem& system);
void game_render_backend_post_initialize(
    RenderSystem& system,
    const GameSystemInitOptions& options);
void game_render_dependents_initialize(const GameSystemInitOptions& options);
void game_register_cleanup_callback(GameCleanupCallback callback);

RenderSystem* game_render_system_instance();
bool game_systems_shutdown_in_progress();
void game_systems_set_shutdown_in_progress(bool in_progress);
void game_render_dependents_shutdown(void* context);
void game_render_system_delete(RenderSystem& system);
void game_clear_render_system_instance();

}  // namespace rb4
