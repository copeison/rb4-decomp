#pragma once

#include "render/core/shaders/render_shader.h"

namespace rb4 {

struct OrbisShader : RenderShader {};
struct OrbisRenderContext;
using OrbisShaderBinary = RenderShaderBinary;

OrbisShader* orbis_create_shader(RenderShaderStage stage);
void orbis_vertex_shader_construct(OrbisShader& shader);
void orbis_geometry_shader_construct(OrbisShader& shader);
void orbis_pixel_shader_construct(OrbisShader& shader);
void orbis_compute_shader_construct(OrbisShader& shader);
void orbis_compute_shader_destruct(OrbisShader& shader);
void orbis_compute_shader_delete(OrbisShader& shader);
bool orbis_compute_shader_initialize(
    OrbisShader& shader,
    const OrbisShaderBinary& binary);
void orbis_compute_shader_bind(
    const OrbisShader& shader,
    OrbisRenderContext& context);
void orbis_compute_shader_release_backend(OrbisShader& shader);
RenderShaderStage orbis_compute_shader_stage();
void orbis_pixel_shader_destruct(OrbisShader& shader);
void orbis_pixel_shader_delete(OrbisShader& shader);
bool orbis_pixel_shader_initialize(
    OrbisShader& shader,
    const OrbisShaderBinary& binary);
void orbis_pixel_shader_bind(
    const OrbisShader& shader,
    OrbisRenderContext& context);
void orbis_pixel_shader_release_backend(OrbisShader& shader);
RenderShaderStage orbis_pixel_shader_stage();
void orbis_geometry_shader_destruct(OrbisShader& shader);
void orbis_geometry_shader_delete(OrbisShader& shader);
bool orbis_geometry_shader_initialize(
    OrbisShader& shader,
    const OrbisShaderBinary& binary);
void orbis_geometry_shader_bind(
    const OrbisShader& shader,
    OrbisRenderContext& context);
void orbis_geometry_shader_release_backend(OrbisShader& shader);
RenderShaderStage orbis_geometry_shader_stage();
void orbis_vertex_shader_destruct(OrbisShader& shader);
void orbis_vertex_shader_delete(OrbisShader& shader);
bool orbis_vertex_shader_initialize(
    OrbisShader& shader,
    const OrbisShaderBinary& binary);
void orbis_vertex_shader_bind(
    const OrbisShader& shader,
    OrbisRenderContext& context);
void orbis_vertex_shader_release_backend(OrbisShader& shader);
RenderShaderStage orbis_vertex_shader_stage();

}  // namespace rb4
