#pragma once

#include <cstddef>
#include <cstdint>

class PS4Device;

namespace rb4 {

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


}  // namespace rb4
