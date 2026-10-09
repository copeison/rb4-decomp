#pragma once

#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/context/orbis_render_context.h"

class PS4Context;

namespace rb4 {

void orbis_render_context_construct_base(PS4Context& context);
void orbis_render_context_destruct_base(PS4Context& context);
void orbis_render_context_install_vtable(PS4Context& context);
void orbis_render_context_initialize_command_state(
    PS4Context& context);
void orbis_render_context_construct_compute_slot(
    PS4Context& context,
    std::size_t slot);
void orbis_render_context_initialize_state_defaults(
    PS4Context& context);
void orbis_render_context_initialize_allocation_map(
    PS4Context& context);
void orbis_render_context_initialize_gfx_slot(
    PS4Context& context,
    std::size_t slot,
    std::size_t cue_heap_size,
    std::size_t draw_command_buffer_size,
    std::size_t resource_buffer_size,
    std::size_t constant_update_size,
    std::size_t scratch_buffer_size);
void orbis_render_context_initialize_timestamp_records(
    PS4Context& context,
    std::size_t timestamp_buffer_size);
void orbis_render_context_initialize_compute_queue(
    PS4Context& context,
    std::size_t queue,
    std::uint32_t pipe,
    std::uint32_t priority,
    std::size_t ring_size,
    std::size_t ring_alignment);
void orbis_render_context_initialize_compute_context(
    PS4Context& context,
    std::size_t slot,
    std::size_t cue_slot_count,
    std::size_t command_buffer_size);
void orbis_render_context_initialize_label_pool(
    PS4Context& context,
    std::size_t initial_capacity);
void orbis_render_context_release_label_pool(PS4Context& context);
void orbis_render_context_release_timestamp_pool(PS4Context& context);
void orbis_render_context_destruct_compute_slot(
    PS4Context& context,
    std::size_t slot);
void orbis_render_context_destruct_command_state(
    PS4Context& context);
void orbis_render_context_emit_end_of_frame_event(
    PS4Context& context,
    std::size_t frame);
void orbis_render_context_emit_compute_completion(
    PS4Context& context,
    std::size_t frame,
    std::size_t slot);
void orbis_render_context_submit_compute(
    PS4Context& context,
    std::size_t frame,
    std::size_t slot,
    std::size_t queue);
void orbis_render_context_emit_gfx_completion(
    PS4Context& context,
    std::size_t frame);
void orbis_render_context_submit_gfx(
    PS4Context& context,
    std::size_t frame);
void orbis_render_context_reset_gfx_slot(
    PS4Context& context,
    std::size_t frame);
void orbis_render_context_initialize_gfx_hardware_state(
    PS4Context& context,
    std::size_t frame);
void orbis_render_context_clear_frame_draw_count(
    PS4Context& context,
    std::size_t frame);
void orbis_render_context_reset_compute_slot(
    PS4Context& context,
    std::size_t frame,
    std::size_t slot);
void orbis_render_context_initialize_frame_command_state(
    PS4Context& context,
    std::size_t frame);
void orbis_render_context_emit_default_control_state(
    PS4Context& context,
    std::size_t frame);

}  // namespace rb4
