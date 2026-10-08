#include "render/platform/orbis/textures/orbis_texture_array_cube.h"

#include <cstddef>

#include "core/memory/engine_memory.h"
#include "render/platform/orbis/textures/orbis_texture_array_cube_adapters.h"

namespace rb4 {

namespace {

void bind_texture_stage(
    const OrbisTextureArrayCube& texture,
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

// Reconstructed from eboot.elf at 0x8D8AA0.
OrbisTextureArrayCube* orbis_create_texture_array_cube(
    const RenderTextureArrayCubeDescriptor& descriptor) {
    auto* storage = render_allocate(sizeof(OrbisTextureArrayCube));
    auto* texture = reinterpret_cast<OrbisTextureArrayCube*>(storage);
    orbis_texture_array_cube_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8E6640.
void orbis_texture_array_cube_construct(
    OrbisTextureArrayCube& texture,
    const RenderTextureArrayCubeDescriptor& descriptor) {
    render_texture_array_cube_construct(texture, descriptor);
    orbis_texture_array_cube_install_vtable(texture);
    texture.gpu_texture = nullptr;
    texture.allocation = nullptr;
}

// Reconstructed from eboot.elf at 0x8E6670.
void orbis_texture_array_cube_destruct(OrbisTextureArrayCube& texture) {
    orbis_texture_array_cube_install_vtable(texture);
    orbis_defer_texture_allocation(texture.allocation);
    if (auto* descriptor = texture.gpu_texture) {
        render_release(descriptor);
    }
    texture.gpu_texture = nullptr;
    render_texture_array_cube_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8E66D0.
void orbis_texture_array_cube_delete(OrbisTextureArrayCube& texture) {
    orbis_texture_array_cube_destruct(texture);
    render_release(&texture);
}

// Reconstructed from eboot.elf at 0x8E6730.
void orbis_texture_array_cube_initialize_backend(
    OrbisTextureArrayCube& texture) {
    orbis_texture_array_cube_initialize_storage(texture);
}

// Reconstructed from eboot.elf at 0x8E6AB0.
void orbis_texture_array_cube_bind_vertex(
    const OrbisTextureArrayCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kVertex, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E6AD0.
void orbis_texture_array_cube_bind_hull(
    const OrbisTextureArrayCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kHull, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E6AF0.
void orbis_texture_array_cube_bind_domain(
    const OrbisTextureArrayCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kDomain, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E6B10.
void orbis_texture_array_cube_bind_geometry(
    const OrbisTextureArrayCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kGeometry, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E6B30.
void orbis_texture_array_cube_bind_pixel(
    const OrbisTextureArrayCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kPixel, slot, flags,
        border_color);
}

// Reconstructed from eboot.elf at 0x8E6B50.
void orbis_texture_array_cube_bind_compute(
    const OrbisTextureArrayCube& texture, OrbisRenderContext& context,
    std::uint32_t slot, std::uint32_t flags,
    const OrbisSamplerBorderColor& border_color) {
    bind_texture_stage(
        texture, context, RenderShaderStage::kCompute, slot, flags,
        border_color);
}

}  // namespace rb4
