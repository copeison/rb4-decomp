#pragma once

#include <cstdint>

#include "render/platform/orbis/shaders/orbis_shader_state.h"
#include "render/shaders/RndShaderEnums.h"

class PS4Context;

namespace rb4 {

void orbis_render_context_bind_graphics_sampler(
    PS4Context& context,
    RndShaderProgramType stage,
    std::uint32_t slot,
    const OrbisSamplerDescriptor& sampler);
void orbis_render_context_bind_compute_sampler(
    PS4Context& context,
    std::uint32_t slot,
    const OrbisSamplerDescriptor& sampler);
void orbis_render_context_clear_vertex_shader(PS4Context& context);
void orbis_render_context_clear_geometry_shader(PS4Context& context);
void orbis_render_context_clear_pixel_shader(PS4Context& context);
void orbis_render_context_clear_compute_shader(PS4Context& context);
bool orbis_render_context_graphics_resources_active(
    const PS4Context& context);
void orbis_render_context_clear_gnm_rw_textures(
    PS4Context& context,
    RndShaderProgramType stage);
void orbis_render_context_clear_gnm_textures(
    PS4Context& context,
    RndShaderProgramType stage);
void orbis_render_context_clear_gnm_buffers(
    PS4Context& context,
    RndShaderProgramType stage);

}  // namespace rb4
