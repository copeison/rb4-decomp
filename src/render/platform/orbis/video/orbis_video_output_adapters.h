#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisRenderSystem;
struct OrbisVertexBuffer;

enum class OrbisSubmitEventType {
    kFlipComplete,
    kEndOfPipe,
};

struct OrbisSubmitEvent {
    OrbisSubmitEventType type;
};

using OrbisSubmitThreadEntry = void (*)(OrbisRenderSystem& system);

void orbis_register_render_factories(OrbisRenderSystem& system);

void orbis_start_submit_thread(
    OrbisRenderSystem& system,
    OrbisSubmitThreadEntry entry,
    const char* name,
    std::uint32_t priority);
void orbis_initialize_submit_profiler(OrbisRenderSystem& system);
void orbis_join_submit_thread(OrbisRenderSystem& system);
void orbis_release_frame_runtime(OrbisRenderSystem& system);
OrbisVertexBuffer& orbis_allocate_default_vertex_buffer(
    OrbisRenderSystem& system,
    const char* name);
void orbis_upload_default_vertex_data(
    OrbisRenderSystem& system,
    OrbisVertexBuffer& buffer);
void orbis_bind_default_vertex_buffer(
    OrbisRenderSystem& system,
    OrbisVertexBuffer& buffer);

OrbisVertexBuffer& orbis_allocate_identity_instance_buffer(
    OrbisRenderSystem& system,
    const char* name,
    std::size_t size);
void orbis_upload_identity_instance_data(
    OrbisRenderSystem& system,
    OrbisVertexBuffer& buffer);
void orbis_bind_identity_instance_buffer(
    OrbisRenderSystem& system,
    OrbisVertexBuffer& buffer);

void orbis_process_flip_complete(
    OrbisRenderSystem& system,
    const OrbisSubmitEvent& event);
void orbis_process_end_of_pipe(
    OrbisRenderSystem& system,
    const OrbisSubmitEvent& event);

void render_free(void* allocation);

}  // namespace rb4
