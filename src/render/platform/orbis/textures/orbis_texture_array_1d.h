#pragma once

#include <cstdint>

#include "render/core/render_texture_array_1d.h"
#include "render/platform/orbis/shaders/orbis_texture_binding.h"

namespace rb4 {

struct OrbisRenderContext;

struct OrbisTextureArray1D : RenderTextureArray1D {
    void* gpu_texture;
    void* allocation;
};

static_assert(sizeof(OrbisTextureArray1D) == 360);

OrbisTextureArray1D* orbis_create_texture_array_1d(
    const RenderTextureArray1DDescriptor& descriptor);
void orbis_texture_array_1d_construct(
    OrbisTextureArray1D& texture,
    const RenderTextureArray1DDescriptor& descriptor);
void orbis_texture_array_1d_destruct(OrbisTextureArray1D& texture);
void orbis_texture_array_1d_delete(OrbisTextureArray1D& texture);
void orbis_texture_array_1d_initialize_backend(
    OrbisTextureArray1D& texture);
void orbis_texture_array_1d_bind_vertex(
    const OrbisTextureArray1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_1d_bind_hull(
    const OrbisTextureArray1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_1d_bind_domain(
    const OrbisTextureArray1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_1d_bind_geometry(
    const OrbisTextureArray1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_1d_bind_pixel(
    const OrbisTextureArray1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_array_1d_bind_compute(
    const OrbisTextureArray1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);

}  // namespace rb4
