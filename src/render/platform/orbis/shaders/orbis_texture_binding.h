#pragma once

#include <cstdint>

#include "render/platform/orbis/shaders/orbis_shader_state.h"

namespace rb4 {

struct OrbisRenderContext;

struct OrbisSamplerBorderColor {
    float channels[4];
};

enum OrbisTextureBindingFlag : std::uint32_t {
    kOrbisTextureBindingWritable = 1U << 0,
    kOrbisTextureBindingSuppressSampler = 1U << 4,
};

void orbis_bind_vertex_texture(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    const OrbisSamplerBorderColor& border_color);
void orbis_bind_hull_texture(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_bind_domain_texture(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_bind_geometry_texture(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_bind_pixel_texture(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);
void orbis_bind_compute_texture(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color);

}  // namespace rb4
