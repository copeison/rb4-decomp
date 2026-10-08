#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

struct OrbisRenderSystem;
struct OrbisRenderContext;
struct OrbisBlendConfiguration;
struct OrbisRenderTargetBinding;

enum class RndMaterialBlendMode : std::int32_t;

enum class OrbisCullMode : std::uint32_t {
    kNone = 0,
    kBack = 1,
    kFront = 2,
};

enum class OrbisColorWriteMode : std::uint32_t {
    kRgba = 0,
    kRgb = 1,
    kDisabled = 2,
};

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
std::size_t orbis_render_context_active_frame(
    const OrbisRenderContext& context);
bool orbis_render_context_submissions_complete(
    const OrbisRenderContext& context);
void orbis_render_context_set_active_frame(
    OrbisRenderContext& context,
    std::size_t frame);
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
void orbis_render_context_set_depth_mode(
    OrbisRenderContext& context,
    std::uint32_t depth_mode);
void orbis_render_context_set_stencil_state(
    OrbisRenderContext& context,
    std::uint32_t stencil_mode,
    std::uint8_t reference,
    std::uint32_t read_mask,
    std::uint32_t write_mask);
void orbis_render_context_set_front_face(
    OrbisRenderContext& context,
    bool counter_clockwise);
void orbis_render_context_set_cull_mode(
    OrbisRenderContext& context,
    OrbisCullMode cull_mode);
void orbis_render_context_set_polygon_fill(
    OrbisRenderContext& context,
    bool enabled);
void orbis_render_context_set_color_write_mask(
    OrbisRenderContext& context,
    std::uint8_t target_mask,
    OrbisColorWriteMode write_mode);

}  // namespace rb4
