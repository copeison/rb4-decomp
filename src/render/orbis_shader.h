#pragma once

#include <cstdint>

namespace rb4 {

enum class RenderShaderStage : std::uint32_t {
    kVertex = 0,
    kHull = 1,
    kDomain = 2,
    kGeometry = 3,
    kPixel = 4,
    kCompute = 5,
};

struct OrbisShader;

OrbisShader* orbis_create_shader(RenderShaderStage stage);
void orbis_vertex_shader_construct(OrbisShader& shader);
void orbis_geometry_shader_construct(OrbisShader& shader);
void orbis_pixel_shader_construct(OrbisShader& shader);
void orbis_compute_shader_construct(OrbisShader& shader);

}  // namespace rb4
