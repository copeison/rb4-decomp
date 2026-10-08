#include "orbis_texture_3d.h"

#include <cstddef>

#include "orbis_texture_3d_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisTexture3DSize = 360;

}  // namespace

// Reconstructed from eboot.elf at 0x8D8A40.
OrbisTexture3D* orbis_create_texture_3d(
    const RenderTexture3DDescriptor& descriptor) {
    auto* storage = render_allocate(kOrbisTexture3DSize);
    auto* texture = reinterpret_cast<OrbisTexture3D*>(storage);
    orbis_texture_3d_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8E5870.
void orbis_texture_3d_construct(
    OrbisTexture3D& texture,
    const RenderTexture3DDescriptor& descriptor) {
    texture_3d_construct(texture, descriptor);
    orbis_texture_3d_clear_backend_state(texture);
}

}  // namespace rb4
