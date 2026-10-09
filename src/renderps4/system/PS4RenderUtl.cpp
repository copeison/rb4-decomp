#include "renderps4/system/PS4RenderUtl.h"

#include "render/platform/orbis/shaders/orbis_texture_binding_adapters.h"
#include "renderps4/context/PS4Context.h"

namespace {

using rb4::OrbisGnmShaderStage;
using rb4::OrbisSamplerAddressMode;

constexpr unsigned long kSamplerSlotCount = 16;

bool ShouldSelectSampler(unsigned long slot, unsigned int flags) {
    return slot < kSamplerSlotCount &&
        (flags & PS4RenderUtl::kSelectNoSampler) == 0;
}

void SelectSampledTexture(
    RndContext& context,
    OrbisGnmShaderStage stage,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    auto& ps4 = static_cast<PS4Context&>(context);
    const auto index = static_cast<unsigned int>(slot);
    if (ShouldSelectSampler(slot, flags)) {
        rb4::orbis_bind_graphics_texture_sampler(
            ps4, stage, index, static_cast<OrbisSamplerAddressMode>(wrap), filter);
    }
    rb4::orbis_bind_graphics_texture(ps4, stage, index, texture);
}

}  // namespace

// Reconstructed from eboot.elf at 0x8E1EE0.
void PS4RenderUtl::SelectTextureForVS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter) {
    SelectSampledTexture(
        context, OrbisGnmShaderStage::kVertex, slot, texture, wrap, filter, 0);
}

// Reconstructed from eboot.elf at 0x8E1F90.
void PS4RenderUtl::SelectTextureForHS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    SelectSampledTexture(
        context, OrbisGnmShaderStage::kHull, slot, texture, wrap, filter, flags);
}

// Reconstructed from eboot.elf at 0x8E2040. Domain textures bind to the
// Gnm local stage.
void PS4RenderUtl::SelectTextureForDS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    SelectSampledTexture(
        context, OrbisGnmShaderStage::kLocal, slot, texture, wrap, filter, flags);
}

// Reconstructed from eboot.elf at 0x8E20F0.
void PS4RenderUtl::SelectTextureForGS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    SelectSampledTexture(
        context, OrbisGnmShaderStage::kGeometry, slot, texture, wrap, filter, flags);
}

// Reconstructed from eboot.elf at 0x8E21A0.
void PS4RenderUtl::SelectTextureForPS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    if ((flags & kSelectWritable) != 0) {
        rb4::orbis_bind_graphics_rw_texture(
            static_cast<PS4Context&>(context),
            OrbisGnmShaderStage::kPixel,
            static_cast<unsigned int>(slot),
            texture);
        return;
    }
    SelectSampledTexture(
        context, OrbisGnmShaderStage::kPixel, slot, texture, wrap, filter, flags);
}

// Reconstructed from eboot.elf at 0x8E2290.
void PS4RenderUtl::SelectTextureForCS(
    RndContext& context,
    unsigned long slot,
    const void* texture,
    unsigned int wrap,
    unsigned int filter,
    unsigned int flags) {
    auto& ps4 = static_cast<PS4Context&>(context);
    const auto index = static_cast<unsigned int>(slot);
    const bool computeQueue = rb4::orbis_render_context_uses_compute_queue(ps4);
    if ((flags & kSelectWritable) != 0) {
        if (computeQueue) {
            rb4::orbis_bind_compute_rw_texture(ps4, index, texture);
        } else {
            rb4::orbis_bind_graphics_rw_texture(
                ps4, OrbisGnmShaderStage::kCompute, index, texture);
        }
        return;
    }

    if (computeQueue) {
        rb4::orbis_bind_compute_texture_descriptor(ps4, index, texture);
        if (ShouldSelectSampler(slot, flags)) {
            rb4::orbis_bind_compute_texture_sampler(
                ps4, index, static_cast<OrbisSamplerAddressMode>(wrap), filter);
        }
        return;
    }

    SelectSampledTexture(
        context, OrbisGnmShaderStage::kCompute, slot, texture, wrap, filter, flags);
}
