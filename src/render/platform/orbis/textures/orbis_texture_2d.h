#pragma once

#include <cstdint>

#include "render/core/textures/render_texture_2d.h"
#include "render/platform/orbis/shaders/orbis_texture_binding.h"

namespace rb4 {

struct OrbisRenderContext;
struct OrbisGpuDepthRenderTarget;
struct OrbisGpuRenderTarget;

struct OrbisTexture2D : RenderTexture2D {
    std::uint8_t backend_state[112];
};

static_assert(sizeof(OrbisTexture2D) == 520);

OrbisTexture2D* orbis_create_texture_2d(
    const RenderTexture2DDescriptor& descriptor);
void orbis_texture_2d_construct(
    OrbisTexture2D& texture,
    const RenderTexture2DDescriptor& descriptor);
void orbis_texture_2d_destruct(OrbisTexture2D& texture);
void orbis_texture_2d_delete(OrbisTexture2D& texture);
void orbis_texture_2d_initialize_backend(
    OrbisTexture2D& texture,
    const OrbisTexture2D* storage_source);
void orbis_texture_2d_update_gpu_data(OrbisTexture2D& texture);
const OrbisGpuRenderTarget* orbis_texture_2d_render_target(
    const OrbisTexture2D& texture);
const OrbisGpuDepthRenderTarget* orbis_texture_2d_depth_target(
    const OrbisTexture2D& texture);
void orbis_texture_2d_bind_vertex(
    const OrbisTexture2D& texture,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_2d_bind_hull(
    const OrbisTexture2D& texture,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_2d_bind_domain(
    const OrbisTexture2D& texture,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_2d_bind_geometry(
    const OrbisTexture2D& texture,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_2d_bind_pixel(
    const OrbisTexture2D& texture,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_2d_bind_compute(
    const OrbisTexture2D& texture,
    OrbisRenderContext& context,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);

}  // namespace rb4
