#pragma once

#include <cstddef>

#include "orbis_shader.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void shader_construct(OrbisShader& shader);
void orbis_vertex_shader_set_backend_defaults(OrbisShader& shader);
void orbis_geometry_shader_set_backend_defaults(OrbisShader& shader);
void orbis_pixel_shader_set_backend_defaults(OrbisShader& shader);
void orbis_compute_shader_set_backend_defaults(OrbisShader& shader);

}  // namespace rb4
