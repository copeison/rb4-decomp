#pragma once

#include <cstddef>

#include "render/platform/orbis/shaders/orbis_shader.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void shader_construct(OrbisShader& shader);
void orbis_vertex_shader_set_backend_defaults(OrbisShader& shader);
void orbis_geometry_shader_set_backend_defaults(OrbisShader& shader);
void orbis_pixel_shader_set_backend_defaults(OrbisShader& shader);
void orbis_compute_shader_set_backend_defaults(OrbisShader& shader);
void shader_destruct(OrbisShader& shader);
void render_delete_shader(OrbisShader& shader);
bool orbis_compute_shader_load_binary(
    OrbisShader& shader,
    const OrbisShaderBinary& binary);
void orbis_compute_shader_bind_backend(
    const OrbisShader& shader,
    OrbisRenderContext& context);
void orbis_compute_shader_release_allocations(OrbisShader& shader);
bool orbis_pixel_shader_load_binary(
    OrbisShader& shader,
    const OrbisShaderBinary& binary);
void orbis_pixel_shader_bind_backend(
    const OrbisShader& shader,
    OrbisRenderContext& context);
void orbis_pixel_shader_release_allocations(OrbisShader& shader);

}  // namespace rb4
