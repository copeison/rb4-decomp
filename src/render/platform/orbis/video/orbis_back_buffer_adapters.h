#pragma once

#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/video/orbis_back_buffer.h"

namespace rb4 {

struct OrbisTexture2D;

void* render_allocate(std::size_t size);
void render_free(void* allocation);
void orbis_back_buffer_construct_base(
    OrbisBackBuffer& back_buffer,
    std::uint32_t render_target_flags,
    bool allocate_target_state);
void orbis_back_buffer_destruct_base(OrbisBackBuffer& back_buffer);
void orbis_back_buffer_install_vtable(OrbisBackBuffer& back_buffer);
OrbisBackBufferSpecification orbis_back_buffer_specification(
    const OrbisRenderSystem& system,
    OrbisBackBufferDataFormat data_format);
void orbis_gpu_render_target_initialize(
    OrbisGpuRenderTarget& target,
    const OrbisBackBufferSpecification& specification);
OrbisSizeAlign orbis_gpu_render_target_size_align(
    const OrbisGpuRenderTarget& target);
void* orbis_gpu_allocate_named(
    std::size_t size,
    const char* name,
    std::size_t alignment);
void orbis_gpu_render_target_set_storage(
    OrbisGpuRenderTarget& target,
    void* allocation);
void orbis_gpu_render_target_disable_auxiliary_surfaces(
    OrbisGpuRenderTarget& target);
OrbisTexture2D* orbis_wrap_back_buffer_textures(
    OrbisGpuRenderTarget* targets,
    std::size_t target_count);
void orbis_back_buffer_attach_texture(
    OrbisBackBuffer& back_buffer,
    OrbisTexture2D& texture);
void orbis_video_output_register_back_buffers(
    OrbisRenderSystem& system,
    const OrbisGpuRenderTarget* targets,
    std::size_t target_count);
void render_system_set_back_buffer(
    OrbisRenderSystem& system,
    OrbisBackBuffer& back_buffer);

}  // namespace rb4
