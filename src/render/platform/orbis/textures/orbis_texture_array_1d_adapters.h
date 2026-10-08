#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_array_1d.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_array_1d_construct(
    OrbisTextureArray1D& texture,
    const RenderTextureArray1DDescriptor& descriptor);
void orbis_texture_array_1d_clear_backend_state(
    OrbisTextureArray1D& texture);
void orbis_texture_array_1d_initialize_storage(
    OrbisTextureArray1D& texture);
void* orbis_texture_array_1d_gpu_texture(
    const OrbisTextureArray1D& texture);
void* orbis_texture_array_1d_allocation(
    const OrbisTextureArray1D& texture);
void orbis_defer_texture_allocation(void* allocation);
void render_release(void* allocation);
void orbis_texture_array_1d_set_gpu_texture(
    OrbisTextureArray1D& texture,
    void* descriptor);
void texture_array_1d_destruct(OrbisTextureArray1D& texture);
void render_delete_texture_array_1d(OrbisTextureArray1D& texture);
OrbisSamplerAddressMode orbis_texture_array_1d_address_mode(
    const OrbisTextureArray1D& texture);
std::uint32_t orbis_texture_array_1d_filter_mode(
    const OrbisTextureArray1D& texture);

}  // namespace rb4
