#include "render/platform/orbis/textures/orbis_texture_2d.h"

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_2d_adapters.h"

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

// Reconstructed from eboot.elf at 0x8D6310.
void orbis_texture_2d_destruct(OrbisTexture2D& texture) {
    orbis_texture_2d_release_backend_state(texture);
    texture_2d_destruct(texture);
}

// Reconstructed from eboot.elf at 0x8D6440.
void orbis_texture_2d_delete(OrbisTexture2D& texture) {
    orbis_texture_2d_destruct(texture);
    render_delete_texture_2d(texture);
}

// Reconstructed from eboot.elf at 0x8D6460.
void orbis_texture_2d_initialize_backend(
    OrbisTexture2D& texture,
    const OrbisTexture2D* storage_source) {
    if (orbis_texture_2d_is_depth(texture)) {
        orbis_texture_2d_initialize_depth_storage(texture);
    } else {
        orbis_texture_2d_initialize_color_storage(texture, storage_source);
    }
}

// Reconstructed from eboot.elf at 0x8D6D10.
void orbis_texture_2d_update_gpu_data(OrbisTexture2D& texture) {
    orbis_texture_2d_flip_active_storage(texture);
    orbis_texture_2d_upload_active_mips(texture);
}

// Reconstructed from eboot.elf at 0x8D71E0.
const OrbisGpuRenderTarget* orbis_texture_2d_render_target(
    const OrbisTexture2D& texture) {
    return orbis_texture_2d_mutable_render_target(texture);
}

// Reconstructed from eboot.elf at 0x8D7240.
const OrbisGpuDepthRenderTarget* orbis_texture_2d_depth_target(
    const OrbisTexture2D& texture) {
    return orbis_texture_2d_mutable_depth_target(texture);
}

}  // namespace rb4
