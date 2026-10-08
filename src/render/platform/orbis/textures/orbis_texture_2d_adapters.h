#pragma once

#include <cstddef>

#include "render/platform/orbis/textures/orbis_texture_2d.h"

namespace rb4 {

void* render_allocate(std::size_t size);
void texture_2d_construct(
    OrbisTexture2D& texture,
    const RenderTexture2DDescriptor& descriptor);
void orbis_texture_2d_clear_backend_state(OrbisTexture2D& texture);
void orbis_texture_2d_release_backend_state(OrbisTexture2D& texture);
bool orbis_texture_2d_is_depth(const OrbisTexture2D& texture);
void orbis_texture_2d_initialize_depth_storage(OrbisTexture2D& texture);
void orbis_texture_2d_initialize_color_storage(
    OrbisTexture2D& texture,
    const OrbisTexture2D* storage_source);
void orbis_texture_2d_flip_active_storage(OrbisTexture2D& texture);
void orbis_texture_2d_upload_active_mips(OrbisTexture2D& texture);
OrbisGpuRenderTarget* orbis_texture_2d_mutable_render_target(
    const OrbisTexture2D& texture);
OrbisGpuDepthRenderTarget* orbis_texture_2d_mutable_depth_target(
    const OrbisTexture2D& texture);
void texture_2d_destruct(OrbisTexture2D& texture);
void render_delete_texture_2d(OrbisTexture2D& texture);
const void* orbis_texture_2d_binding_view(
    const OrbisTexture2D& texture,
    std::uint32_t flags);
OrbisSamplerAddressMode orbis_texture_2d_address_mode(
    const OrbisTexture2D& texture);
std::uint32_t orbis_texture_2d_filter_mode(
    const OrbisTexture2D& texture);

}  // namespace rb4
