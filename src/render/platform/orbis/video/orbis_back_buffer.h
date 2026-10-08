#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/render_target.h"

namespace rb4 {

struct OrbisRenderSystem;

struct OrbisBackBuffer : RenderTarget {
    std::size_t active_buffer;
};

struct OrbisGpuRenderTarget {
    std::uint8_t registers[64];
};

enum class OrbisBackBufferDataFormat : std::uint32_t {
    kB8G8R8A8Srgb = 0x00F2E90A,
};

struct OrbisBackBufferSpecification {
    std::uint32_t width;
    std::uint32_t height;
    OrbisBackBufferDataFormat data_format;
    std::uint32_t tile_mode;
    std::uint32_t gpu_mode;
};

struct OrbisSizeAlign {
    std::size_t size;
    std::size_t alignment;
};

OrbisBackBuffer* orbis_back_buffer_create(OrbisRenderSystem& system);
void orbis_back_buffer_construct(
    OrbisBackBuffer& back_buffer,
    OrbisRenderSystem& system);
void orbis_back_buffer_destruct(OrbisBackBuffer& back_buffer);
void orbis_back_buffer_delete(OrbisBackBuffer& back_buffer);
bool orbis_back_buffer_advance(OrbisBackBuffer& back_buffer);

}  // namespace rb4
