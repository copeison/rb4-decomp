#include "orbis_texture_2d.h"

#include <cstddef>

#include "orbis_texture_2d_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisTexture2DSize = 520;

}  // namespace

// Reconstructed from eboot.elf at 0x8D89B0.
OrbisTexture2D* orbis_create_texture_2d(
    const RenderTexture2DDescriptor& descriptor) {
    auto* storage = render_allocate(kOrbisTexture2DSize);
    auto* texture = reinterpret_cast<OrbisTexture2D*>(storage);
    orbis_texture_2d_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8D62C0.
void orbis_texture_2d_construct(
    OrbisTexture2D& texture,
    const RenderTexture2DDescriptor& descriptor) {
    texture_2d_construct(texture, descriptor);
    orbis_texture_2d_clear_backend_state(texture);
}

}  // namespace rb4
