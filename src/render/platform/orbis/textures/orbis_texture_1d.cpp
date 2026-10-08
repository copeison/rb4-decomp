#include "render/platform/orbis/textures/orbis_texture_1d.h"

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_1d_adapters.h"

namespace rb4 {

namespace {

void bind_texture_stage(
    const OrbisTexture1D& texture,
    OrbisRenderContext& context,
    RenderShaderStage stage,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    orbis_bind_texture(
        context, stage, slot, texture.gpu_texture,
        static_cast<OrbisSamplerAddressMode>(texture.address_mode),
        texture.filter_mode, flags, border_color);
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D8980.
OrbisTexture1D* orbis_create_texture_1d(
    const RenderTexture1DDescriptor& descriptor) {
    auto* storage = render_allocate(sizeof(OrbisTexture1D));
    auto* texture = reinterpret_cast<OrbisTexture1D*>(storage);
    orbis_texture_1d_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8E4F60.
void orbis_texture_1d_construct(
    OrbisTexture1D& texture,
    const RenderTexture1DDescriptor& descriptor) {
    render_texture_1d_construct(texture, descriptor);
    orbis_texture_1d_install_vtable(texture);
    texture.gpu_texture = nullptr;
    texture.allocation = nullptr;
}

// Reconstructed from eboot.elf at 0x8E4F90.
void orbis_texture_1d_destruct(OrbisTexture1D& texture) {
    orbis_texture_1d_install_vtable(texture);
    orbis_defer_texture_allocation(texture.allocation);
    if (auto* descriptor = texture.gpu_texture) {
        render_release(descriptor);
    }
    texture.gpu_texture = nullptr;
    render_texture_1d_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8E4FF0.
void orbis_texture_1d_delete(OrbisTexture1D& texture) {
    orbis_texture_1d_destruct(texture);
    render_delete_texture_1d_storage(texture);
}

// Reconstructed from eboot.elf at 0x8E5050.
void orbis_texture_1d_initialize_backend(OrbisTexture1D& texture) {
    orbis_texture_1d_initialize_storage(texture);
}

// Reconstructed from eboot.elf at 0x8E52D0.
void orbis_texture_1d_bind_vertex(
    const OrbisTexture1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kVertex, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E52F0.
void orbis_texture_1d_bind_hull(
    const OrbisTexture1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kHull, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E5310.
void orbis_texture_1d_bind_domain(
    const OrbisTexture1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kDomain, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E5330.
void orbis_texture_1d_bind_geometry(
    const OrbisTexture1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kGeometry, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E5350.
void orbis_texture_1d_bind_pixel(
    const OrbisTexture1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kPixel, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E5370.
void orbis_texture_1d_bind_compute(
    const OrbisTexture1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kCompute, slot, flags,
        border_color);
}

}  // namespace rb4
