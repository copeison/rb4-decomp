#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_1d.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_1d_construct(
    OrbisTexture1D& texture,
    const RenderTexture1DDescriptor& descriptor);
void orbis_texture_1d_clear_backend_state(OrbisTexture1D& texture);
void orbis_texture_1d_initialize_storage(OrbisTexture1D& texture);
void* orbis_texture_1d_gpu_texture(const OrbisTexture1D& texture);
void* orbis_texture_1d_allocation(const OrbisTexture1D& texture);
void orbis_defer_texture_allocation(void* allocation);
void render_release(void* allocation);
void orbis_texture_1d_set_gpu_texture(
    OrbisTexture1D& texture,
    void* descriptor);
void texture_1d_destruct(OrbisTexture1D& texture);
void render_delete_texture_1d(OrbisTexture1D& texture);

}  // namespace rb4
