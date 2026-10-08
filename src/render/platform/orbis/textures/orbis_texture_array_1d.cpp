#include "render/platform/orbis/textures/orbis_texture_array_1d.h"

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_array_1d_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisTextureArray1DSize = 360;

}  // namespace

// Reconstructed from eboot.elf at 0x8D8A40.
OrbisTextureArray1D* orbis_create_texture_array_1d(
    const RenderTextureArray1DDescriptor& descriptor) {
    auto* storage = render_allocate(kOrbisTextureArray1DSize);
    auto* texture = reinterpret_cast<OrbisTextureArray1D*>(storage);
    orbis_texture_array_1d_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8E5870.
void orbis_texture_array_1d_construct(
    OrbisTextureArray1D& texture,
    const RenderTextureArray1DDescriptor& descriptor) {
    texture_array_1d_construct(texture, descriptor);
    orbis_texture_array_1d_clear_backend_state(texture);
}

}  // namespace rb4
