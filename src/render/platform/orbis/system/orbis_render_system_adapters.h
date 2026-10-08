#pragma once

#include <cstddef>

namespace rb4 {

struct OrbisRenderSystem;
struct RenderSystem;

void* render_allocate(std::size_t size);
void orbis_render_system_initialize_video_state(OrbisRenderSystem& system);
void orbis_render_system_initialize_submission_state(
    OrbisRenderSystem& system);
void orbis_render_system_initialize_command_list(OrbisRenderSystem& system);
void orbis_render_system_destroy_command_list(OrbisRenderSystem& system);
void orbis_render_system_destroy_submission_state(OrbisRenderSystem& system);
void orbis_render_system_destroy_condition_state(OrbisRenderSystem& system);

}  // namespace rb4
