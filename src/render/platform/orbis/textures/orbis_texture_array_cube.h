#pragma once

#include <cstdint>

#include "render/platform/orbis/shaders/orbis_texture_binding.h"

namespace rb4 {

struct OrbisTextureArrayCube;
struct OrbisRenderContext;
struct RenderTextureArrayCubeDescriptor;

OrbisTextureArrayCube* orbis_create_texture_array_cube(
    const RenderTextureArrayCubeDescriptor& descriptor);
void orbis_texture_array_cube_construct(
    OrbisTextureArrayCube& texture,
    const RenderTextureArrayCubeDescriptor& descriptor);
void orbis_texture_array_cube_destruct(OrbisTextureArrayCube& texture);
void orbis_texture_array_cube_delete(OrbisTextureArrayCube& texture);
void orbis_texture_array_cube_initialize_backend(
    OrbisTextureArrayCube& texture);
void orbis_texture_array_cube_bind_vertex(
    const OrbisTextureArrayCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_cube_bind_hull(
    const OrbisTextureArrayCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_cube_bind_domain(
    const OrbisTextureArrayCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_cube_bind_geometry(
    const OrbisTextureArrayCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_cube_bind_pixel(
    const OrbisTextureArrayCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_cube_bind_compute(
    const OrbisTextureArrayCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);

}  // namespace rb4
