#pragma once

#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/context/orbis_render_context.h"
#include "renderps4/context/PS4Context.h"

class PS4Context;

namespace rb4 {

struct OrbisGpuDepthRenderTarget;
struct OrbisGpuRenderTarget;

struct OrbisViewportRect {
    float x;
    float y;
    float width;
    float height;
};

const RndContext::RenderTargetParams& orbis_default_render_target_binding();
const RndContext::BlendParams& orbis_default_blend_configuration();
std::size_t orbis_render_target_color_count(
    const RndContext::RenderTargetParams& binding);
const OrbisGpuRenderTarget* orbis_resolve_color_render_target(
    const RndContext::RenderTargetParams& binding,
    std::int32_t target_kind,
    std::size_t slot);
const OrbisGpuDepthRenderTarget* orbis_resolve_depth_render_target(
    const RndContext::RenderTargetParams& binding,
    std::int32_t target_kind);
OrbisViewportRect orbis_render_target_viewport(
    const RndContext::RenderTargetParams& binding);
bool orbis_color_render_target_requires_sync(
    const RndContext::RenderTargetParams& binding,
    std::size_t slot);
bool orbis_depth_render_target_requires_prepare(
    const RndContext::RenderTargetParams& binding);
void orbis_render_context_bind_color_target(
    PS4Context& context,
    std::size_t slot,
    const OrbisGpuRenderTarget* target);
void orbis_render_context_bind_depth_target(
    PS4Context& context,
    const OrbisGpuDepthRenderTarget* target);
void orbis_render_context_set_viewport_and_scissor(
    PS4Context& context,
    const OrbisViewportRect& viewport);
void orbis_render_context_begin_render_target_sync(
    PS4Context& context);
void orbis_render_context_prepare_color_target(
    PS4Context& context,
    const RndContext::RenderTargetParams& binding,
    std::size_t slot);
bool orbis_render_context_prepare_depth_target(
    PS4Context& context,
    const OrbisGpuDepthRenderTarget& target,
    const RndContext::RenderTargetParams& binding);
void orbis_render_context_finish_render_target_sync(
    PS4Context& context);
std::uint32_t orbis_build_blend_control(
    RndMaterialBlendMode mode,
    const RndContext::BlendParams& configuration,
    std::size_t target_slot);
void orbis_render_context_set_gnm_blend_control(
    PS4Context& context,
    std::size_t target_slot,
    std::uint32_t blend_control);
void orbis_render_context_reset_cached_pipeline_state(
    PS4Context& context);
void orbis_render_context_set_default_raster_state(
    PS4Context& context);
void orbis_render_context_set_default_depth_stencil_state(
    PS4Context& context);
void orbis_render_context_disable_stream_output(
    PS4Context& context);
void orbis_render_context_clear_shader_resources(
    PS4Context& context);
void orbis_render_context_cache_depth_mode(
    PS4Context& context,
    std::uint32_t depth_mode);
void orbis_render_context_cache_stencil_state(
    PS4Context& context,
    std::uint32_t stencil_mode,
    std::uint8_t reference,
    std::uint8_t read_mask,
    std::uint8_t write_mask);
void orbis_render_context_apply_depth_stencil_state(
    PS4Context& context);
void orbis_render_context_cache_front_face(
    PS4Context& context,
    bool counter_clockwise);
void orbis_render_context_cache_cull_mode(
    PS4Context& context,
    RndCullMode cull_mode);
void orbis_render_context_cache_polygon_fill(
    PS4Context& context,
    bool enabled);
void orbis_render_context_apply_primitive_setup(
    PS4Context& context);
void orbis_render_context_set_gnm_render_target_mask(
    PS4Context& context,
    std::uint32_t write_mask);
void orbis_render_context_cache_color_write_mask(
    PS4Context& context,
    std::uint8_t target_mask,
    RndWriteMaskChannelSet write_mode);

}  // namespace rb4
