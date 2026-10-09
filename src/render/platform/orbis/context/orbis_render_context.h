#pragma once

#include <cstddef>
#include <cstdint>

class PS4Context;

namespace rb4 {

struct OrbisRenderSystem;
struct OrbisRenderCommandContext;
struct OrbisTransientVertexBuffer;

enum class RndMaterialBlendMode : std::int32_t;

constexpr std::size_t kOrbisFrameSlotCount = 2;
constexpr std::size_t kOrbisComputeContextCount = 18;
constexpr std::size_t kOrbisComputeContextsPerFrame = 9;
constexpr std::size_t kOrbisTransientFormatCount = 8;

PS4Context* orbis_render_context_create(
    OrbisRenderSystem& system);
void orbis_render_context_create_gfx_contexts(
    PS4Context& context);
void orbis_render_context_create_gpu_timestamp_pool(
    PS4Context& context);
bool orbis_render_context_compute_queues_enabled(
    const PS4Context& context);
std::size_t orbis_render_context_active_frame(
    const PS4Context& context);
OrbisTransientVertexBuffer& orbis_render_context_transient_vertex_buffer(
    PS4Context& context,
    std::size_t frame,
    std::size_t format);
OrbisRenderCommandContext& orbis_active_render_command_context(
    PS4Context& context);
bool orbis_render_context_submissions_complete(
    const PS4Context& context);
bool orbis_render_context_frame_submissions_complete(
    const PS4Context& context,
    std::size_t frame);
void orbis_render_context_mark_compute_completion_pending(
    PS4Context& context,
    std::size_t frame,
    std::size_t slot);
void orbis_render_context_mark_gfx_completion_pending(
    PS4Context& context,
    std::size_t frame);
void orbis_render_context_set_active_frame(
    PS4Context& context,
    std::size_t frame);
void orbis_render_context_submit_frame(PS4Context& context);
void orbis_render_context_reset_active_frame(PS4Context& context);

}  // namespace rb4
