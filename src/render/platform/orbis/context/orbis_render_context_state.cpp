#include "render/platform/orbis/context/orbis_render_context.h"
#include "renderps4/context/PS4Context.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/context/orbis_render_context_state_adapters.h"
#include "render/core/system/render_runtime_adapters.h"

using namespace rb4;

namespace rb4 {

namespace {

constexpr std::size_t kColorRenderTargetCount = 8;
constexpr std::int32_t kUnboundTargetKind = -1;
constexpr std::int32_t kPerTargetBlendMode = 11;
constexpr std::array<std::uint8_t, 10> kStencilMasks = {
    0xFF, 0x07, 0x08, 0x10, 0x0F,
    0x1F, 0x20, 0x28, 0x30, 0xC0,
};

std::uint8_t stencil_mask(std::uint32_t index) {
    return index < kStencilMasks.size() ? kStencilMasks[index] : 0;
}

}  // namespace

}  // namespace rb4

// Reconstructed from eboot.elf at 0x8E8850.
void PS4Context::_BeginFrameImpl() {
    auto& context = *this;
    orbis_render_context_reset_cached_pipeline_state(context);
    context.PS4Context::_SetRenderTargetsImpl(
        kUnboundTargetKind, orbis_default_render_target_binding());
    context.PS4Context::_SetBlendModeImpl(
        RndMaterialBlendMode::kSource, orbis_default_blend_configuration());
    orbis_render_context_set_default_raster_state(context);
    orbis_render_context_set_default_depth_stencil_state(context);
    orbis_render_context_disable_stream_output(context);
    orbis_render_context_clear_shader_resources(context);
}

// Reconstructed from eboot.elf at 0x8E8D20.
void PS4Context::_SetRenderTargetsImpl(int target_kind, const RenderTargetParams& binding) {
    auto& context = *this;
    std::array<const OrbisGpuRenderTarget*, kColorRenderTargetCount>
        color_targets{};
    const auto color_count = std::min(
        orbis_render_target_color_count(binding), color_targets.size());
    for (std::size_t slot = 0; slot < color_count; ++slot) {
        color_targets[slot] = orbis_resolve_color_render_target(
            binding, target_kind, slot);
    }
    const auto* depth_target =
        orbis_resolve_depth_render_target(binding, target_kind);

    for (std::size_t slot = 0; slot < color_targets.size(); ++slot) {
        orbis_render_context_bind_color_target(
            context, slot, color_targets[slot]);
    }
    orbis_render_context_bind_depth_target(context, depth_target);
    orbis_render_context_set_viewport_and_scissor(
        context, orbis_render_target_viewport(binding));

    bool synchronized = false;
    for (std::size_t slot = 0; slot < color_count; ++slot) {
        if (!orbis_color_render_target_requires_sync(binding, slot)) {
            continue;
        }
        if (!synchronized) {
            orbis_render_context_begin_render_target_sync(context);
        }
        orbis_render_context_prepare_color_target(context, binding, slot);
        synchronized = true;
    }

    if (depth_target != nullptr &&
        orbis_depth_render_target_requires_prepare(binding)) {
        synchronized = orbis_render_context_prepare_depth_target(
                           context, *depth_target, binding) ||
                       synchronized;
    }
    if (synchronized) {
        orbis_render_context_finish_render_target_sync(context);
    }
}

// Reconstructed from eboot.elf at 0x8E92D0.
void PS4Context::_SetBlendModeImpl(rb4::RndMaterialBlendMode mode, const BlendParams& configuration) {
    auto& context = *this;
    if (static_cast<std::int32_t>(mode) == kPerTargetBlendMode) {
        for (std::size_t slot = 0; slot < kColorRenderTargetCount; ++slot) {
            const auto control =
                orbis_build_blend_control(mode, configuration, slot);
            orbis_render_context_set_gnm_blend_control(
                context, slot, control);
        }
        return;
    }

    const auto control = orbis_build_blend_control(mode, configuration, 0);
    for (std::size_t slot = 0; slot < kColorRenderTargetCount; ++slot) {
        orbis_render_context_set_gnm_blend_control(context, slot, control);
    }
}

// Reconstructed from eboot.elf at 0x8E9F60.
void PS4Context::_SetDepthModeImpl(unsigned int depth_mode) {
    auto& context = *this;
    orbis_render_context_cache_depth_mode(context, depth_mode);
    orbis_render_context_apply_depth_stencil_state(context);
}

// Reconstructed from eboot.elf at 0x8EA030.
void PS4Context::_SetStencilModeImpl(unsigned int stencil_mode, unsigned char reference, unsigned int read_mask, unsigned int write_mask) {
    auto& context = *this;
    orbis_render_context_cache_stencil_state(
        context,
        stencil_mode,
        reference,
        stencil_mask(read_mask),
        stencil_mask(write_mask));
    orbis_render_context_apply_depth_stencil_state(context);
}

// Reconstructed from eboot.elf at 0x8EA150.
void PS4Context::_SetFrontFaceImpl(bool counter_clockwise) {
    auto& context = *this;
    orbis_render_context_cache_front_face(context, counter_clockwise);
    orbis_render_context_apply_primitive_setup(context);
}

// Reconstructed from eboot.elf at 0x8EA1C0.
void PS4Context::_SetCullModeImpl(RndCullMode cull_mode) {
    auto& context = *this;
    orbis_render_context_cache_cull_mode(context, cull_mode);
    orbis_render_context_apply_primitive_setup(context);
}

// Reconstructed from eboot.elf at 0x8EA230.
void PS4Context::_SetFillModeImpl(bool enabled) {
    auto& context = *this;
    orbis_render_context_cache_polygon_fill(context, enabled);
    orbis_render_context_apply_primitive_setup(context);
}

// Reconstructed from eboot.elf at 0x8E96F0.
void PS4Context::_SetColorWriteMaskImpl(unsigned char target_mask, RndWriteMaskChannelSet write_mode) {
    auto& context = *this;
    std::uint32_t channel_mask = 0;
    if (write_mode == kWriteRGBA) {
        channel_mask = 0xF;
    } else if (write_mode == kWriteRGB) {
        channel_mask = 0x7;
    }

    std::uint32_t gnm_mask = 0;
    for (std::size_t slot = 0; slot < kColorRenderTargetCount; ++slot) {
        if ((target_mask & (1U << slot)) != 0) {
            gnm_mask |= channel_mask << (slot * 4);
        }
    }
    orbis_render_context_set_gnm_render_target_mask(context, gnm_mask);
    orbis_render_context_cache_color_write_mask(
        context, target_mask, write_mode);
}
