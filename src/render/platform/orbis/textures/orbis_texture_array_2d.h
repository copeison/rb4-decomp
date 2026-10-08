#pragma once

#include <cstdint>

#include "render/core/render_texture_array_2d.h"
#include "render/platform/orbis/shaders/orbis_texture_binding.h"

namespace rb4 {

struct OrbisRenderContext;

struct OrbisTextureArray2D : RenderTextureArray2D {
    void* gpu_texture;
    void* primary_allocation;
    void* stencil_allocation;
    void* metadata_allocation;
    void* color_target;
    void* depth_target;
};

static_assert(sizeof(OrbisTextureArray2D) == 392);

OrbisTextureArray2D* orbis_create_texture_array_2d(
    const RenderTextureArray2DDescriptor& descriptor);
void orbis_texture_array_2d_construct(
    OrbisTextureArray2D& texture,
    const RenderTextureArray2DDescriptor& descriptor);
void orbis_texture_array_2d_destruct(OrbisTextureArray2D& texture);
void orbis_texture_array_2d_delete(OrbisTextureArray2D& texture);
void orbis_texture_array_2d_initialize_backend(
    OrbisTextureArray2D& texture);
void orbis_texture_array_2d_bind_vertex(
    const OrbisTextureArray2D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_2d_bind_hull(
    const OrbisTextureArray2D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_2d_bind_domain(
    const OrbisTextureArray2D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_2d_bind_geometry(
    const OrbisTextureArray2D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_2d_bind_pixel(
    const OrbisTextureArray2D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_2d_bind_compute(
    const OrbisTextureArray2D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);

}  // namespace rb4
