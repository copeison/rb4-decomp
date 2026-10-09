#pragma once

#include <cstdint>

#include "render/platform/orbis/shaders/orbis_shader_state.h"

class PS4Context;

namespace rb4 {

enum class OrbisGnmShaderStage : std::uint32_t {
    kCompute = 0,
    kPixel = 1,
    kVertex = 2,
    kGeometry = 3,
    kHull = 5,
    kLocal = 6,
};

bool orbis_render_context_uses_compute_queue(
    const PS4Context& context);
void orbis_bind_graphics_texture(
    PS4Context& context,
    OrbisGnmShaderStage stage,
    std::uint32_t slot,
    const void* texture);
void orbis_bind_graphics_rw_texture(
    PS4Context& context,
    OrbisGnmShaderStage stage,
    std::uint32_t slot,
    const void* texture);
void orbis_bind_compute_texture_descriptor(
    PS4Context& context,
    std::uint32_t slot,
    const void* texture);
void orbis_bind_compute_rw_texture(
    PS4Context& context,
    std::uint32_t slot,
    const void* texture);
void orbis_bind_graphics_texture_sampler(
    PS4Context& context,
    OrbisGnmShaderStage stage,
    std::uint32_t slot,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode);
void orbis_bind_compute_texture_sampler(
    PS4Context& context,
    std::uint32_t slot,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode);

}  // namespace rb4
