#include "render/platform/orbis/textures/orbis_texture_3d.h"

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_3d_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisTexture3DSize = 408;

void bind_texture_stage(
    const OrbisTexture3D& texture,
    OrbisRenderContext& context,
    RenderShaderStage stage,
    std::uint32_t slot,
    std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    orbis_bind_texture(
        context, stage, slot, orbis_texture_3d_gpu_texture(texture),
        orbis_texture_3d_address_mode(texture),
        orbis_texture_3d_filter_mode(texture), flags, border_color);
}

}  // namespace

// Reconstructed from eboot.elf at 0x8D89E0.
OrbisTexture3D* orbis_create_texture_3d(
    const RenderTexture3DDescriptor& descriptor) {
    auto* storage = render_allocate(kOrbisTexture3DSize);
    auto* texture = reinterpret_cast<OrbisTexture3D*>(storage);
    orbis_texture_3d_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8E53C0.
void orbis_texture_3d_construct(
    OrbisTexture3D& texture,
    const RenderTexture3DDescriptor& descriptor) {
    texture_3d_construct(texture, descriptor);
    orbis_texture_3d_clear_backend_state(texture);
}

// Reconstructed from eboot.elf at 0x8E53F0.
void orbis_texture_3d_destruct(OrbisTexture3D& texture) {
    orbis_defer_texture_allocation(orbis_texture_3d_allocation(texture));
    if (auto* descriptor = orbis_texture_3d_gpu_texture(texture)) {
        render_release(descriptor);
    }
    orbis_texture_3d_set_gpu_texture(texture, nullptr);
    texture_3d_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8E5450.
void orbis_texture_3d_delete(OrbisTexture3D& texture) {
    orbis_texture_3d_destruct(texture);
    render_delete_texture_3d(texture);
}

// Reconstructed from eboot.elf at 0x8E54B0.
void orbis_texture_3d_initialize_backend(
    OrbisTexture3D& texture,
    const OrbisTexture3D* storage_source) {
    orbis_texture_3d_initialize_storage(texture, storage_source);
}

// Reconstructed from eboot.elf at 0x8E5770.
void orbis_texture_3d_bind_vertex(
    const OrbisTexture3D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kVertex, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E5790.
void orbis_texture_3d_bind_hull(
    const OrbisTexture3D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kHull, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E57B0.
void orbis_texture_3d_bind_domain(
    const OrbisTexture3D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kDomain, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E57D0.
void orbis_texture_3d_bind_geometry(
    const OrbisTexture3D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kGeometry, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E57F0.
void orbis_texture_3d_bind_pixel(
    const OrbisTexture3D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kPixel, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E5810.
void orbis_texture_3d_bind_compute(
    const OrbisTexture3D& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kCompute, slot, flags,
        border_color);
}

}  // namespace rb4
