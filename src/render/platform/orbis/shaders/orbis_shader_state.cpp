#include "render/platform/orbis/shaders/orbis_shader_state.h"
#include "renderps4/context/PS4Context.h"
#include "render/shaders/RndShaderEnums.h"

#include <cstdint>

#include "render/platform/orbis/shaders/orbis_shader_state_adapters.h"
#include "renderps4/context/PS4RenderStateUtl.h"

using namespace rb4;

namespace rb4 {

namespace {

constexpr std::uint32_t kShaderStageCount = 6;

}  // namespace

}  // namespace rb4

// Reconstructed from eboot.elf at 0x8EA830.
void PS4Context::_SetSamplerImpl(RndShaderProgramType stage, unsigned int slot, unsigned int wrap, unsigned int filter_mode) {
    auto& context = *this;
    const auto address_mode = static_cast<rb4::OrbisSamplerAddressMode>(wrap);
    OrbisSamplerDescriptor sampler;
    PS4RenderStateUtl::InitSampler(sampler, address_mode, filter_mode);
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
void PS4Context::_DeactivateShaderProgramTypeImpl(RndShaderProgramType stage) {
    auto& context = *this;
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
void PS4Context::_DeselectAllReadWriteTexturesImpl(unsigned int stage_mask) {
    auto& context = *this;
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
void PS4Context::_DeselectAllSourceTexturesImpl(unsigned int stage_mask) {
    auto& context = *this;
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
