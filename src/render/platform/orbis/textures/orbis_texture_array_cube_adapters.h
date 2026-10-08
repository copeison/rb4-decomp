#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_array_cube.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_array_cube_construct(
    OrbisTextureArrayCube& texture,
    const RenderTextureArrayCubeDescriptor& descriptor);
void orbis_texture_array_cube_clear_backend_state(
    OrbisTextureArrayCube& texture);
void orbis_texture_array_cube_initialize_storage(
    OrbisTextureArrayCube& texture);
void* orbis_texture_array_cube_gpu_texture(
    const OrbisTextureArrayCube& texture);
void* orbis_texture_array_cube_allocation(
    const OrbisTextureArrayCube& texture);
void orbis_defer_texture_allocation(void* allocation);
void render_release(void* allocation);
void orbis_texture_array_cube_set_gpu_texture(
    OrbisTextureArrayCube& texture,
    void* descriptor);
void texture_array_cube_destruct(OrbisTextureArrayCube& texture);
void render_delete_texture_array_cube(OrbisTextureArrayCube& texture);
OrbisSamplerAddressMode orbis_texture_array_cube_address_mode(
    const OrbisTextureArrayCube& texture);
std::uint32_t orbis_texture_array_cube_filter_mode(
    const OrbisTextureArrayCube& texture);

}  // namespace rb4
