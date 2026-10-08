#pragma once

namespace rb4 {

struct RenderSystem;

void render_system_construct_deferred_release_state(RenderSystem& system);
void render_system_destroy_deferred_release_state(RenderSystem& system);
void render_system_flush_deferred_releases(RenderSystem& system);
void render_system_enqueue_deferred_release(
    RenderSystem& system,
    void* object);

}  // namespace rb4
