#pragma once

#include <cstdint>

namespace rb4 {

struct RenderTexture {
    void* implementation;
    std::int64_t frame_stamp;
    std::int32_t descriptor_type;
    std::uint8_t descriptor_state[76];
    std::uint32_t address_mode;
    std::uint32_t filter_mode;
    std::uint32_t flags;
    std::int32_t data_format;
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t depth;
    std::uint8_t reserved_124[4];
    void* source_data;
    std::uint32_t source_size;
    bool backend_initialized;
    std::uint8_t reserved_141[3];
    std::int32_t bindless_index;
    std::uint32_t usage;
    const char* name;
    std::int32_t resource_index;
    std::uint8_t trailing_reserved[4];
};

static_assert(sizeof(RenderTexture) == 168);

void render_texture_construct(RenderTexture& texture);
void render_texture_destruct(RenderTexture& texture);
void render_texture_delete(RenderTexture& texture);

}  // namespace rb4
