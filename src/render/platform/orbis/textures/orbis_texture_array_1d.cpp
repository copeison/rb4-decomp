#include "render/platform/orbis/textures/orbis_texture_array_1d.h"

#include <cstddef>

#include "core/memory/engine_memory.h"
#include "render/platform/orbis/textures/orbis_texture_array_1d_adapters.h"

namespace rb4 {

namespace {

void bind_texture_stage(
    const OrbisTextureArray1D& texture,
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

// Reconstructed from eboot.elf at 0x8D8A40.
OrbisTextureArray1D* orbis_create_texture_array_1d(
    const RenderTextureArray1DDescriptor& descriptor) {
    auto* storage = render_allocate(sizeof(OrbisTextureArray1D));
    auto* texture = reinterpret_cast<OrbisTextureArray1D*>(storage);
    orbis_texture_array_1d_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8E5870.
void orbis_texture_array_1d_construct(
    OrbisTextureArray1D& texture,
    const RenderTextureArray1DDescriptor& descriptor) {
    render_texture_array_1d_construct(texture, descriptor);
    orbis_texture_array_1d_install_vtable(texture);
    texture.gpu_texture = nullptr;
    texture.allocation = nullptr;
}

// Reconstructed from eboot.elf at 0x8E58A0.
void orbis_texture_array_1d_destruct(OrbisTextureArray1D& texture) {
    orbis_texture_array_1d_install_vtable(texture);
    orbis_defer_texture_allocation(texture.allocation);
    if (auto* descriptor = texture.gpu_texture) {
        render_release(descriptor);
    }
    texture.gpu_texture = nullptr;
    render_texture_array_1d_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8E5900.
void orbis_texture_array_1d_delete(OrbisTextureArray1D& texture) {
    orbis_texture_array_1d_destruct(texture);
    render_delete_texture_array_1d_storage(texture);
}

// Reconstructed from eboot.elf at 0x8E5960.
void orbis_texture_array_1d_initialize_backend(
    OrbisTextureArray1D& texture) {
    orbis_texture_array_1d_initialize_storage(texture);
}

// Reconstructed from eboot.elf at 0x8E5C50.
void orbis_texture_array_1d_bind_vertex(
    const OrbisTextureArray1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kVertex, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E5C70.
void orbis_texture_array_1d_bind_hull(
    const OrbisTextureArray1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kHull, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E5C90.
void orbis_texture_array_1d_bind_domain(
    const OrbisTextureArray1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kDomain, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E5CB0.
void orbis_texture_array_1d_bind_geometry(
    const OrbisTextureArray1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kGeometry, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E5CD0.
void orbis_texture_array_1d_bind_pixel(
    const OrbisTextureArray1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kPixel, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E5CF0.
void orbis_texture_array_1d_bind_compute(
    const OrbisTextureArray1D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kCompute, slot, flags,
        border_color);
}

}  // namespace rb4
