#include "orbis_texture_1d.h"

#include <cstddef>

#include "orbis_texture_1d_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisTexture1DSize = 408;

}  // namespace

// Reconstructed from eboot.elf at 0x8D8980.
OrbisTexture1D* orbis_create_texture_1d(
    const RenderTexture1DDescriptor& descriptor) {
    auto* storage = render_allocate(kOrbisTexture1DSize);
    auto* texture = reinterpret_cast<OrbisTexture1D*>(storage);
    orbis_texture_1d_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8E4F60.
void orbis_texture_1d_construct(
    OrbisTexture1D& texture,
    const RenderTexture1DDescriptor& descriptor) {
    texture_1d_construct(texture, descriptor);
    orbis_texture_1d_clear_backend_state(texture);
}

}  // namespace rb4
