#pragma once

#include <cstdint>

#include "render/platform/orbis/shaders/orbis_shader.h"

namespace rb4 {

struct OrbisRenderContext;

enum class OrbisSamplerAddressMode : std::uint32_t {
    kClamp = 1,
    kWrap = 2,
    kClampOpaqueBlack = 3,
    kClampOpaqueWhite = 4,
    kMirror = 5,
};

void orbis_render_context_set_sampler(
    OrbisRenderContext& context,
    RenderShaderStage stage,
    std::uint32_t slot,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode);
void orbis_render_context_clear_shader(
    OrbisRenderContext& context,
    RenderShaderStage stage);
void orbis_render_context_clear_rw_resources(
    OrbisRenderContext& context,
    std::uint32_t stage_mask);
void orbis_render_context_clear_read_resources(
    OrbisRenderContext& context,
    std::uint32_t stage_mask);

}  // namespace rb4
