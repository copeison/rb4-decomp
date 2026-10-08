#pragma once

#include <cstddef>
#include <cstdint>

namespace rb4 {

enum class RenderTextureUsage : std::int32_t {
    kDefault = 0,
    kDepth = 2,
};

struct RenderTexture {
    void* implementation;
    std::int64_t frame_stamp;
    std::int32_t descriptor_type;
    std::uint8_t descriptor_prefix[44];
    RenderTextureUsage usage_type;
    std::uint8_t descriptor_suffix[28];
    std::uint32_t address_mode;
    std::uint32_t filter_mode;
    std::uint32_t flags;
    std::int32_t data_format;
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t depth;
    std::uint8_t reserved_124[4];
    void* source_data;
    union {
        std::uint32_t source_size;
        std::uint32_t target_flags;
    };
    bool backend_initialized;
    std::uint8_t reserved_141[3];
    union {
        std::int32_t bindless_index;
        std::int32_t attachment_index;
    };
    union {
        std::uint32_t resource_flags;
        std::int32_t attachment_count;
    };
    const char* name;
    std::int32_t resource_index;
    std::uint8_t trailing_reserved[4];
};

struct RenderTextureMipChainFields {
    void* implementation;
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t depth;
    std::int32_t data_format;
    void* source_data;
    std::size_t source_size;
    std::uint8_t source_state[40];
};

union RenderTextureMipChainState {
    RenderTextureMipChainFields fields;
    std::uint8_t storage[80];
};

union RenderTextureMipChainDescriptor {
    RenderTextureMipChainFields fields;
    std::uint8_t storage[80];
};

struct RenderTextureMipChainDescriptorRange {
    const RenderTextureMipChainDescriptor* begin;
    const RenderTextureMipChainDescriptor* end;
    const RenderTextureMipChainDescriptor* capacity;
};

struct RenderTextureMipChainArray {
    RenderTextureMipChainState* begin;
    RenderTextureMipChainState* end;
    RenderTextureMipChainState* capacity;
    void* allocator;
};

static_assert(sizeof(RenderTexture) == 168);
static_assert(sizeof(RenderTextureMipChainFields) == 80);
static_assert(offsetof(RenderTextureMipChainFields, width) == 8);
static_assert(offsetof(RenderTextureMipChainFields, data_format) == 20);
static_assert(offsetof(RenderTextureMipChainFields, source_data) == 24);
static_assert(offsetof(RenderTextureMipChainFields, source_size) == 32);
static_assert(sizeof(RenderTextureMipChainState) == 80);
static_assert(sizeof(RenderTextureMipChainDescriptor) == 80);
static_assert(sizeof(RenderTextureMipChainDescriptorRange) == 24);
static_assert(sizeof(RenderTextureMipChainArray) == 32);

void render_texture_construct(RenderTexture& texture);
void render_texture_destruct(RenderTexture& texture);
void render_texture_delete(RenderTexture& texture);
void render_texture_initialize_backend(
    RenderTexture& texture,
    const RenderTexture* reusable_texture);
std::int32_t render_texture_default_address_mode(
    std::uint32_t resource_kind);
std::int32_t render_texture_default_filter_mode(
    std::uint32_t resource_kind);

}  // namespace rb4
