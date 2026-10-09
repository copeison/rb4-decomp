#include "render/platform/orbis/shaders/orbis_texture_binding.h"
#include "render/shaders/RndShaderEnums.h"

#include "render/platform/orbis/shaders/orbis_texture_binding_adapters.h"

namespace rb4 {

namespace {

constexpr std::uint32_t kSamplerSlotCount = 16;

bool should_bind_sampler(std::uint32_t slot, std::uint32_t flags) {
    return slot < kSamplerSlotCount &&
        (flags & kOrbisTextureBindingSuppressSampler) == 0;
}

void bind_graphics_sampled_texture(
    OrbisRenderContext& context,
    OrbisGnmShaderStage stage,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    if (should_bind_sampler(slot, flags)) {
        orbis_bind_graphics_texture_sampler(
            context, stage, slot, address_mode, filter_mode, border_color);
    }
    orbis_bind_graphics_texture(context, stage, slot, texture);
}

}  // namespace

void orbis_bind_texture(
    OrbisRenderContext& context,
    RndShaderProgramType stage,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    switch (stage) {
    case kShaderProgramVertex:
        orbis_bind_vertex_texture(
            context, slot, texture, address_mode, filter_mode, border_color);
        break;
    case kShaderProgramHull:
        orbis_bind_hull_texture(
            context, slot, texture, address_mode, filter_mode, flags,
            border_color);
        break;
    case kShaderProgramDomain:
        orbis_bind_domain_texture(
            context, slot, texture, address_mode, filter_mode, flags,
            border_color);
        break;
    case kShaderProgramGeometry:
        orbis_bind_geometry_texture(
            context, slot, texture, address_mode, filter_mode, flags,
            border_color);
        break;
    case kShaderProgramPixel:
        orbis_bind_pixel_texture(
            context, slot, texture, address_mode, filter_mode, flags,
            border_color);
        break;
    case kShaderProgramCompute:
        orbis_bind_compute_texture(
            context, slot, texture, address_mode, filter_mode, flags,
            border_color);
        break;
    }
}

// Reconstructed from eboot.elf at 0x8E1EE0.
void orbis_bind_vertex_texture(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    const OrbisSamplerBorderColor& border_color) {
    bind_graphics_sampled_texture(
        context, OrbisGnmShaderStage::kVertex, slot, texture,
        address_mode, filter_mode, 0, border_color);
}

// Reconstructed from eboot.elf at 0x8E1F90.
void orbis_bind_hull_texture(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_graphics_sampled_texture(
        context, OrbisGnmShaderStage::kHull, slot, texture,
        address_mode, filter_mode, flags, border_color);
}

// Reconstructed from eboot.elf at 0x8E2040.
void orbis_bind_domain_texture(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_graphics_sampled_texture(
        context, OrbisGnmShaderStage::kLocal, slot, texture,
        address_mode, filter_mode, flags, border_color);
}

// Reconstructed from eboot.elf at 0x8E20F0.
void orbis_bind_geometry_texture(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_graphics_sampled_texture(
        context, OrbisGnmShaderStage::kGeometry, slot, texture,
        address_mode, filter_mode, flags, border_color);
}

// Reconstructed from eboot.elf at 0x8E21A0.
void orbis_bind_pixel_texture(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    if ((flags & kOrbisTextureBindingWritable) != 0) {
        orbis_bind_graphics_rw_texture(
            context, OrbisGnmShaderStage::kPixel, slot, texture);
        return;
    }
    bind_graphics_sampled_texture(
        context, OrbisGnmShaderStage::kPixel, slot, texture,
        address_mode, filter_mode, flags, border_color);
}

// Reconstructed from eboot.elf at 0x8E2290.
void orbis_bind_compute_texture(
    OrbisRenderContext& context,
    std::uint32_t slot,
    const void* texture,
    OrbisSamplerAddressMode address_mode,
    std::uint32_t filter_mode,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    const bool compute_queue =
        orbis_render_context_uses_compute_queue(context);
    if ((flags & kOrbisTextureBindingWritable) != 0) {
        if (compute_queue) {
            orbis_bind_compute_rw_texture(context, slot, texture);
        } else {
            orbis_bind_graphics_rw_texture(
                context, OrbisGnmShaderStage::kCompute, slot, texture);
        }
        return;
    }

    if (compute_queue) {
        orbis_bind_compute_texture_descriptor(context, slot, texture);
        if (should_bind_sampler(slot, flags)) {
            orbis_bind_compute_texture_sampler(
                context, slot, address_mode, filter_mode, border_color);
        }
        return;
    }

    bind_graphics_sampled_texture(
        context, OrbisGnmShaderStage::kCompute, slot, texture,
        address_mode, filter_mode, flags, border_color);
}

}  // namespace rb4
