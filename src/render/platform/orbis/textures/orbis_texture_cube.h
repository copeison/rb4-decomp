#pragma once

#include <cstdint>

#include "render/core/textures/render_texture_cube.h"
#include "render/platform/orbis/shaders/orbis_texture_binding.h"

namespace rb4 {

struct OrbisRenderContext;
struct OrbisGpuDepthRenderTarget;
struct OrbisGpuRenderTarget;

struct OrbisTextureCube : RenderTextureCube {
    void* gpu_texture;
    void* primary_allocation;
    void* secondary_allocation;
    OrbisGpuRenderTarget* render_target;
    OrbisGpuDepthRenderTarget* depth_target;
};

static_assert(sizeof(OrbisTextureCube) == 832);

OrbisTextureCube* orbis_create_texture_cube(
    const RenderTextureCubeDescriptor& descriptor);
void orbis_texture_cube_construct(
    OrbisTextureCube& texture,
    const RenderTextureCubeDescriptor& descriptor);
void orbis_texture_cube_destruct(OrbisTextureCube& texture);
void orbis_texture_cube_delete(OrbisTextureCube& texture);
void orbis_texture_cube_initialize_backend(OrbisTextureCube& texture);
const OrbisGpuRenderTarget* orbis_texture_cube_render_target(
    const OrbisTextureCube& texture);
const OrbisGpuDepthRenderTarget* orbis_texture_cube_depth_target(
    const OrbisTextureCube& texture);
void orbis_texture_cube_bind_vertex(
    const OrbisTextureCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_cube_bind_hull(
    const OrbisTextureCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_cube_bind_domain(
    const OrbisTextureCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_cube_bind_geometry(
    const OrbisTextureCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_cube_bind_pixel(
    const OrbisTextureCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_cube_bind_compute(
    const OrbisTextureCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);

}  // namespace rb4
