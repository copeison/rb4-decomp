#include "render/platform/orbis/textures/orbis_texture_cube.h"

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_cube_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisTextureCubeSize = 832;

}  // namespace

// Reconstructed from eboot.elf at 0x8D8A10.
OrbisTextureCube* orbis_create_texture_cube(
    const RenderTextureCubeDescriptor& descriptor) {
    auto* storage = render_allocate(kOrbisTextureCubeSize);
    auto* texture = reinterpret_cast<OrbisTextureCube*>(storage);
    orbis_texture_cube_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8E6BA0.
void orbis_texture_cube_construct(
    OrbisTextureCube& texture,
    const RenderTextureCubeDescriptor& descriptor) {
    texture_cube_construct(texture, descriptor);
    orbis_texture_cube_clear_backend_state(texture);
}

// Reconstructed from eboot.elf at 0x8E6BE0.
void orbis_texture_cube_destruct(OrbisTextureCube& texture) {
    if (auto* target =
            orbis_texture_cube_mutable_render_target(texture)) {
        orbis_defer_texture_allocation(
            orbis_render_target_metadata_allocation(*target));
        orbis_defer_texture_allocation(
            orbis_render_target_surface_allocation(*target));
        render_release(target);
        orbis_texture_cube_set_render_target(texture, nullptr);
    }

    orbis_defer_texture_allocation(
        orbis_texture_cube_primary_allocation(texture));
    orbis_defer_texture_allocation(
        orbis_texture_cube_secondary_allocation(texture));

    if (auto* descriptor = orbis_texture_cube_gpu_texture(texture)) {
        render_release(descriptor);
    }
    orbis_texture_cube_set_gpu_texture(texture, nullptr);

    if (auto* target =
            orbis_texture_cube_mutable_depth_target(texture)) {
        render_release(target);
    }
    orbis_texture_cube_set_depth_target(texture, nullptr);
    texture_cube_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8E6CC0.
void orbis_texture_cube_delete(OrbisTextureCube& texture) {
    orbis_texture_cube_destruct(texture);
    render_delete_texture_cube(texture);
}

// Reconstructed from eboot.elf at 0x8E6CE0.
void orbis_texture_cube_initialize_backend(OrbisTextureCube& texture) {
    if (orbis_texture_cube_is_depth(texture)) {
        orbis_texture_cube_initialize_depth_storage(texture);
    } else {
        orbis_texture_cube_initialize_color_storage(texture);
    }
}

// Reconstructed from eboot.elf at 0x8E7270.
const OrbisGpuRenderTarget* orbis_texture_cube_render_target(
    const OrbisTextureCube& texture) {
    return orbis_texture_cube_mutable_render_target(texture);
}

// Reconstructed from eboot.elf at 0x8E7280.
const OrbisGpuDepthRenderTarget* orbis_texture_cube_depth_target(
    const OrbisTextureCube& texture) {
    return orbis_texture_cube_mutable_depth_target(texture);
}

}  // namespace rb4
