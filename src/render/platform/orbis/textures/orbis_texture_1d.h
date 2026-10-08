#pragma once

#include <cstdint>

#include "render/platform/orbis/shaders/orbis_texture_binding.h"

namespace rb4 {

struct OrbisTexture1D;
struct OrbisRenderContext;
struct RenderTexture1DDescriptor;

OrbisTexture1D* orbis_create_texture_1d(
    const RenderTexture1DDescriptor& descriptor);
void orbis_texture_1d_construct(
    OrbisTexture1D& texture,
    const RenderTexture1DDescriptor& descriptor);
void orbis_texture_1d_destruct(OrbisTexture1D& texture);
void orbis_texture_1d_delete(OrbisTexture1D& texture);
void orbis_texture_1d_initialize_backend(OrbisTexture1D& texture);
void orbis_texture_1d_bind_vertex(
    const OrbisTexture1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_1d_bind_hull(
    const OrbisTexture1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_1d_bind_domain(
    const OrbisTexture1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_1d_bind_geometry(
    const OrbisTexture1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_1d_bind_pixel(
    const OrbisTexture1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_texture_1d_bind_compute(
    const OrbisTexture1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);

}  // namespace rb4
