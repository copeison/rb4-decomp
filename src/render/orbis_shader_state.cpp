#include "orbis_shader_state.h"

#include <cstdint>

#include "orbis_shader_state_adapters.h"

namespace rb4 {

namespace {

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
    RenderShaderStage stage,
    std::uint32_t slot,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode) {
    const auto sampler = build_sampler_descriptor(address_mode, filter_mode);
    switch (stage) {
    case RenderShaderStage::kVertex:
    case RenderShaderStage::kPixel:
        orbis_render_context_bind_graphics_sampler(
            context, stage, slot, sampler);
        break;
    case RenderShaderStage::kCompute:
        orbis_render_context_bind_compute_sampler(context, slot, sampler);
        break;
    case RenderShaderStage::kHull:
    case RenderShaderStage::kDomain:
    case RenderShaderStage::kGeometry:
        break;
    }
}

// Reconstructed from eboot.elf at 0x8EA920.
void orbis_render_context_clear_shader(
    OrbisRenderContext& context,
    RenderShaderStage stage) {
    switch (stage) {
    case RenderShaderStage::kVertex:
        orbis_render_context_clear_vertex_shader(context);
        break;
    case RenderShaderStage::kGeometry:
        orbis_render_context_clear_geometry_shader(context);
        break;
    case RenderShaderStage::kPixel:
        orbis_render_context_clear_pixel_shader(context);
        break;
    case RenderShaderStage::kCompute:
        orbis_render_context_clear_compute_shader(context);
        break;
    case RenderShaderStage::kHull:
    case RenderShaderStage::kDomain:
        break;
    }
}

}  // namespace rb4
