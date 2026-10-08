#pragma once

namespace rb4 {

struct RenderSystem;

RenderSystem* game_render_system_instance();
bool game_systems_shutdown_in_progress();
void game_systems_set_shutdown_in_progress(bool in_progress);
void game_render_dependents_shutdown(void* context);
void game_render_system_delete(RenderSystem& system);
void game_clear_render_system_instance();

}  // namespace rb4
