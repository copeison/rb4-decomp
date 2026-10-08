#pragma once

#include <cstdint>

#include "orbis_shader_state.h"

namespace rb4 {

struct OrbisRenderContext;

struct OrbisSamplerDescriptor {
    std::uint32_t registers[4];
};

void orbis_render_context_bind_graphics_sampler(
    OrbisRenderContext& context,
    RenderShaderStage stage,
    std::uint32_t slot,
    const OrbisSamplerDescriptor& sampler);
void orbis_render_context_bind_compute_sampler(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const OrbisSamplerDescriptor& sampler);
void orbis_render_context_clear_vertex_shader(OrbisRenderContext& context);
void orbis_render_context_clear_geometry_shader(OrbisRenderContext& context);
void orbis_render_context_clear_pixel_shader(OrbisRenderContext& context);
void orbis_render_context_clear_compute_shader(OrbisRenderContext& context);

}  // namespace rb4
