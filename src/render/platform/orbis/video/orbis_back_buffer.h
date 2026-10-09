#pragma once

#include <cstddef>
#include <cstdint>

#include "render/core/targets/render_target.h"

class PS4Device;

namespace rb4 {

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

OrbisBackBuffer* orbis_back_buffer_create(PS4Device& system);
void orbis_back_buffer_construct(
    OrbisBackBuffer& back_buffer,
    PS4Device& system);
void orbis_back_buffer_destruct(OrbisBackBuffer& back_buffer);
void orbis_back_buffer_delete(OrbisBackBuffer& back_buffer);
bool orbis_back_buffer_advance(OrbisBackBuffer& back_buffer);

}  // namespace rb4
