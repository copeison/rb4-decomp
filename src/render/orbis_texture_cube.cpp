#include "orbis_texture_cube.h"

#include <cstddef>

#include "orbis_texture_cube_adapters.h"

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

}  // namespace rb4
