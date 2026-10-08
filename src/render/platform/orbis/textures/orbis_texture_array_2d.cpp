#include "render/platform/orbis/textures/orbis_texture_array_2d.h"

#include <cstddef>

#include "core/memory/engine_memory.h"
#include "render/platform/orbis/textures/orbis_texture_array_2d_adapters.h"

namespace rb4 {

namespace {

void bind_texture_stage(
    const OrbisTextureArray2D& texture,
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

// Reconstructed from eboot.elf at 0x8D8A70.
OrbisTextureArray2D* orbis_create_texture_array_2d(
    const RenderTextureArray2DDescriptor& descriptor) {
    auto* storage = render_allocate(sizeof(OrbisTextureArray2D));
    auto* texture = reinterpret_cast<OrbisTextureArray2D*>(storage);
    orbis_texture_array_2d_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8E5D40.
void orbis_texture_array_2d_construct(
    OrbisTextureArray2D& texture,
    const RenderTextureArray2DDescriptor& descriptor) {
    render_texture_array_2d_construct(texture, descriptor);
    orbis_texture_array_2d_install_vtable(texture);
    texture.gpu_texture = nullptr;
    texture.primary_allocation = nullptr;
    texture.stencil_allocation = nullptr;
    texture.metadata_allocation = nullptr;
    texture.color_target = nullptr;
    texture.depth_target = nullptr;
}

// Reconstructed from eboot.elf at 0x8E5D80.
void orbis_texture_array_2d_destruct(OrbisTextureArray2D& texture) {
    orbis_texture_array_2d_install_vtable(texture);
    orbis_defer_texture_allocation(texture.primary_allocation);
    orbis_defer_texture_allocation(texture.stencil_allocation);
    orbis_defer_texture_allocation(texture.metadata_allocation);
    if (texture.color_target != nullptr) {
        orbis_defer_texture_allocation(
            orbis_color_target_metadata_allocation(texture.color_target));
        render_release(texture.color_target);
        texture.color_target = nullptr;
    }
    if (texture.gpu_texture != nullptr) {
        render_release(texture.gpu_texture);
        texture.gpu_texture = nullptr;
    }
    if (texture.depth_target != nullptr) {
        render_release(texture.depth_target);
        texture.depth_target = nullptr;
    }
    render_texture_array_2d_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8E5E50.
void orbis_texture_array_2d_delete(OrbisTextureArray2D& texture) {
    orbis_texture_array_2d_destruct(texture);
    render_delete_texture_array_2d_storage(texture);
}

// Reconstructed from eboot.elf at 0x8E5E70.
void orbis_texture_array_2d_initialize_backend(
    OrbisTextureArray2D& texture) {
    if (texture.usage_type == RenderTextureUsage::kDepth) {
        orbis_texture_array_2d_initialize_depth_storage(texture);
    } else {
        orbis_texture_array_2d_initialize_color_storage(texture);
    }
}

// Reconstructed from eboot.elf at 0x8E64C0.
void orbis_texture_array_2d_bind_vertex(
    const OrbisTextureArray2D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kVertex, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E64E0.
void orbis_texture_array_2d_bind_hull(
    const OrbisTextureArray2D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kHull, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E6500.
void orbis_texture_array_2d_bind_domain(
    const OrbisTextureArray2D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kDomain, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E6520.
void orbis_texture_array_2d_bind_geometry(
    const OrbisTextureArray2D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kGeometry, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E6540.
void orbis_texture_array_2d_bind_pixel(
    const OrbisTextureArray2D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kPixel, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E6560.
void orbis_texture_array_2d_bind_compute(
    const OrbisTextureArray2D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kCompute, slot, flags,
        border_color);
}

}  // namespace rb4
