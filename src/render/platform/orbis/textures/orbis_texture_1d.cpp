#include "render/platform/orbis/textures/orbis_texture_1d.h"

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_1d_adapters.h"

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

// Reconstructed from eboot.elf at 0x8E4F90.
void orbis_texture_1d_destruct(OrbisTexture1D& texture) {
    orbis_defer_texture_allocation(orbis_texture_1d_allocation(texture));
    if (auto* descriptor = orbis_texture_1d_gpu_texture(texture)) {
        render_release(descriptor);
    }
    orbis_texture_1d_set_gpu_texture(texture, nullptr);
    texture_1d_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8E4FF0.
void orbis_texture_1d_delete(OrbisTexture1D& texture) {
    orbis_texture_1d_destruct(texture);
    render_delete_texture_1d(texture);
}

// Reconstructed from eboot.elf at 0x8E5050.
void orbis_texture_1d_initialize_backend(OrbisTexture1D& texture) {
    orbis_texture_1d_initialize_storage(texture);
}

}  // namespace rb4
