#include "render/platform/orbis/textures/orbis_texture_3d.h"

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_3d_adapters.h"

namespace rb4 {

namespace {

constexpr std::size_t kOrbisTexture3DSize = 408;

}  // namespace

// Reconstructed from eboot.elf at 0x8D89E0.
OrbisTexture3D* orbis_create_texture_3d(
    const RenderTexture3DDescriptor& descriptor) {
    auto* storage = render_allocate(kOrbisTexture3DSize);
    auto* texture = reinterpret_cast<OrbisTexture3D*>(storage);
    orbis_texture_3d_construct(*texture, descriptor);
    return texture;
}

// Reconstructed from eboot.elf at 0x8E53C0.
void orbis_texture_3d_construct(
    OrbisTexture3D& texture,
    const RenderTexture3DDescriptor& descriptor) {
    texture_3d_construct(texture, descriptor);
    orbis_texture_3d_clear_backend_state(texture);
}

// Reconstructed from eboot.elf at 0x8E53F0.
void orbis_texture_3d_destruct(OrbisTexture3D& texture) {
    orbis_defer_texture_allocation(orbis_texture_3d_allocation(texture));
    if (auto* descriptor = orbis_texture_3d_gpu_texture(texture)) {
        render_release(descriptor);
    }
    orbis_texture_3d_set_gpu_texture(texture, nullptr);
    texture_3d_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8E5450.
void orbis_texture_3d_delete(OrbisTexture3D& texture) {
    orbis_texture_3d_destruct(texture);
    render_delete_texture_3d(texture);
}

// Reconstructed from eboot.elf at 0x8E54B0.
void orbis_texture_3d_initialize_backend(
    OrbisTexture3D& texture,
    const OrbisTexture3D* storage_source) {
    orbis_texture_3d_initialize_storage(texture, storage_source);
}

}  // namespace rb4
