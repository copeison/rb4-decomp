#pragma once

#include <cstddef>
#include <cstdint>

// Gnm render-target types shared by the PS4 window, textures and context.
// They stand in for the SDK's sce::Gnm types. Names not in the reference map.
namespace rb4 {

// Opaque sce::Gnm::RenderTarget register block.
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
