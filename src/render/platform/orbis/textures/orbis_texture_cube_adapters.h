#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_cube.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_cube_construct(
    OrbisTextureCube& texture,
    const RenderTextureCubeDescriptor& descriptor);
void orbis_texture_cube_clear_backend_state(OrbisTextureCube& texture);
bool orbis_texture_cube_is_depth(const OrbisTextureCube& texture);
void orbis_texture_cube_initialize_color_storage(
    OrbisTextureCube& texture);
void orbis_texture_cube_initialize_depth_storage(
    OrbisTextureCube& texture);
void* orbis_texture_cube_gpu_texture(const OrbisTextureCube& texture);
void* orbis_texture_cube_primary_allocation(
    const OrbisTextureCube& texture);
void* orbis_texture_cube_secondary_allocation(
    const OrbisTextureCube& texture);
OrbisGpuRenderTarget* orbis_texture_cube_mutable_render_target(
    const OrbisTextureCube& texture);
OrbisGpuDepthRenderTarget* orbis_texture_cube_mutable_depth_target(
    const OrbisTextureCube& texture);
void* orbis_render_target_metadata_allocation(
    const OrbisGpuRenderTarget& target);
void* orbis_render_target_surface_allocation(
    const OrbisGpuRenderTarget& target);
void orbis_defer_texture_allocation(void* allocation);
void render_release(void* allocation);
void orbis_texture_cube_set_gpu_texture(
    OrbisTextureCube& texture,
    void* descriptor);
void orbis_texture_cube_set_render_target(
    OrbisTextureCube& texture,
    OrbisGpuRenderTarget* target);
void orbis_texture_cube_set_depth_target(
    OrbisTextureCube& texture,
    OrbisGpuDepthRenderTarget* target);
void texture_cube_destruct(OrbisTextureCube& texture);
void render_delete_texture_cube(OrbisTextureCube& texture);
OrbisSamplerAddressMode orbis_texture_cube_address_mode(
    const OrbisTextureCube& texture);
std::uint32_t orbis_texture_cube_filter_mode(
    const OrbisTextureCube& texture);

}  // namespace rb4
