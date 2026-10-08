#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_3d.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_3d_construct(
    OrbisTexture3D& texture,
    const RenderTexture3DDescriptor& descriptor);
void orbis_texture_3d_clear_backend_state(OrbisTexture3D& texture);
void orbis_texture_3d_initialize_storage(
    OrbisTexture3D& texture,
    const OrbisTexture3D* storage_source);
void* orbis_texture_3d_gpu_texture(const OrbisTexture3D& texture);
void* orbis_texture_3d_allocation(const OrbisTexture3D& texture);
void orbis_defer_texture_allocation(void* allocation);
void render_release(void* allocation);
void orbis_texture_3d_set_gpu_texture(
    OrbisTexture3D& texture,
    void* descriptor);
void texture_3d_destruct(OrbisTexture3D& texture);
void render_delete_texture_3d(OrbisTexture3D& texture);

}  // namespace rb4
