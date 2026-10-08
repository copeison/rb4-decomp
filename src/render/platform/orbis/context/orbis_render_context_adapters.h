#pragma once

#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/context/orbis_render_context.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void render_free(void* allocation);
void orbis_render_context_construct_base(OrbisRenderContext& context);
void orbis_render_context_destruct_base(OrbisRenderContext& context);
void orbis_render_context_install_vtable(OrbisRenderContext& context);
void orbis_render_context_initialize_command_state(
    OrbisRenderContext& context);
void orbis_render_context_construct_compute_slot(
    OrbisRenderContext& context,
    std::size_t slot);
void orbis_render_context_initialize_state_defaults(
    OrbisRenderContext& context);
void orbis_transient_vertex_buffer_construct(
    OrbisRenderContext& context,
    std::size_t bank,
    std::size_t format);
void orbis_transient_vertex_buffer_initialize(
    OrbisRenderContext& context,
    std::size_t bank,
    std::size_t format,
    std::size_t vertex_capacity);
void orbis_render_context_initialize_allocation_map(
    OrbisRenderContext& context);
void orbis_render_context_initialize_gfx_slot(
    OrbisRenderContext& context,
    std::size_t slot,
    std::size_t cue_heap_size,
    std::size_t draw_command_buffer_size,
    std::size_t resource_buffer_size,
    std::size_t constant_update_size,
    std::size_t scratch_buffer_size);
void orbis_render_context_initialize_timestamp_records(
    OrbisRenderContext& context,
    std::size_t timestamp_buffer_size);
bool orbis_render_context_compute_queues_enabled(
    const OrbisRenderContext& context);
void orbis_render_context_initialize_compute_queue(
    OrbisRenderContext& context,
    std::size_t queue,
    std::uint32_t pipe,
    std::uint32_t priority,
    std::size_t ring_size,
    std::size_t ring_alignment);
void orbis_render_context_initialize_compute_context(
    OrbisRenderContext& context,
    std::size_t slot,
    std::size_t cue_slot_count,
    std::size_t command_buffer_size);
void orbis_render_context_initialize_label_pool(
    OrbisRenderContext& context,
    std::size_t initial_capacity);
void orbis_render_context_release_label_pool(OrbisRenderContext& context);
void orbis_render_context_release_timestamp_pool(OrbisRenderContext& context);
void orbis_transient_vertex_buffer_destruct(
    OrbisRenderContext& context,
    std::size_t bank,
    std::size_t format);
void orbis_render_context_destruct_compute_slot(
    OrbisRenderContext& context,
    std::size_t slot);
void orbis_render_context_destruct_command_state(
    OrbisRenderContext& context);
void render_system_set_render_context(
    OrbisRenderSystem& system,
    OrbisRenderContext& context);
void orbis_render_context_emit_end_of_frame_event(
    OrbisRenderContext& context,
    std::size_t frame);
void orbis_render_context_mark_compute_completion_pending(
    OrbisRenderContext& context,
    std::size_t frame,
    std::size_t slot);
void orbis_render_context_emit_compute_completion(
    OrbisRenderContext& context,
    std::size_t frame,
    std::size_t slot);
void orbis_render_context_submit_compute(
    OrbisRenderContext& context,
    std::size_t frame,
    std::size_t slot,
    std::size_t queue);
void orbis_render_context_mark_gfx_completion_pending(
    OrbisRenderContext& context,
    std::size_t frame);
void orbis_render_context_emit_gfx_completion(
    OrbisRenderContext& context,
    std::size_t frame);
void orbis_render_context_submit_gfx(
    OrbisRenderContext& context,
    std::size_t frame);
void orbis_render_context_reset_gfx_slot(
    OrbisRenderContext& context,
    std::size_t frame);
void orbis_render_context_initialize_gfx_hardware_state(
    OrbisRenderContext& context,
    std::size_t frame);
void orbis_render_context_clear_frame_draw_count(
    OrbisRenderContext& context,
    std::size_t frame);
void orbis_render_context_reset_compute_slot(
    OrbisRenderContext& context,
    std::size_t frame,
    std::size_t slot);
void orbis_render_context_initialize_frame_command_state(
    OrbisRenderContext& context,
    std::size_t frame);
void orbis_transient_vertex_buffer_reset(
    OrbisRenderContext& context,
    std::size_t frame,
    std::size_t format);
void orbis_render_context_emit_default_control_state(
    OrbisRenderContext& context,
    std::size_t frame);

}  // namespace rb4
