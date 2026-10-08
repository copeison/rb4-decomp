#include "render/platform/orbis/textures/orbis_texture_cube.h"

#include <cstddef>

#include "core/memory/engine_memory.h"
#include "render/platform/orbis/textures/orbis_texture_cube_adapters.h"

namespace rb4 {

namespace {

void bind_texture_stage(
    const OrbisTextureCube& texture,
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

// Reconstructed from eboot.elf at 0x8D8A10.
OrbisTextureCube* orbis_create_texture_cube(
    const RenderTextureCubeDescriptor& descriptor) {
    auto* storage = render_allocate(sizeof(OrbisTextureCube));
    auto* texture = reinterpret_cast<OrbisTextureCube*>(storage);
    orbis_texture_cube_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8E6BA0.
void orbis_texture_cube_construct(
    OrbisTextureCube& texture,
    const RenderTextureCubeDescriptor& descriptor) {
    render_texture_cube_construct(texture, descriptor);
    orbis_texture_cube_install_vtable(texture);
    texture.gpu_texture = nullptr;
    texture.primary_allocation = nullptr;
    texture.secondary_allocation = nullptr;
    texture.render_target = nullptr;
    texture.depth_target = nullptr;
}

// Reconstructed from eboot.elf at 0x8E6BE0.
void orbis_texture_cube_destruct(OrbisTextureCube& texture) {
    orbis_texture_cube_install_vtable(texture);
    if (auto* target = texture.render_target) {
        orbis_defer_texture_allocation(
            orbis_render_target_metadata_allocation(*target));
        orbis_defer_texture_allocation(
            orbis_render_target_surface_allocation(*target));
        render_release(target);
        texture.render_target = nullptr;
    }

    orbis_defer_texture_allocation(texture.primary_allocation);
    orbis_defer_texture_allocation(texture.secondary_allocation);

    if (auto* descriptor = texture.gpu_texture) {
        render_release(descriptor);
    }
    texture.gpu_texture = nullptr;

    if (auto* target = texture.depth_target) {
        render_release(target);
    }
    texture.depth_target = nullptr;
    render_texture_cube_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8E6CC0.
void orbis_texture_cube_delete(OrbisTextureCube& texture) {
    orbis_texture_cube_destruct(texture);
    render_release(&texture);
}

// Reconstructed from eboot.elf at 0x8E6CE0.
void orbis_texture_cube_initialize_backend(OrbisTextureCube& texture) {
    if (texture.usage_type == RenderTextureUsage::kDepth) {
        orbis_texture_cube_initialize_depth_storage(texture);
    } else {
        orbis_texture_cube_initialize_color_storage(texture);
    }
}

// Reconstructed from eboot.elf at 0x8E7270.
const OrbisGpuRenderTarget* orbis_texture_cube_render_target(
    const OrbisTextureCube& texture) {
    return texture.render_target;
}

// Reconstructed from eboot.elf at 0x8E7280.
const OrbisGpuDepthRenderTarget* orbis_texture_cube_depth_target(
    const OrbisTextureCube& texture) {
    return texture.depth_target;
}

// Reconstructed from eboot.elf at 0x8E71A0.
void orbis_texture_cube_bind_vertex(
    const OrbisTextureCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kVertex, slot, flags,
        border_color);
}

#define RB4_DEFINE_CUBE_BINDING(method, stage_value)                   \
    void method(                                                       \
        const OrbisTextureCube& texture, OrbisRenderContext& context,   \
        std::uint32_t slot, std::uint32_t flags,                        \
        const OrbisSamplerBorderColor& border_color) {                  \
        bind_texture_stage(                                             \
            texture, context, stage_value, slot, flags, border_color);  \
    }

// Reconstructed from eboot.elf at 0x8E71C0.
RB4_DEFINE_CUBE_BINDING(orbis_texture_cube_bind_hull, RenderShaderStage::kHull)
// Reconstructed from eboot.elf at 0x8E71E0.
RB4_DEFINE_CUBE_BINDING(
    orbis_texture_cube_bind_domain, RenderShaderStage::kDomain)
// Reconstructed from eboot.elf at 0x8E7200.
RB4_DEFINE_CUBE_BINDING(
    orbis_texture_cube_bind_geometry, RenderShaderStage::kGeometry)
// Reconstructed from eboot.elf at 0x8E7220.
RB4_DEFINE_CUBE_BINDING(
    orbis_texture_cube_bind_pixel, RenderShaderStage::kPixel)
// Reconstructed from eboot.elf at 0x8E7240.
RB4_DEFINE_CUBE_BINDING(
    orbis_texture_cube_bind_compute, RenderShaderStage::kCompute)

#undef RB4_DEFINE_CUBE_BINDING

}  // namespace rb4
