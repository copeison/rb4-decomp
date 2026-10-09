#include "render/platform/orbis/shaders/orbis_shader_state.h"
#include "render/shaders/RndShaderEnums.h"

#include <cstdint>

#include "render/platform/orbis/shaders/orbis_shader_state_adapters.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kShaderStageCount = 6;

OrbisSamplerDescriptor build_sampler_descriptor(
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode) {
    OrbisSamplerDescriptor sampler = {
        {0, 0x00FFF000, 0x05000000, 0},
    };

    switch (address_mode) {
    case OrbisSamplerAddressMode::kClamp:
        sampler.registers[0] = 0x92;
        break;
    case OrbisSamplerAddressMode::kWrap:
        break;
    case OrbisSamplerAddressMode::kClampOpaqueBlack:
        sampler.registers[0] = 0x1B6;
        sampler.registers[3] = 0x40000000;
        break;
    case OrbisSamplerAddressMode::kClampOpaqueWhite:
        sampler.registers[0] = 0x1B6;
        sampler.registers[3] = 0x80000000;
        break;
    case OrbisSamplerAddressMode::kMirror:
        sampler.registers[0] = 0x49;
        break;
    }

    switch (filter_mode) {
    case 1:
        sampler.registers[2] = 0x05000000;
        break;
    case 2:
        sampler.registers[2] = 0x06500000;
        break;
    case 3:
        sampler.registers[2] = 0x0A500000;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        sampler.registers[0] |= (filter_mode - 3) << 9;
        sampler.registers[2] = 0x0AF00000;
        break;
    default:
        break;
    }
    return sampler;
}

}  // namespace

// Reconstructed from eboot.elf at 0x8EA830.
void orbis_render_context_set_sampler(
    OrbisRenderContext& context,
    RndShaderProgramType stage,
    std::uint32_t slot,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode) {
    const auto sampler = build_sampler_descriptor(address_mode, filter_mode);
    switch (stage) {
    case kShaderProgramVertex:
    case kShaderProgramPixel:
        orbis_render_context_bind_graphics_sampler(
            context, stage, slot, sampler);
        break;
    case kShaderProgramCompute:
        orbis_render_context_bind_compute_sampler(context, slot, sampler);
        break;
    case kShaderProgramHull:
    case kShaderProgramDomain:
    case kShaderProgramGeometry:
        break;
    }
}

// Reconstructed from eboot.elf at 0x8EA920.
void orbis_render_context_clear_shader(
    OrbisRenderContext& context,
    RndShaderProgramType stage) {
    switch (stage) {
    case kShaderProgramVertex:
        orbis_render_context_clear_vertex_shader(context);
        break;
    case kShaderProgramGeometry:
        orbis_render_context_clear_geometry_shader(context);
        break;
    case kShaderProgramPixel:
        orbis_render_context_clear_pixel_shader(context);
        break;
    case kShaderProgramCompute:
        orbis_render_context_clear_compute_shader(context);
        break;
    case kShaderProgramHull:
    case kShaderProgramDomain:
        break;
    }
}

// Reconstructed from eboot.elf at 0x8E9810.
void orbis_render_context_clear_rw_resources(
    OrbisRenderContext& context,
    std::uint32_t stage_mask) {
    if (!orbis_render_context_graphics_resources_active(context)) {
        return;
    }

    for (std::uint32_t index = 0; index < kShaderStageCount; ++index) {
        if ((stage_mask & (1U << index)) == 0) {
            continue;
        }
        const auto stage = static_cast<RndShaderProgramType>(index);
        orbis_render_context_clear_gnm_rw_textures(context, stage);
    }
}

// Reconstructed from eboot.elf at 0x8E9940.
void orbis_render_context_clear_read_resources(
    OrbisRenderContext& context,
    std::uint32_t stage_mask) {
    if (!orbis_render_context_graphics_resources_active(context)) {
        return;
    }

    for (std::uint32_t index = 0; index < kShaderStageCount; ++index) {
        if ((stage_mask & (1U << index)) == 0) {
            continue;
        }
        const auto stage = static_cast<RndShaderProgramType>(index);
        orbis_render_context_clear_gnm_textures(context, stage);
        orbis_render_context_clear_gnm_buffers(context, stage);
    }
}

}  // namespace rb4
