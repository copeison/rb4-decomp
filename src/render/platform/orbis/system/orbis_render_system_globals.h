#pragma once

#include <cstddef>
#include <cstdint>
#include <_pthread.h>
#include <kernel/equeue.h>

class NamedThread;
class Thread;

class PS4Context;

namespace rb4 {

struct OrbisRenderSystem;
struct OrbisBackBuffer;
struct OrbisBufferDescriptor;
struct RenderSystem;

extern OrbisRenderSystem* g_orbis_render_system;

OrbisRenderSystem* orbis_render_system_instance();
RenderSystem& orbis_render_system_base(OrbisRenderSystem& system);
PS4Context& orbis_render_system_context(OrbisRenderSystem& system);
std::int32_t orbis_video_output_handle(const OrbisRenderSystem& system);
SceKernelEqueue orbis_event_queue(const OrbisRenderSystem& system);
void orbis_set_video_output_handle(
    OrbisRenderSystem& system,
    std::int32_t handle);
void orbis_set_event_queue(
    OrbisRenderSystem& system,
    SceKernelEqueue queue);
void orbis_render_system_initialize_video_state(OrbisRenderSystem& system);
void orbis_render_system_initialize_submission_state(
    OrbisRenderSystem& system);
void orbis_render_system_initialize_command_list(OrbisRenderSystem& system);
void orbis_render_system_destroy_command_list(OrbisRenderSystem& system);
void orbis_render_system_destroy_submission_state(OrbisRenderSystem& system);
void orbis_initialize_submit_condition(OrbisRenderSystem& system);
void orbis_destroy_submit_condition(OrbisRenderSystem& system);
void orbis_wait_for_submit_token(OrbisRenderSystem& system);
void orbis_lock_submission(OrbisRenderSystem& system);
void orbis_unlock_submission(OrbisRenderSystem& system);
void orbis_submit_scope_begin(OrbisRenderSystem& system);
void orbis_submit_scope_end(OrbisRenderSystem& system);
void orbis_lock_retired_allocations(OrbisRenderSystem& system);
void orbis_unlock_retired_allocations(OrbisRenderSystem& system);
void orbis_enqueue_retired_allocation(
    OrbisRenderSystem& system,
    void* allocation,
    std::uint64_t frame);
void orbis_release_retired_allocations_through(
    OrbisRenderSystem& system,
    std::uint64_t completed_frame);
void orbis_release_all_retired_allocations_locked(
    OrbisRenderSystem& system);
void render_system_set_render_context(
    OrbisRenderSystem& system,
    PS4Context& context);
void render_system_set_back_buffer(
    OrbisRenderSystem& system,
    OrbisBackBuffer& back_buffer);
std::uint64_t orbis_render_system_epoch(const OrbisRenderSystem& system);
bool orbis_submit_token_available(const OrbisRenderSystem& system);
void orbis_publish_submit_token(OrbisRenderSystem& system);
void orbis_consume_submit_token(OrbisRenderSystem& system);
void orbis_signal_submit_condition(OrbisRenderSystem& system);
bool orbis_submit_thread_running(const OrbisRenderSystem& system);
void orbis_set_submit_thread_running(
    OrbisRenderSystem& system,
    bool running);
std::int32_t orbis_cached_flip_rate(const OrbisRenderSystem& system);
void orbis_set_cached_flip_rate(
    OrbisRenderSystem& system,
    std::int32_t rate);
Thread& orbis_submit_thread(OrbisRenderSystem& system);
NamedThread& orbis_submit_thread_wrapper(OrbisRenderSystem& system);
OrbisBufferDescriptor* orbis_default_vertex_descriptors(
    OrbisRenderSystem& system);
const OrbisBufferDescriptor* orbis_default_vertex_descriptors();
void orbis_set_default_vertex_buffer(
    OrbisRenderSystem& system,
    void* buffer);
OrbisBufferDescriptor* orbis_identity_instance_descriptors(
    OrbisRenderSystem& system);
const OrbisBufferDescriptor* orbis_identity_instance_descriptors();
void orbis_set_identity_instance_buffer(
    OrbisRenderSystem& system,
    void* buffer);
std::size_t orbis_active_render_frame_index();
void orbis_render_system_publish_instance(OrbisRenderSystem& system);
void orbis_render_system_clear_instance();

}  // namespace rb4
