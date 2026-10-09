#pragma once

#include <cstddef>
#include <cstdint>

#include "render/platform/orbis/video/orbis_back_buffer.h"

class PS4Texture2D;

namespace rb4 {

void orbis_back_buffer_install_vtable(OrbisBackBuffer& back_buffer);
OrbisBackBufferSpecification orbis_back_buffer_specification(
    const PS4Device& system,
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
PS4Texture2D* orbis_wrap_back_buffer_textures(
    OrbisGpuRenderTarget* targets,
    std::size_t target_count);
void orbis_back_buffer_attach_texture(
    OrbisBackBuffer& back_buffer,
    PS4Texture2D& texture);
void orbis_video_output_register_back_buffers(
    PS4Device& system,
    const OrbisGpuRenderTarget* targets,
    std::size_t target_count);
}  // namespace rb4
