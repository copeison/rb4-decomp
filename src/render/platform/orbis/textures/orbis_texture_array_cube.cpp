#include "render/platform/orbis/textures/orbis_texture_array_cube.h"

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_array_cube_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisTextureArrayCubeSize = 360;

}  // namespace

// Reconstructed from eboot.elf at 0x8D8AA0.
OrbisTextureArrayCube* orbis_create_texture_array_cube(
    const RenderTextureArrayCubeDescriptor& descriptor) {
    auto* storage = render_allocate(kOrbisTextureArrayCubeSize);
    auto* texture = reinterpret_cast<OrbisTextureArrayCube*>(storage);
    orbis_texture_array_cube_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8E6640.
void orbis_texture_array_cube_construct(
    OrbisTextureArrayCube& texture,
    const RenderTextureArrayCubeDescriptor& descriptor) {
    texture_array_cube_construct(texture, descriptor);
    orbis_texture_array_cube_clear_backend_state(texture);
}

// Reconstructed from eboot.elf at 0x8E6670.
void orbis_texture_array_cube_destruct(OrbisTextureArrayCube& texture) {
    orbis_defer_texture_allocation(
        orbis_texture_array_cube_allocation(texture));
    if (auto* descriptor = orbis_texture_array_cube_gpu_texture(texture)) {
        render_release(descriptor);
    }
    orbis_texture_array_cube_set_gpu_texture(texture, nullptr);
    texture_array_cube_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8E66D0.
void orbis_texture_array_cube_delete(OrbisTextureArrayCube& texture) {
    orbis_texture_array_cube_destruct(texture);
    render_delete_texture_array_cube(texture);
}

// Reconstructed from eboot.elf at 0x8E6730.
void orbis_texture_array_cube_initialize_backend(
    OrbisTextureArrayCube& texture) {
    orbis_texture_array_cube_initialize_storage(texture);
}

}  // namespace rb4
