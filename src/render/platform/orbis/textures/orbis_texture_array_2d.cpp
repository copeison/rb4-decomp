#include "render/platform/orbis/textures/orbis_texture_array_2d.h"

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_array_2d_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisTextureArray2DSize = 392;

void bind_texture_stage(
    const OrbisTextureArray2D& texture,
    OrbisRenderContext& context,
    RenderShaderStage stage,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    orbis_bind_texture(
        context, stage, slot, orbis_texture_array_2d_gpu_texture(texture),
        orbis_texture_array_2d_address_mode(texture),
        orbis_texture_array_2d_filter_mode(texture), flags, border_color);
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D8A70.
OrbisTextureArray2D* orbis_create_texture_array_2d(
    const RenderTextureArray2DDescriptor& descriptor) {
    auto* storage = render_allocate(kOrbisTextureArray2DSize);
    auto* texture = reinterpret_cast<OrbisTextureArray2D*>(storage);
    orbis_texture_array_2d_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8E5D40.
void orbis_texture_array_2d_construct(
    OrbisTextureArray2D& texture,
    const RenderTextureArray2DDescriptor& descriptor) {
    texture_array_2d_construct(texture, descriptor);
    orbis_texture_array_2d_clear_backend_state(texture);
}

// Reconstructed from eboot.elf at 0x8E5D80.
void orbis_texture_array_2d_destruct(OrbisTextureArray2D& texture) {
    orbis_texture_array_2d_release_backend_state(texture);
    texture_array_2d_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8E5E50.
void orbis_texture_array_2d_delete(OrbisTextureArray2D& texture) {
    orbis_texture_array_2d_destruct(texture);
    render_delete_texture_array_2d(texture);
}

// Reconstructed from eboot.elf at 0x8E5E70.
void orbis_texture_array_2d_initialize_backend(
    OrbisTextureArray2D& texture) {
    if (orbis_texture_array_2d_is_depth(texture)) {
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
