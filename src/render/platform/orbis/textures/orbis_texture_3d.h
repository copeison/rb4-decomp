#pragma once

#include <cstdint>

#include "render/core/textures/render_texture_3d.h"
#include "render/platform/orbis/shaders/orbis_texture_binding.h"

namespace rb4 {

struct OrbisRenderContext;

struct OrbisTexture3D : RenderTexture3D {
    void* gpu_texture;
    void* allocation;
};

static_assert(sizeof(OrbisTexture3D) == 408);

OrbisTexture3D* orbis_create_texture_3d(
    const RenderTexture3DDescriptor& descriptor);
void orbis_texture_3d_construct(
    OrbisTexture3D& texture,
    const RenderTexture3DDescriptor& descriptor);
void orbis_texture_3d_destruct(OrbisTexture3D& texture);
void orbis_texture_3d_delete(OrbisTexture3D& texture);
void orbis_texture_3d_initialize_backend(
    OrbisTexture3D& texture,
    const OrbisTexture3D* storage_source);
void orbis_texture_3d_bind_vertex(
    const OrbisTexture3D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_3d_bind_hull(
    const OrbisTexture3D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_3d_bind_domain(
    const OrbisTexture3D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_3d_bind_geometry(
    const OrbisTexture3D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_3d_bind_pixel(
    const OrbisTexture3D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_3d_bind_compute(
    const OrbisTexture3D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);

}  // namespace rb4
