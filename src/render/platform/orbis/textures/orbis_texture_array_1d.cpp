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

// Reconstructed from eboot.elf at 0x8E58A0.
void orbis_texture_array_1d_destruct(OrbisTextureArray1D& texture) {
    orbis_defer_texture_allocation(
        orbis_texture_array_1d_allocation(texture));
    if (auto* descriptor = orbis_texture_array_1d_gpu_texture(texture)) {
        render_release(descriptor);
    }
    orbis_texture_array_1d_set_gpu_texture(texture, nullptr);
    texture_array_1d_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8E5900.
void orbis_texture_array_1d_delete(OrbisTextureArray1D& texture) {
    orbis_texture_array_1d_destruct(texture);
    render_delete_texture_array_1d(texture);
}

// Reconstructed from eboot.elf at 0x8E5960.
void orbis_texture_array_1d_initialize_backend(
    OrbisTextureArray1D& texture) {
    orbis_texture_array_1d_initialize_storage(texture);
}

}  // namespace rb4
