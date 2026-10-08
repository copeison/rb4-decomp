#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisRenderSystem;
struct OrbisRenderContext;
struct OrbisBlendConfiguration;
struct OrbisRenderTargetBinding;

enum class RndMaterialBlendMode : std::int32_t;

constexpr std::size_t kOrbisFrameSlotCount = 2;
constexpr std::size_t kOrbisComputeContextCount = 18;
constexpr std::size_t kOrbisComputeContextsPerFrame = 9;
constexpr std::size_t kOrbisTransientFormatCount = 8;

OrbisRenderContext* orbis_render_context_create(
    OrbisRenderSystem& system);
void orbis_render_context_construct(OrbisRenderContext& context);
void orbis_render_context_destruct(OrbisRenderContext& context);
void orbis_render_context_delete(OrbisRenderContext& context);
void orbis_render_context_create_gfx_contexts(
    OrbisRenderContext& context);
void orbis_render_context_create_gpu_timestamp_pool(
    OrbisRenderContext& context);
void orbis_render_context_submit_frame(OrbisRenderContext& context);
void orbis_render_context_reset_active_frame(OrbisRenderContext& context);
void orbis_render_context_reset_pipeline_state(OrbisRenderContext& context);
void orbis_render_context_bind_render_targets(
    OrbisRenderContext& context,
    std::int32_t target_kind,
    const OrbisRenderTargetBinding& binding);
void orbis_render_context_set_blend_mode(
    OrbisRenderContext& context,
    RndMaterialBlendMode mode,
    const OrbisBlendConfiguration& configuration);

}  // namespace rb4
