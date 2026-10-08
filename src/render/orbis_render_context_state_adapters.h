#pragma once

#include <cstddef>
#include <cstdint>

#include "orbis_render_context.h"

namespace rb4 {

struct OrbisGpuDepthRenderTarget;
struct OrbisGpuRenderTarget;

struct OrbisViewportRect {
    float x;
    float y;
    float width;
    float height;
};

const OrbisRenderTargetBinding& orbis_default_render_target_binding();
const OrbisBlendConfiguration& orbis_default_blend_configuration();
std::size_t orbis_render_target_color_count(
    const OrbisRenderTargetBinding& binding);
const OrbisGpuRenderTarget* orbis_resolve_color_render_target(
    const OrbisRenderTargetBinding& binding,
    std::int32_t target_kind,
    std::size_t slot);
const OrbisGpuDepthRenderTarget* orbis_resolve_depth_render_target(
    const OrbisRenderTargetBinding& binding,
    std::int32_t target_kind);
OrbisViewportRect orbis_render_target_viewport(
    const OrbisRenderTargetBinding& binding);
bool orbis_color_render_target_requires_sync(
    const OrbisRenderTargetBinding& binding,
    std::size_t slot);
bool orbis_depth_render_target_requires_prepare(
    const OrbisRenderTargetBinding& binding);
void orbis_render_context_bind_color_target(
    OrbisRenderContext& context,
    std::size_t slot,
    const OrbisGpuRenderTarget* target);
void orbis_render_context_bind_depth_target(
    OrbisRenderContext& context,
    const OrbisGpuDepthRenderTarget* target);
void orbis_render_context_set_viewport_and_scissor(
    OrbisRenderContext& context,
    const OrbisViewportRect& viewport);
void orbis_render_context_begin_render_target_sync(
    OrbisRenderContext& context);
void orbis_render_context_prepare_color_target(
    OrbisRenderContext& context,
    const OrbisRenderTargetBinding& binding,
    std::size_t slot);
bool orbis_render_context_prepare_depth_target(
    OrbisRenderContext& context,
    const OrbisGpuDepthRenderTarget& target,
    const OrbisRenderTargetBinding& binding);
void orbis_render_context_finish_render_target_sync(
    OrbisRenderContext& context);
std::uint32_t orbis_build_blend_control(
    RndMaterialBlendMode mode,
    const OrbisBlendConfiguration& configuration,
    std::size_t target_slot);
void orbis_render_context_set_gnm_blend_control(
    OrbisRenderContext& context,
    std::size_t target_slot,
    std::uint32_t blend_control);
void orbis_render_context_reset_cached_pipeline_state(
    OrbisRenderContext& context);
void orbis_render_context_set_default_raster_state(
    OrbisRenderContext& context);
void orbis_render_context_set_default_depth_stencil_state(
    OrbisRenderContext& context);
void orbis_render_context_disable_stream_output(
    OrbisRenderContext& context);
void orbis_render_context_clear_shader_resources(
    OrbisRenderContext& context);
void orbis_render_context_cache_depth_mode(
    OrbisRenderContext& context,
    std::uint32_t depth_mode);
void orbis_render_context_cache_stencil_state(
    OrbisRenderContext& context,
    std::uint32_t stencil_mode,
    std::uint8_t reference,
    std::uint8_t read_mask,
    std::uint8_t write_mask);
void orbis_render_context_apply_depth_stencil_state(
    OrbisRenderContext& context);
void orbis_render_context_cache_front_face(
    OrbisRenderContext& context,
    bool counter_clockwise);
void orbis_render_context_cache_cull_mode(
    OrbisRenderContext& context,
    OrbisCullMode cull_mode);
void orbis_render_context_cache_polygon_fill(
    OrbisRenderContext& context,
    bool enabled);
void orbis_render_context_apply_primitive_setup(
    OrbisRenderContext& context);
void orbis_render_context_set_gnm_render_target_mask(
    OrbisRenderContext& context,
    std::uint32_t write_mask);
void orbis_render_context_cache_color_write_mask(
    OrbisRenderContext& context,
    std::uint8_t target_mask,
    OrbisColorWriteMode write_mode);

}  // namespace rb4
