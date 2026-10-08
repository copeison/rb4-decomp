#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace rb4 {

enum class RenderTextureUsage : std::int32_t {
    kDefault = 0,
    kDepth = 2,
};

struct RenderTextureCreationState {
    std::uint32_t values[11];
};

struct RenderTextureDescriptorState {
    std::int32_t descriptor_type;
    RenderTextureCreationState creation_state;
    RenderTextureUsage usage_type;
    std::array<std::uint8_t, 28> resolved_state;
    std::uint32_t address_mode;
    std::uint32_t filter_mode;
    std::uint32_t flags;
    std::int32_t data_format;
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t depth;
    std::uint32_t reserved_6C;
    union {
        void* source_data;
        std::size_t array_size;
    };
    union {
        std::uint32_t source_size;
        std::uint32_t target_flags;
    };
    bool backend_initialized;
    std::uint8_t reserved_7D[3];
    union {
        std::int32_t bindless_index;
        std::int32_t attachment_index;
    };
    union {
        std::uint32_t resource_flags;
        std::int32_t attachment_count;
    };
    const char* name;
};

struct RenderTexture {
    void* implementation;
    std::int64_t frame_stamp;
    std::int32_t descriptor_type;
    RenderTextureCreationState creation_state;
    RenderTextureUsage usage_type;
    std::array<std::uint8_t, 28> resolved_state;
    std::uint32_t address_mode;
    std::uint32_t filter_mode;
    std::uint32_t flags;
    std::int32_t data_format;
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t depth;
    std::uint8_t reserved_124[4];
    union {
        void* source_data;
        std::size_t array_size;
    };
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

union RenderTextureMipChainState;

struct RenderTextureMipChainFields {
    void* implementation;
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t depth;
    std::int32_t data_format;
    void* source_data;
    std::size_t source_size;
    RenderTextureMipChainState* next_mip;
    std::uint32_t metadata[6];
    void* auxiliary_data;
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
static_assert(offsetof(RenderTexture, creation_state) == 20);
static_assert(offsetof(RenderTexture, usage_type) == 64);
static_assert(offsetof(RenderTexture, resolved_state) == 68);
static_assert(sizeof(RenderTextureCreationState) == 44);
static_assert(sizeof(RenderTextureDescriptorState) == 144);
static_assert(offsetof(RenderTextureDescriptorState, creation_state) == 4);
static_assert(offsetof(RenderTextureDescriptorState, usage_type) == 48);
static_assert(offsetof(RenderTextureDescriptorState, flags) == 88);
static_assert(offsetof(RenderTextureDescriptorState, data_format) == 92);
static_assert(offsetof(RenderTextureDescriptorState, source_data) == 112);
static_assert(offsetof(RenderTextureDescriptorState, attachment_index) == 128);
static_assert(offsetof(RenderTextureDescriptorState, name) == 136);
static_assert(
    offsetof(RenderTexture, name) + sizeof(RenderTexture::name) -
        offsetof(RenderTexture, descriptor_type) ==
    sizeof(RenderTextureDescriptorState));
static_assert(sizeof(RenderTextureMipChainFields) == 80);
static_assert(offsetof(RenderTextureMipChainFields, width) == 8);
static_assert(offsetof(RenderTextureMipChainFields, data_format) == 20);
static_assert(offsetof(RenderTextureMipChainFields, source_data) == 24);
static_assert(offsetof(RenderTextureMipChainFields, source_size) == 32);
static_assert(offsetof(RenderTextureMipChainFields, next_mip) == 40);
static_assert(offsetof(RenderTextureMipChainFields, metadata) == 48);
static_assert(offsetof(RenderTextureMipChainFields, auxiliary_data) == 72);
static_assert(sizeof(RenderTextureMipChainState) == 80);
static_assert(sizeof(RenderTextureMipChainDescriptor) == 80);
static_assert(sizeof(RenderTextureMipChainDescriptorRange) == 24);
static_assert(sizeof(RenderTextureMipChainArray) == 32);

void render_texture_descriptor_construct(
    RenderTextureDescriptorState& descriptor);
void render_texture_apply_descriptor_state(
    RenderTexture& texture,
    const RenderTextureDescriptorState& descriptor);
bool render_texture_descriptor_has_source_data(
    const RenderTextureDescriptorState& descriptor);
void render_texture_construct(RenderTexture& texture);
void render_texture_destruct(RenderTexture& texture);
void render_texture_delete(RenderTexture& texture);
void render_texture_release_dynamic(RenderTexture& texture);
void render_texture_initialize_backend(
    RenderTexture& texture,
    const RenderTexture* reusable_texture);
std::int32_t render_texture_runtime_descriptor_type(
    const RenderTexture& texture);
std::int32_t render_texture_default_address_mode(
    std::uint32_t resource_kind);
std::int32_t render_texture_default_filter_mode(
    std::uint32_t resource_kind);

}  // namespace rb4
